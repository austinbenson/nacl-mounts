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

class MemNode;

// mem_mount is a storage mount representing local memory.  The node
// class is MemNode.
class MemMount: public Mount {
 public:
  MemMount();
  ~MemMount();

  Node2 *Creat(std::string path, mode_t mode);

  // mkdir() creates a new directory at path
  // and mimics the mkdir sys call.  This mkdir
  // implementation ignores the mode parameter.
  int mkdir(std::string path, mode_t mode);

  // Given a path, GetNode() returns the Node
  // corresponding to that path.  The MemMounnt class
  // will return a MemNode (subclass of MountNode).
  // If a node cannot be found at path, NULL is returned.
  Node2 *GetNode(std::string path);

  // Given a path, GetParentNode returns the parent
  // of the Node located at path.  If path is not a valid
  // path for a Node, NULL is returned.
  MemNode *GetParentNode(std::string path);

  // GetMemNode() is like GetNode(), but the method
  // is used internally to the memory mount structure.
  MemNode *GetMemNode(std::string path);

  MemNode *ToMemNode(Node2* node);

  // GetMemParentNode() is like GetParentNode(), but
  // the method is used internally to the memory mount
  // structure.
  MemNode *GetParentMemNode(std::string path);

  // Temp methods for Node -> Node2 -> ino_t transition period.
  bool is_dir(Node2* node);
  int chmod(Node2* node, mode_t mode);
  int stat(Node2* node, struct stat *buf);
  int remove(Node2* node);
  int access(Node2* node, int amode);
  int rmdir(Node2* node);
  void DecrementUseCount(Node2* node);
  int Getdents(Node2* node, off_t offset, struct dirent *dirp, unsigned int count);

  virtual ssize_t Read(Node2* node, off_t offset, void *buf, size_t count);
  virtual ssize_t Write(Node2* node, off_t offset, const void *buf, size_t count);

  MemNode *root() { return root_; }

  // concurrency tools
  virtual void AcquireLock(void);
  virtual void ReleaseLock(void);

 private:
  PathHandle *path_handle_;
  pthread_mutex_t lock_;
  MemNode *root_;
};

#endif  // PACKAGES_SCRIPTS_FILESYS_MEMORY_MEMMOUNT_H_
