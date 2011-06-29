/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#ifndef PACKAGES_SCRIPTS_FILESYS_BASE_FILEHANDLE_H_
#define PACKAGES_SCRIPTS_FILESYS_BASE_FILEHANDLE_H_

#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>

class Mount;

struct FileDescriptor {
  // An index in open_files_ table
  int handle;
};

struct FileHandle {
  Mount *mount;
  ino_t node;
  off_t offset;
  int flags;
  int use_count;
};

#endif  // PACKAGES_SCRIPTS_FILESYS_BASE_FILEHANDLE_H_
