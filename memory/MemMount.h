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
#include "../base/Node.h"
#include "../base/PathHandle.h"
#include "../base/FileHandle.h"
#include "MemNode.h"

class MemNode;

// mem_mount is a storage mount representing local memory.  The node
// class is MemNode and the file handle class is FileHandle.
class MemMount: public Mount {
 public:
  MemMount();
  ~MemMount();

  // open() opens a new node at path and mimics the
  // open sys call.  open() covers node creation, so
  // the method is in MemMount and not Node
  FileHandle *MountOpen(std::string path, int oflag, ...);

  // mkdir() creates a new directory at path
  // and mimics the mkdir sys call.  This mkdir
  // implementation ignores the mode parameter.
  int mkdir(std::string path, mode_t mode);

  // Given a path, GetNode() returns the Node
  // corresponding to that path.  The MemMounnt class
  // will return a MemNode (subclass of MountNode).
  // If a node cannot be found at path, NULL is returned.
  Node *GetNode(std::string path);

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
