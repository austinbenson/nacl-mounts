/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#ifndef PACKAGES_SCRIPTS_FILESYS_BASE_MOUNT_H_
#define PACKAGES_SCRIPTS_FILESYS_BASE_MOUNT_H_

#include <fcntl.h>
#include <string>
#include "FileHandle.h"
#include "Node.h"
#include "PathHandle.h"

// Mount serves as the base mounting class that will be used by
// the mount manager (class MountManager).  The mount manager
// relies heavily on the GetNode method as a way of directing
// sys calls that take a path as an argument.
class Mount {
 public:
  Mount() {}
  virtual ~Mount() {}

  // GetNode() returns the node at path.  It returns NULL if no
  // node is at path.
  virtual Node *GetNode(std::string path) { return NULL; }

  // MountOpen() opens a node at path (representing the open sys call).
  // MountOpen() is also responsible for node creation when the value
  // of oflag indicates such.
  virtual FileHandle *MountOpen(std::string path, int oflag, ...) { return 0; }

  // mkdir() creates a directory at path (representing the mkdir
  // sys call).
  virtual int mkdir(std::string path, mode_t mode) { return 0; }
};
#endif  // PACKAGES_SCRIPTS_FILESYS_BASE_MOUNT_H_

