/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
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
  std::vector<char> remote_data;
  fprintf(stderr, "before remote read...\n");
  url_request_.Read(path, remote_data);
  fprintf(stderr, "after remote read...\n");

  if (remote_data.size() != 0) {
    memcpy(child->data(), &remote_data[0], remote_data.size());
  }

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
  // Check that it is a directory.
  if (!(node->is_dir())) {
    errno = ENOTDIR;
    return -1;
  }

  std::vector<std::string> dst;
  url_request_.List(node->path(), dst);

  // TODO(arbenson): update the dirent struct

  return dst.size()*sizeof(struct dirent);
}

ssize_t AppEngineMount::Read(ino_t slot, off_t offset, void *buf, size_t count) {
  AppEngineNode* node = slots_.At(slot);
  if (node == NULL) {
    errno = ENOENT;
    return -1;
  }
  // Limit to the end of the file.
  size_t len = count;
  if (len > node->len() - offset) {
    len = node->len() - offset;
  }

  // Do the read.
  memcpy(buf, node->data() + offset, len);
  return len;
}

ssize_t AppEngineMount::Write(ino_t slot, off_t offset, const void *buf, size_t count) {
  fprintf(stderr, "Entering AppEngineMount::Write()\n");
  AppEngineNode* node = slots_.At(slot);
  if (node == NULL) {
    errno = ENOENT;
    return -1;
  }

  size_t len;
  // Grow the file if needed.
  if (offset + static_cast<off_t>(count) > node->capacity()) {
    len = offset + count;
    size_t next = (node->capacity() + 1) * 2;
    if (next > len) {
      len = next;
    }
    fprintf(stderr, "About to realloc data\n");
    fprintf(stderr, "Is node NULL?: %d\n", node == NULL);
    node->ReallocData(len);
  }
  // Pad any gap with zeros.
  if (offset > static_cast<off_t>(node->len())) {
    fprintf(stderr, "About to memset\n");
    memset(node->data()+len, 0, offset);
  }

  // Write out the block.
  fprintf(stderr, "About to memcpy\n");
  memcpy(node->data() + offset, buf, count);
  offset += count;
  if (offset > static_cast<off_t>(node->len())) {
    node->set_len(offset);
  }
  fprintf(stderr, "Leaving AppEngineMount::Write()\n");
  return count;
}

int AppEngineMount::Fsync(ino_t slot) {
  AppEngineNode* node = slots_.At(slot);
  char *data = node->data();
  int numbytes = sizeof(data);
  std::vector<char> data_to_send(numbytes);
  memcpy(&data_to_send[0], data, numbytes);
  return url_request_.Write(node->path(), data_to_send);
}

