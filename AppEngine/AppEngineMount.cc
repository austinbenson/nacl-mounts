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

AppEngineMount::AppEngineMount(pp::Instance *instance, std::string base_url)
  : url_request_(instance, base_url) {
}

AppEngineMount::~AppEngineMount()  {
}

AppEngineNode* AppEngineMount::ToAppEngineNode(Node2 *node) {
  if (node == NULL) {
    return NULL;
  }
  return reinterpret_cast<AppEngineNode2*>(node)->node();
}

Node2 *AppEngineMount::Creat(std::string path, mode_t mode) {
  AppEngineNode *node;
  AppEngineNode *parent;
  fprintf(stderr, "in Creat\n");

  // Create it.
  node = new AppEngineNode();
  node->set_path(path);
  node->set_is_dir(false);
  std::string p(path);
  PathHandle ph(p);
  node->set_name(ph.Last());
  node->set_mount(this);
  node->IncrementUseCount();

  // read from GAE
  std::vector<char> remote_data;
  fprintf(stderr, "before remote read...\n");
  url_request_.Read(path, remote_data);
  fprintf(stderr, "after remote read...\n");
  
  if (remote_data.size() != 0)
    memcpy(node->data(), &remote_data[0], remote_data.size());
  return new AppEngineNode2(node);
}

int AppEngineMount::mkdir(std::string path, mode_t mode) {
  return 0;
}

Node2 *AppEngineMount::GetNode(std::string path) {
  return Creat(path, 0);
}

int AppEngineMount::chmod(Node2 *node, mode_t mode) {
  return ToAppEngineNode(node)->chmod(mode);
}

int AppEngineMount::stat(Node2* node, struct stat *buf) {
  return ToAppEngineNode(node)->stat(buf);
}

int AppEngineMount::remove(Node2* node) {
  return ToAppEngineNode(node)->remove();
}

int AppEngineMount::rmdir(Node2* node) {
  return ToAppEngineNode(node)->rmdir();
}

void AppEngineMount::DecrementUseCount(Node2* node) {
  return ToAppEngineNode(node)->DecrementUseCount();
}

int AppEngineMount::Getdents(Node2* node2, off_t offset,
                             struct dirent *dir, unsigned int count) {


  AppEngineNode* node = reinterpret_cast<AppEngineNode2*>(node2)->node();
  // Check that it is a directory.
  if (!(node->is_dir())) {
    errno = ENOTDIR;
    return -1;
  }

  std::vector<std::string> dst;
  url_request_.List(node->path(), dst);

  // TODO(arbenson): update the dirent struct

  return dst.size()*sizeof(struct stat);
}

ssize_t AppEngineMount::Read(Node2* node, off_t offset, void *buf, size_t count) {
  AppEngineNode* aenode = ToAppEngineNode(node);
  // Limit to the end of the file.
  size_t len = count;
  if (len > aenode->len() - offset) {
    len = aenode->len() - offset;
  }

  // Do the read.
  memcpy(buf, aenode->data() + offset, len);
  return len;
}

ssize_t AppEngineMount::Write(Node2* node, off_t offset, const void *buf, size_t count) {
  AppEngineNode* aenode = ToAppEngineNode(node);

  fprintf(stderr, "Entering AppEngineMount::Write()\n");
  size_t len;
  // Grow the file if needed.
  if (offset + static_cast<off_t>(count) > aenode->capacity()) {
    len = offset + count;
    size_t next = (aenode->capacity() + 1) * 2;
    if (next > len) {
      len = next;
    }
    fprintf(stderr, "About to realloc data\n");
    fprintf(stderr, "Is aenode NULL?: %d\n", aenode == NULL);
    aenode->ReallocData(len);
  }
  // Pad any gap with zeros.
  if (offset > static_cast<off_t>(aenode->len())) {
    fprintf(stderr, "About to memset\n");
    memset(aenode->data()+len, 0, offset);
  }

  // Write out the block.
  fprintf(stderr, "About to memcpy\n");
  memcpy(aenode->data() + offset, buf, count);
  offset += count;
  if (offset > static_cast<off_t>(aenode->len())) {
    aenode->set_len(offset);
  }
  fprintf(stderr, "Leaving AppEngineMount::Write()\n");
  return count;
}

int AppEngineMount::Fsync(Node2* node) {
  AppEngineNode* aenode = ToAppEngineNode(node);
  char *data = aenode->data();
  int numbytes = sizeof(data);
  std::vector<char> data_to_send(numbytes);
  memcpy(&data_to_send[0], data, numbytes);
  if (url_request_.Write(aenode->path(), data_to_send) == 1) {
    return 0;
  }
  return -1;
}
