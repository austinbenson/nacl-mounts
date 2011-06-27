/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#ifndef PACKAGES_SCRIPTS_FILESYS_BASE_MOUNT_H_
#define PACKAGES_SCRIPTS_FILESYS_BASE_MOUNT_H_

#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include "PathHandle.h"
#include "Node2.h"

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
  virtual Node2 *GetNode(std::string path) { return NULL; }

  // MountOpen() opens a node at path (representing the open sys call).
  // MountOpen() is also responsible for node creation when the value
  // of oflag indicates such.
  virtual Node2 *MountOpen(std::string path, int oflag, mode_t mode) { return 0; }

  // mkdir() creates a directory at path (representing the mkdir
  // sys call).
  virtual int mkdir(std::string path, mode_t mode) { return 0; }

  virtual bool is_dir(Node2* node) { return false; }
  virtual size_t len(Node2* node) { return 0; }
  virtual int chmod(Node2* node, mode_t mode) { return 0; }
  virtual int stat(Node2* node, struct stat *buf) { return 0; }
  virtual int remove(Node2* node) { return 0; }
  virtual int access(Node2* node, int amode) { return 0; }
  virtual int rmdir(Node2* node) { return 0; }
  virtual void raw_stat(Node2* node, struct stat *buf) {}
  virtual void DecrementUseCount(Node2* node) { }
  virtual std::string name(Node2* node) { return ""; }
  virtual int Getdents(Node2* node, off_t offset,
                       struct dirent *dirp, unsigned int count) { return -1; }

  virtual ssize_t Read(Node2* node, off_t offset, void *buf, size_t count) { return -1; }
  virtual ssize_t Write(Node2* node, off_t offset, const void *buf, size_t count) { return -1; }

  virtual void AcquireLock(void) {}
  virtual void ReleaseLock(void) {}
};

class Mount2 {
 public:
  Mount2() {}
  virtual ~Mount2() {}

  virtual ino_t GetNode(const std::string& path) = 0;

  virtual void Ref(ino_t inode) = 0;
  virtual void Unref(ino_t inode) = 0;

  virtual ino_t Creat(ino_t parent, const std::string& name, mode_t mode) = 0;
  virtual ino_t Mkdir(ino_t parent, const std::string* name, mode_t mode) = 0;

  virtual int Unlink(ino_t inode) = 0;
  virtual int Rmdir(ino_t inode) = 0;

  virtual int Stat(ino_t inode, struct stat* buf) = 0;
  virtual int Getdents(ino_t inode, off_t offset, struct dirent *dirp, unsigned int count) = 0;

  virtual ssize_t Read(ino_t inode, off_t offset, void *buf, size_t count) = 0;
  virtual ssize_t Write(ino_t inode, off_t offset, void *buf, size_t count) = 0;
  virtual int Sync(ino_t inode) = 0;

};

#endif  // PACKAGES_SCRIPTS_FILESYS_BASE_MOUNT_H_
