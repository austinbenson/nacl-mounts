/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#ifndef PACKAGES_SCRIPTS_FILESYS_BASE_MOUNTMANAGER_H_
#define PACKAGES_SCRIPTS_FILESYS_BASE_MOUNTMANAGER_H_

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
#include "../memory/MemMount.h"
#include "FileHandle.h"
#include "Mount.h"
#include "Node.h"
#include "PathHandle.h"

class Mount;
class FileHandle;
// MountManager serves as an indirection layer between libc and IRT.  Different
// mount types can be added to the mount manager so that different sys call
// implementations can be used at different locations in the file system.
// This allows for the use of different backend storage devices to be used
// by one native client executable.
class MountManager {
 public:
  ~MountManager();
  static MountManager *MMInstance();

  // Add a new mount type in the file system.  Starting at path, sys calls
  // are determined by the implementation in mount m.  The mount manager
  // is responsible for deleting all mounts that are added.
  // Return value is:
  // 0 on success
  // -1 on failure due to another mount rooted at path
  // -2 on failure due to a bad (NULL) mount provided
  // -3 on failure due to a bad path provided
  int AddMount(Mount *m, const char *path);

  // Remove a mount type from the file system.  RemoveMount() will try to
  // remove a mount rooted at path.  If there is no mount rooted at path,
  // then -1 is returned.  On success, 0 is returned.
  int RemoveMount(const char *path);

  // Given an absolute path, GetMount() will
  // return the mount at that location.
  std::pair<Mount *, std::string> GetMount(std::string path);

  // Remove all mounts that have been added.  The destructors
  // of these mounts will be called.
  void ClearMounts(void);

  // sys calls handled by mount manager (not mount-specific)
  int chdir(const char *path);
  // TODO(arbenson): implement link()
  int link(const char *path1, const char *path2);
  // TODO(arbenson): implement symlink()
  int symlink(const char *path1, const char *path2);
  char *getcwd(char *buf, size_t size);
  char *getwd(char *buf);

  // sys calls that take a path as an argument
  // The mount manager will look for the Node associated to the path.  To
  // find the node, the mount manager calls the corresponding mounts GetNode()
  // method.  The corresponding  method will be called.  If the node
  // cannot be found, errno is set and -1 is returned.
  int chmod(const char *path, mode_t mode);
  int remove(const char *path);
  int stat(const char *path, struct stat *buf);
  int access(const char *path, int amode);
  int mkdir(const char *path, mode_t mode);
  int rmdir(const char *path);
  int open(const char *path, int oflag, ...);

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
  int ioctl(int fd, unsigned long request, ...);

 private:
  std::map<std::string, Mount*> mount_map_;
  std::map<std::string, std::string> symlinks_;
  std::vector<FileHandle*> file_handles_;
  Mount *cwd_mount_;
  PathHandle cwd_;

  MountManager();
  static void Instantiate();

  // Add a file to the mount manager's vector of handles.  This method
  // will assign the file handle's fd and will return that value
  // on success.  On failure, -1 is returned.  Note that file handles
  // are removed when close() is called on a Node
  int RegisterFileHandle(FileHandle *fh);

  void Init(void);
  Node *GetNode(std::string path);
  FileHandle *GetFileHandle(int fd);

  int max_path_len_;
  static MountManager *mm_instance_;

  // concurrency tools
  void AcquireLock();
  void ReleaseLock();
  pthread_mutex_t lock_;
};

#endif  // PACKAGES_SCRIPTS_FILESYS_BASE_MOUNTMANAGER_H_

