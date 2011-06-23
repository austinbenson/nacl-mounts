/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#ifndef PACKAGES_SCRIPTS_FILESYS_BASE_NODE_H_
#define PACKAGES_SCRIPTS_FILESYS_BASE_NODE_H_

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Node is the base class for implementing nodes for
// a particular mount.  Node is designed to intercept sys
// calls that take a path as an argument.
// The implementations here are all do-nothing stubs. A mount node
// class that inherits class can override whichever methods.
class Node {
 public:
  Node() {}
  virtual ~Node() {}

  // The following sys calls are called by the mount manager
  // When implementing a new mount, nodes should override
  // these methods as appropriate.
  virtual int open(int oflag) { return 0; }
  virtual int stat(struct stat *buf);
  virtual int unlink() { return 0; }
  virtual int chmod(mode_t mode) { return 0; }
  virtual int remove() { return 0; }
  virtual int utime(struct utimbuf const *times) { return 0; }
  virtual int rmdir() { return 0; }
  // access() is fully implemented in this base class by
  // calling stat
  virtual int access(int amode);

  bool is_dir() { return is_dir_; }
  void set_is_dir(bool is_dir) { is_dir_ = is_dir; }

 protected:
  bool is_dir_;
};

#endif  // PACKAGES_SCRIPTS_FILESYS_BASE_NODE_H_

