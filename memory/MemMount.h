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

class Node;

// mem_mount is a storage mount representing local memory.  The node
// class is MemNode.
class MemMount: public Mount {
 public:
  MemMount();
  ~MemMount();

  // open() opens a new node at path and mimics the
  // open sys call.  open() covers node creation, so
  // the method is in MemMount and not Node
  Node2 *MountOpen(std::string path, int oflag, mode_t mode);

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
  Node *GetParentNode(std::string path);

  // GetMemNode() is like GetNode(), but the method
  // is used internally to the memory mount structure.
  Node *GetMemNode(std::string path);

  // GetMemParentNode() is like GetParentNode(), but
  // the method is used internally to the memory mount
  // structure.
  Node *GetParentMemNode(std::string path);

  // Temp methods for Node -> Node2 -> ino_t transition period.
  bool is_dir(Node2* node);
  size_t len(Node2* node);
  int capacity(Node2 *node);
  int chmod(Node2* node, mode_t mode);
  int stat(Node2* node, struct stat *buf);
  int remove(Node2* node);
  int access(Node2* node, int amode);
  int rmdir(Node2* node);
  void set_len(Node2* node, size_t len);
  void ReallocData(Node2* node, int len);
  void raw_stat(Node2* node, struct stat *buf);
  void DecrementUseCount(Node2* node);
  int Getdents(Node2* node, off_t offset, struct dirent *dirp, unsigned int count);
  std::string name(Node2* node);
  char *data(Node2* node);

  Node *root() { return root_; }

  // concurrency tools
  virtual void AcquireLock(void);
  virtual void ReleaseLock(void);

 private:
  PathHandle *path_handle_;
  pthread_mutex_t lock_;
  Node *root_;
};

#endif  // PACKAGES_SCRIPTS_FILESYS_MEMORY_MEMMOUNT_H_
