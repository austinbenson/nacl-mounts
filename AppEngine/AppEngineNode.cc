/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */

#include "AppEngineNode.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

AppEngineNode::AppEngineNode() {
  len_ = 0;
  capacity_ = 0;
  use_count_ = 0;
}

AppEngineNode::~AppEngineNode() {
  children_.clear();
}

int AppEngineNode::remove() {
  if (mount_->url_request()->Remove(path()) == 0) {
    errno = ENOENT;
    return -1;
  }
  return 0;
}

int AppEngineNode::rmdir() {
  return 0;
}

int AppEngineNode::stat(struct stat *buf) {
  // TODO(arbenson): change to not use raw_stat
  raw_stat(buf);
  return 0;
}

void AppEngineNode::raw_stat(struct stat *buf) {
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

int AppEngineNode::chmod(mode_t mode) {
  fprintf(stderr, "chmod is not implemented!\n");
  assert(0);
  return 0;
}

int AppEngineNode::utime(struct utimbuf const *times) {
  return 0;
}

int AppEngineNode::unlink() {
  return 0;
}

void AppEngineNode::ReallocData(int len) {
  fprintf(stderr, "Reallocing data with len=%d\n", len);
  assert(len > 0);
  fprintf(stderr, "Reallocing...\n");
  data_ = reinterpret_cast<char *>(realloc(data_, len));
  fprintf(stderr, "Done reallocing\n");
  set_capacity(len);
  fprintf(stderr, "Reallocing data NULL? %d\n", data_ == NULL);
  assert(data_);
}

