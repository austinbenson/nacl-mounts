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
#include "KernelProxy.h"
#include "Mount.h"
#include "PathHandle.h"

class Mount;
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

  KernelProxy *kp() { return &kp_; }

  std::pair<Mount*, Node2*> GetNode(std::string path);

 private:
  std::map<std::string, Mount*> mount_map_;
  KernelProxy kp_;

  Mount *cwd_mount_;

  MountManager();
  static void Instantiate();

  void Init(void);


  static MountManager *mm_instance_;

  // concurrency tools
  void AcquireLock();
  void ReleaseLock();
  pthread_mutex_t lock_;
};

#endif  // PACKAGES_SCRIPTS_FILESYS_BASE_MOUNTMANAGER_H_
