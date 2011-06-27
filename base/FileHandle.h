/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#ifndef PACKAGES_SCRIPTS_FILESYS_BASE_FILEHANDLE_H_
#define PACKAGES_SCRIPTS_FILESYS_BASE_FILEHANDLE_H_

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include <string>

#include "dirent.h"
#include "Node2.h"


class Mount;

// FileHandle is the file handle object for the memory
// mount (MemMount class).  This class overrides all of the
// MountFileHandle sys call methods.  In addition, this
// class contains a corresponding node for the file handle.
class FileHandle {
 public:
  Mount *mount;
  Node2 *node;
  off_t offset;
  int flags;

  FileHandle();
  virtual ~FileHandle();

  int close(void);

  // set_used() sets the used indicator for this
  // file handle
  void set_used(int used) { used_ = used; }

  void set_in_use(bool in_use) { in_use_ = in_use; }
  bool in_use(void) { return in_use_; }

 private:

  int used_;
  bool in_use_;
};

#endif  // PACKAGES_SCRIPTS_FILESYS_BASE_FILEHANDLE_H_
