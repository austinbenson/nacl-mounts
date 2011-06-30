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
}

int AppEngineNode::stat(struct stat *buf) {
  // TODO(arbenson): change to not use raw_stat
  raw_stat(buf);
  return 0;
}

void AppEngineNode::raw_stat(struct stat *buf) {
  memset(buf, 0, sizeof(struct stat));
  buf->st_ino = (ino_t)slot;
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
  errno = ENOSYS;
  return -1;
}

int AppEngineNode::utime(struct utimbuf const *times) {
  errno = ENOSYS;
  return -1;
}

int AppEngineNode::unlink() {
  errno = ENOSYS;
  return -1;
}

void AppEngineNode::ReallocData(int len) {
  assert(len > 0);
  data_.resize(len);
  set_capacity(len);
}

int AppEngineNode::WriteData(off_t offset, const void *buf, size_t count) {
  size_t len;
  // Grow the file if needed.
  if (offset + static_cast<off_t>(count) > data_.size()) {
    len = offset + count;
    size_t next = (data_.size() + 1) * 2;
    if (next > len) {
      len = next;
    }
    ReallocData(len);
  }

  memcpy(&data_[0]+offset, buf, count);
  offset += count;
  if (offset > static_cast<off_t>(data_.size())) {
    set_len(offset);
  }
  return 0;
}
