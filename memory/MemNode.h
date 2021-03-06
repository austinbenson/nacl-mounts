/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#ifndef PACKAGES_SCRIPTS_FILESYS_MEMORY_MEMNODE_H_
#define PACKAGES_SCRIPTS_FILESYS_MEMORY_MEMNODE_H_

#include <string.h>
#include <sys/stat.h>
#include <list>
#include <string>

#include "../base/SlotAllocator.h"


class MemMount;

// A macro to disallow the evil copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName)      \
  TypeName(const TypeName&);                    \
  void operator=(const TypeName&)

// MemNode is the node object for the memory mount (mem_mount class)
// This class overrides all of the MountNode sys call methods.  In
// addition, this class keeps track of parent/child relationships by
// maintaining a parent node pointer and a list of children.
class MemNode {
 public:
  int slot;
  // constructor initializes the private variables
  MemNode();

  // destructor frees allocated memory
  virtual ~MemNode();

  // Override the sys call methods from mount_node
  int remove();
  int stat(struct stat *buf);
  int chmod(mode_t mode);
  int utime(struct utimbuf const *times);
  int unlink(void);
  int rmdir(void);

  // Add child to this node's children.  This method will do nothing
  // if this node is a directory or if child points to a child that is
  // not in the children list of this node
  virtual void AddChild(int slot);

  // Remove child from this node's children.  This method will do
  // nothing if the node is not a directory
  virtual void RemoveChild(int slot);

  // Reallocate the size of data to be len bytes.  Copies the
  // current data to the reallocated memory.
  virtual void ReallocData(int len);

  // children() returns a list of MemNode pointers
  // which represent the children of this node.
  // If this node is a file or a directory with no children,
  // a NULL pointer is returned.
  virtual std::list<int> *children(void);

  // set_name() sets the name of this node.  This is not the
  // path but rather the name of the file or directory
  virtual void set_name(std::string name) { name_ = name; }

  // name() returns the name of this node
  virtual std::string name(void) { return name_; }

  // set_parent() sets the parent node of this node to
  // parent_
  virtual void set_parent(int parent) { parent_ = parent; }

  // parent() returns a pointer to the parent node of
  // this node.  If this node is the root node, NULL
  // is returned.
  virtual int parent(void) { return parent_; }

  // increase the use count by one
  virtual void IncrementUseCount(void) { ++use_count_; }

  // decrease the use count by one
  virtual void DecrementUseCount(void) { --use_count_; }

  // returns the use count of this node
  virtual int use_count(void) { return use_count_; }

  // capacity() returns the capcity (in bytes) of this node
  virtual int capacity(void) { return capacity_; }

  // set the capacity of this node to capacity bytes
  virtual void set_capacity(int capacity) { capacity_ = capacity; }

  // truncate() sets the length of this node to zero
  virtual void Truncate() { len_ = 0; }

  // data() returns a pointer to the data of this node
  virtual char *data(void) { return data_; }

  // set_data() sets the length of this node to len
  virtual void set_len(size_t len) { len_ = len; }

  // len() returns the length of this node
  virtual size_t len(void) { return len_; }

  // set_mount() sets the mount to which this node belongs
  virtual void set_mount(MemMount *mount) { mount_ = mount; }

  // stat helper
  virtual void raw_stat(struct stat *buf);

  bool is_dir() { return is_dir_; }
  void set_is_dir(bool is_dir) { is_dir_ = is_dir; }

 private:
  std::string name_;
  int parent_;
  MemMount *mount_;
  char *data_;
  size_t len_;
  size_t capacity_;
  int use_count_;
  bool is_dir_;
  std::list<int> children_;
};

#endif  // PACKAGES_SCRIPTS_FILESYS_MEMORY_MEMNODE_H_
