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

// Mount serves as the base mounting class that will be used by
// the mount manager (class MountManager).  The mount manager
// relies heavily on the GetNode method as a way of directing
// sys calls that take a path as an argument.
class Mount {
 public:
  Mount() {}
  virtual ~Mount() {}

  virtual int GetNode(const std::string& path, struct stat *st) { return -1; }

  virtual void Ref(ino_t node) {}
  virtual void Unref(ino_t node) {}

  virtual int Creat(const std::string& path, mode_t mode, struct stat* st) { return -1; }
  virtual int Mkdir(const std::string& path, mode_t mode, struct stat* st) { return -1; }

  virtual int Unlink(const std::string& path) { return -1; }
  virtual int Rmdir(ino_t node) { return -1; }


  virtual int Chmod(ino_t node, mode_t mode) { return -1; }
  virtual int Stat(ino_t node, struct stat *buf) { return -1; }

  virtual int Fsync(ino_t node) { return -1; }

  virtual int Getdents(ino_t node, off_t offset,
                       struct dirent *dirp, unsigned int count) { return -1; }

  virtual ssize_t Read(ino_t node, off_t offset, void *buf, size_t count) { return -1; }
  virtual ssize_t Write(ino_t node, off_t offset, const void *buf, size_t count) { return -1; }

};

#endif  // PACKAGES_SCRIPTS_FILESYS_BASE_MOUNT_H_
