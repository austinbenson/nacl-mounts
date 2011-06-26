/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#include "MemNode.h"

MemNode::MemNode() {
  data_ = NULL;
  len_ = 0;
  capacity_ = 0;
  use_count_ = 0;
}

MemNode::~MemNode() {
  children_.clear();
}

int MemNode::remove() {
  mount_->AcquireLock();

  // Check that it's a file.
  if (is_dir()) {
    errno = EISDIR;
    mount_->ReleaseLock();
    return -1;
  }
  // Check that it's not busy.
  if (use_count() > 0) {
    errno = EBUSY;
    mount_->ReleaseLock();
    return -1;
  }
  // Get the node's parent.
  assert(parent_);
  // Drop it from parent.
  parent_->RemoveChild(this);

  // Free it.
  mount_->ReleaseLock();
  delete this;
  return 0;
}

int MemNode::rmdir() {
  mount_->AcquireLock();
  // Check if it's a directory.
  if (!is_dir()) {
    errno = ENOTDIR;
    mount_->ReleaseLock();
    return -1;
  }
  // Check if it's empty.
  if (children_.size() > 0) {
    errno = ENOTEMPTY;
    mount_->ReleaseLock();
    return -1;
  }
  // if this isn't the root node, remove from parent's
  // children list
  if (parent_) {
    parent_->RemoveChild(this);
  }
  mount_->ReleaseLock();
  delete this;
  return 0;
}

int MemNode::stat(struct stat *buf) {
  mount_->AcquireLock();
  raw_stat(buf);
  mount_->ReleaseLock();
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

int MemNode::access(int amode) {
  return 0;
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

void MemNode::AddChild(Node *child) {
  if (!(is_dir()))
    return;
  children_.push_back(child);
}

void MemNode::RemoveChild(Node *child) {
  if (!(is_dir()))
    return;
  children_.remove(child);
}

void MemNode::ReallocData(int len) {
  assert(len > 0);
  // TODO(arbenson): Handle memory overflow more gracefully.
  data_ = reinterpret_cast<char *>(realloc(data_, len));
  set_capacity(len);
  assert(data_);
}

std::list<Node2 *> *MemNode::children() {
  if (is_dir()) {
    std::list<Node2*>* res = new std::list<Node2*>();
    std::list<Node *>::iterator it;
    for (it = children_.begin(); it != children_.end(); ++it) {
      res->push_back(new MemNode2(*it));
    }
    return res;
  } else {
    return NULL;
  }
}
