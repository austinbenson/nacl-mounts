/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#ifndef PACKAGES_SCRIPTS_FILESYS_BASE_FILEHANDLE_H_
#define PACKAGES_SCRIPTS_FILESYS_BASE_FILEHANDLE_H_

#include <stdint.h>
#include <stdlib.h>

class Node2;
class Mount;

struct FileHandle {
  Mount *mount;
  Node2 *node;
  off_t offset;
  int flags;
  bool in_use;
};

#endif  // PACKAGES_SCRIPTS_FILESYS_BASE_FILEHANDLE_H_
