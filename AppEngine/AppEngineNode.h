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
#include "AppEngineMount.h"

class AppEngineMount;
class AppEngineNode;

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

  // stat helper
  virtual void raw_stat(struct stat *buf);

  bool is_dir() { return is_dir_; }
  void set_is_dir(bool is_dir) { is_dir_ = is_dir; }

  void set_path(const std::string& path) { path_ = path; }
  std::string path() { return path_; }

  void set_mount(AppEngineMount *mount) { mount_ = mount; }

 private:
  std::string name_;
  char *data_;
  size_t len_;
  size_t capacity_;
  int use_count_;
  bool is_dir_;
  AppEngineMount *mount_;
  std::list<AppEngineNode *> children_;
  std::string path_;
};

class AppEngineNode2 : public Node2 {
 public:
  explicit AppEngineNode2(AppEngineNode* node) : node_(node) {
  }

  virtual ~AppEngineNode2() {
    delete node_;
  }

  AppEngineNode* node() { return node_; }

 private:
  AppEngineNode* node_;

  DISALLOW_COPY_AND_ASSIGN(AppEngineNode2);
};


#endif  // PACKAGES_SCRIPTS_FILESYS_APPENGINE_APPENGINENODE_H_
