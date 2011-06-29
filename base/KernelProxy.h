/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#ifndef PACKAGES_SCRIPTS_FILESYS_BASE_KERNELPROXY_H_
#define PACKAGES_SCRIPTS_FILESYS_BASE_KERNELPROXY_H_

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include "FileHandle.h"
#include "Mount.h"
#include "PathHandle.h"

class MountManager;

class KernelProxy {

 public:
  ~KernelProxy();
  void Init(MountManager *mm);

  // sys calls handled by mount manager (not mount-specific)
  int chdir(const std::string& path);
  // TODO(arbenson): implement link()
  int link(const std::string& path1, const std::string& path2);
  // TODO(arbenson): implement symlink()
  int symlink(const std::string& path1, const std::string& path2);
  bool getcwd(std::string *buf, size_t size);
  bool getwd(std::string *buf);

  // sys calls that take a path as an argument
  // The mount manager will look for the Node associated to the path.  To
  // find the node, the mount manager calls the corresponding mounts GetNode()
  // method.  The corresponding  method will be called.  If the node
  // cannot be found, errno is set and -1 is returned.
  int chmod(const std::string& path, mode_t mode);
  int remove(const std::string& path);
  int stat(const std::string& path, struct stat *buf);
  int access(const std::string& path, int amode);
  int mkdir(const std::string& path, mode_t mode);
  int rmdir(const std::string& path);
  int open(const std::string& path, int oflag);
  int open(const std::string& path, int oflag, mode_t mode);

  // sys calls that take a file descriptor as an argument
  // The mount manager will look at the registered file handles for the
  // FileHandle corresponding to fd.  If the handle is found
  // the handle's corresonding mount_*() method gets called.  If the handle
  // cannot be found, errno is set and the return value indicates an error
  // in the same way that the sys call would.
  int close(int fd);
  ssize_t read(int fd, void *buf, size_t nbyte);
  ssize_t write(int fd, const void *buf, size_t nbyte);
  int fstat(int fd, struct stat *buf);
  int isatty(int fd);
  int getdents(int fd, void *buf, unsigned int count);
  off_t lseek(int fd, off_t offset, int whence);
  int ioctl(int fd, unsigned long request);
  int fsync(int fd);

 private:
  PathHandle cwd_;
  int max_path_len_;
  MountManager *mm_;

  std::vector<FileHandle*> file_handles_;

  FileHandle *GetFileHandle(int fd);
  FileHandle* OpenHandle(Mount* mount, const std::string& path, int oflag, mode_t mode);
  // Add a file to the mount manager's vector of handles.  This method
  // will assign the file handle's fd and will return that value
  // on success.  On failure, -1 is returned.  Note that file handles
  // are removed when close() is called on a Node
  int RegisterFileHandle(FileHandle *fh);
};

#endif  // PACKAGES_SCRIPTS_FILESYS_BASE_KERNELPROXY_H_
