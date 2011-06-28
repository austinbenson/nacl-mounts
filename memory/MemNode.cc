/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */

#include "MemNode.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

MemNode::MemNode() {
  data_ = NULL;
  len_ = 0;
  capacity_ = 0;
  use_count_ = 0;
}

MemNode::~MemNode() {
  children_.clear();
}

int MemNode::stat(struct stat *buf) {
  raw_stat(buf);
  return 0;
}

void MemNode::raw_stat(struct stat *buf) {
  memset(buf, 0, sizeof(struct stat));
  if (is_dir()) {
    buf->st_mode = S_IFDIR | 0777;
  } else {
    buf->st_mode = S_IFREG | 0777;
    buf->st_size = len_;
  }
  buf->st_uid = 1001;
  buf->st_gid = 1002;
  buf->st_blksize = 1024;
}

int MemNode::chmod(mode_t mode) {
  fprintf(stderr, "chmod is not implemented!\n");
  assert(0);
  return 0;
}

int MemNode::utime(struct utimbuf const *times) {
  return 0;
}

int MemNode::unlink() {
  return 0;
}

void MemNode::AddChild(int child) {
  if (!is_dir()) {
    return;
  }
  children_.push_back(child);
}

void MemNode::RemoveChild(int child) {
  if (!is_dir()) {
    return;
  }
  children_.remove(child);
}

void MemNode::ReallocData(int len) {
  assert(len > 0);
  // TODO(arbenson): Handle memory overflow more gracefully.
  data_ = reinterpret_cast<char *>(realloc(data_, len));
  set_capacity(len);
  assert(data_);
}

std::list<int> *MemNode::children() {
  if (is_dir()) {
    return &children_;
  } else {
    return NULL;
  }
}
