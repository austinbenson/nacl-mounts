/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#ifndef PACKAGES_SCRIPTS_FILESYS_MEMORY_MEMMOUNT_H_
#define PACKAGES_SCRIPTS_FILESYS_MEMORY_MEMMOUNT_H_

#include <list>
#include <string>
#include "../base/Mount.h"
#include "../base/PathHandle.h"
#include "../base/SlotAllocator.h"
#include "MemNode.h"

// mem_mount is a storage mount representing local memory.  The node
// class is MemNode.
class MemMount: public Mount {
 public:
  MemMount();
  virtual ~MemMount() {}

  void Ref(ino_t node);
  void Unref(ino_t node);

  int Creat(const std::string& path, mode_t mode, struct stat* st);
  int Mkdir(const std::string& path, mode_t mode, struct stat* st);
  int GetNode(const std::string& path, struct stat* st);

  int Unlink(const std::string& path);
  int Rmdir(ino_t node);

  // Given a path, GetParentNode returns the parent
  // of the Node located at path.  If path is not a valid
  // path for a Node, NULL is returned.
  MemNode *GetParentNode(std::string path);

  // GetMemNode() is like GetNode(), but the method
  // is used internally to the memory mount structure.
  MemNode *GetMemNode(std::string path);
  int GetSlot(std::string path);

  MemNode *ToMemNode(ino_t node) {
    return slots_.At(node);
  }

  // GetMemParentNode() is like GetParentNode(), but
  // the method is used internally to the memory mount
  // structure.
  MemNode *GetParentMemNode(std::string path);
  int GetParentSlot(std::string path);

  int Chmod(ino_t node, mode_t mode);
  int Stat(ino_t node, struct stat *buf);

  int Getdents(ino_t node, off_t offset, struct dirent *dirp, unsigned int count);

  virtual ssize_t Read(ino_t node, off_t offset, void *buf, size_t count);
  virtual ssize_t Write(ino_t node, off_t offset, const void *buf, size_t count);

  MemNode *root() { return root_; }

 private:
  PathHandle *path_handle_;
  MemNode *root_;
  SlotAllocator<MemNode> slots_;
};

#endif  // PACKAGES_SCRIPTS_FILESYS_MEMORY_MEMMOUNT_H_
