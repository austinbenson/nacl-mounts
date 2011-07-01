/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "AppEngineMount.h"
#include "AppEngineNode.h"
#include "../base/dirent.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>

AppEngineMount::AppEngineMount(MainThreadRunner *runner, std::string base_url)
  : url_request_(runner, base_url) {
  slots_.Alloc();
}

int AppEngineMount::Creat(const std::string& path, mode_t mode, struct stat* buf) {
  AppEngineNode *child;
  AppEngineNode *parent;
  fprintf(stderr, "in Creat\n");

  // Create it.
  int slot = slots_.Alloc();
  child = slots_.At(slot);
  child->set_path(path);
  child->slot = slot;
  child->set_is_dir(false);
  child->set_mount(this);
  std::string p(path);
  PathHandle ph(p);
  child->set_name(ph.Last());
  child->IncrementUseCount();

  // read from GAE
  fprintf(stderr, "Before remote read...\n");
  std::vector<char> data;
  url_request_.Read(path, data);
  fprintf(stderr, "Done with remote read.\n");
  child->set_data(data);

  std::vector<char> test = child->data();
  std::vector<char>::iterator it;

  fprintf(stderr, "-------------\n");
  for (it = test.begin(); it != test.end(); ++it) {
    fprintf(stderr, "%c, ", *it);
  }
  fprintf(stderr, "\n-------------\n");

  if (!buf) {
    return 0;
  }
  return Stat(slot, buf);
}

int AppEngineMount::Mkdir(const std::string& path, mode_t mode, struct stat* buf) {
  return 0;
}

int AppEngineMount::GetNode(const std::string& path, struct stat* buf) {
  return Creat(path, 0, buf);
}

int AppEngineMount::Chmod(ino_t slot, mode_t mode) {
  AppEngineNode* node = slots_.At(slot);
  if (node == NULL) {
    errno = ENOENT;
    return -1;
  }
  return node->chmod(mode);
}

int AppEngineMount::Stat(ino_t slot, struct stat *buf) {
  AppEngineNode* node = slots_.At(slot);
  if (node == NULL) {
    errno = ENOENT;
    return -1;
  }
  return node->stat(buf);
}

int AppEngineMount::Rmdir(ino_t slot) {
  return 0;
}

void AppEngineMount::Ref(ino_t slot) {
  AppEngineNode* node = slots_.At(slot);
  if (node == NULL) {
    return;
  }
  node->IncrementUseCount();
}

void AppEngineMount::Unref(ino_t slot) {
  AppEngineNode* node = slots_.At(slot);
  if (node == NULL) {
    return;
  }
  if (node->is_dir()) {
    return;
  }
  node->DecrementUseCount();
  if (node->use_count() > 0) {
    return;
  }
  // If Ref/Unref misused by KernelProxy, it's possible
  // that parent will have a dangling inode to the deleted child
  // TODO(krasin): remove the possibility to misuse this API.
  slots_.Free(node->slot);
}

int AppEngineMount::Getdents(ino_t slot, off_t offset,
                       struct dirent *dir, unsigned int count) {
  AppEngineNode* node = slots_.At(slot);
  if (node == NULL) {
    errno = ENOTDIR;
    return -1;
  }
  std::vector<char> dst;
  url_request_.List(node->path(), dst);

  std::vector<std::string> entries;
  std::vector<char>::iterator ind = dst.begin();
  std::vector<char>::iterator next = dst.begin();
  while (ind != dst.end() && next != dst.end()) {
    if (*next == '\n' && ind != next) {
      entries.push_back(std::string(ind, next-1));
      ind = next;
    }
    ++next;
  }
  std::string last(ind, next-1);
  if (!last.empty()) entries.push_back(last);

  // TODO(arbenson): update the dirent struct
  return entries.size();
}

ssize_t AppEngineMount::Read(ino_t slot, off_t offset, void *buf, size_t count) {
  AppEngineNode* node = slots_.At(slot);
  if (node == NULL) {
    errno = ENOENT;
    return -1;
  }
  // Limit to the end of the file.
  size_t len = count;
  std::vector<char> data = node->data();
  if (len > data.size() - offset) {
    len = data.size() - offset;
  }

  // Do the read.
  if (!data.empty()) {
    memcpy(buf, &data[0] + offset, len);
  }
  return len;
}

ssize_t AppEngineMount::Write(ino_t slot, off_t offset, const void *buf, size_t count) {
  fprintf(stderr, "Entering AppEngineMount::Write()\n");
  AppEngineNode* node = slots_.At(slot);
  if (node == NULL) {
    errno = ENOENT;
    return -1;
  }
  // Write out the block.
  if (node->WriteData(offset, buf, count) == -1) {
    return -1;
  }
  return count;
}

int AppEngineMount::Fsync(ino_t slot) {
  fprintf(stderr, "In sync\n");
  AppEngineNode* node = slots_.At(slot);
  return url_request_.Write(node->path(), node->data());
}

