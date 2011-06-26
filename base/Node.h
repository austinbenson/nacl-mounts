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

#include <list>
#include <string>

#include "Node2.h"

class MemMount;

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

  // set_data() sets the length of this node to len
  virtual void set_len(size_t len) { }

  // len() returns the length of this node
  virtual size_t len(void) { return 0; }

  // capacity() returns the capcity (in bytes) of this node
  virtual int capacity(void) { return 0; }

  // set the capacity of this node to capacity bytes
  virtual void set_capacity(int capacity) { }

  // data() returns a pointer to the data of this node
  virtual char *data(void) { return 0; }

  // Reallocate the size of data to be len bytes.  Copies the
  // current data to the reallocated memory.
  virtual void ReallocData(int len) {}

  // Add child to this node's children.  This method will do nothing
  // if this node is a directory or if child points to a child that is
  // not in the children list of this node
  virtual void AddChild(Node *child) {}

  // returns the use count of this node
  virtual int use_count(void) { return 0; }

  // Remove child from this node's children.  This method will do
  // nothing if the node is not a directory
  virtual void RemoveChild(Node *child) {}

  // stat helper
  virtual void raw_stat(struct stat *buf) {}

  // increase the use count by one
  virtual void IncrementUseCount(void) { }

  // decrease the use count by one
  virtual void DecrementUseCount(void) { }

  // truncate() sets the length of this node to zero
  virtual void Truncate() { }

  // set_name() sets the name of this node.  This is not the
  // path but rather the name of the file or directory
  virtual void set_name(std::string name) { }

  // name() returns the name of this node
  virtual std::string name(void) { return ""; }

  // set_parent() sets the parent node of this node to
  // parent_
  virtual void set_parent(Node *parent) { }

  // parent() returns a pointer to the parent node of
  // this node.  If this node is the root node, NULL
  // is returned.
  virtual Node *parent(void) { return NULL; }

  // children() returns a list of MemNode pointers
  // which represent the children of this node.
  // If this node is a file or a directory with no children,
  // a NULL pointer is returned.
  virtual std::list<Node2 *> *children(void) { return NULL; }

  // set_mount() sets the mount to which this node belongs
  virtual void set_mount(MemMount *mount) { }


 protected:
  bool is_dir_;
};

#endif  // PACKAGES_SCRIPTS_FILESYS_BASE_NODE_H_
