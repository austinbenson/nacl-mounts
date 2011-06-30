/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#ifndef PACKAGES_SCRIPTS_FILESYS_APPENGINE_APPENGINENODE_H_
#define PACKAGES_SCRIPTS_FILESYS_APPENGINE_APPENGINENODE_H_

#include <string.h>
#include <sys/stat.h>
#include <list>
#include <string>

#include "../base/SlotAllocator.h"

class AppEngineMount;

// A macro to disallow the evil copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName)      \
  TypeName(const TypeName&);                    \
  void operator=(const TypeName&)

// AppEngineNode is the node object for the memory mount (mem_mount class)
// This class overrides all of the MountNode sys call methods.  In
// addition, this class keeps track of parent/child relationships by
// maintaining a parent node pointer and a list of children.
class AppEngineNode {
 public:
  int slot;
  // constructor initializes the private variables
  AppEngineNode();

  // destructor frees allocated memory
  virtual ~AppEngineNode();

  // Override the sys call methods from mount_node
  int remove();
  int stat(struct stat *buf);
  int chmod(mode_t mode);
  int utime(struct utimbuf const *times);
  int unlink(void);
  int rmdir(void);

  // Reallocate the size of data to be len bytes.  Copies the
  // current data to the reallocated memory.
  virtual void ReallocData(int len);

  // set_name() sets the name of this node.  This is not the
  // path but rather the name of the file or directory
  virtual void set_name(std::string name) { name_ = name; }

  // name() returns the name of this node
  virtual std::string name(void) { return name_; }

  // capacity() returns the capcity (in bytes) of this node
  virtual int capacity(void) { return capacity_; }

  // set the capacity of this node to capacity bytes
  virtual void set_capacity(int capacity) { capacity_ = capacity; }

  // truncate() sets the length of this node to zero
  virtual void Truncate() { len_ = 0; }

  // set_data() sets the length of this node to len
  virtual void set_len(size_t len) { len_ = len; }

  // len() returns the length of this node
  virtual size_t len(void) { return len_; }

  // set_mount() sets the mount to which this node belongs
  virtual void set_mount(AppEngineMount *mount) { mount_ = mount; }

  // stat helper
  virtual void raw_stat(struct stat *buf);

  bool is_dir() { return is_dir_; }
  void set_is_dir(bool is_dir) { is_dir_ = is_dir; }

  void set_path(const std::string& path) { path_ = path; }
  std::string path() { return path_; }

  int use_count(void) { return use_count_; }
  void IncrementUseCount(void) { ++use_count_; }
  void DecrementUseCount(void) { ++use_count_; }

  void set_data(std::vector<char> data) { data_ = data; }
  std::vector<char> data(void) { return data_; }

  int WriteData(off_t offset, const void *buf, size_t count);

 private:
  std::string name_;
  int parent_;
  AppEngineMount *mount_;
  std::vector<char> data_;
  size_t len_;
  size_t capacity_;
  bool is_dir_;
  int use_count_;
  std::string path_;
};

#endif  // PACKAGES_SCRIPTS_FILESYS_MEMORY_APPENGINENODE_H_
