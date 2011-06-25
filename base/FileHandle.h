/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#ifndef PACKAGES_SCRIPTS_FILESYS_MEMORY_MEMFILEHANDLE_H_
#define PACKAGES_SCRIPTS_FILESYS_MEMORY_MEMFILEHANDLE_H_

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include <string>
#include "../base/FileHandle.h"
#include "../base/Mount.h"
#include "../base/Node.h"

struct dirent {
  ino_t d_ino;
  off_t d_off;
  uint16_t d_reclen;
  char d_name[256];
};

// MemFileHandle is the file handle object for the memory
// mount (MemMount class).  This class overrides all of the
// MountFileHandle sys call methods.  In addition, this
// class contains a corresponding node for the file handle.
class MemFileHandle : public FileHandle {
 public:
  MemFileHandle();
  virtual ~MemFileHandle();

  // override FileHandle system calls
  off_t lseek(off_t offset, int whence);
  ssize_t read(void *buf, size_t nbyte);
  ssize_t write(const void *buf, size_t nbyte);
  int getdents(void *buf, unsigned int count);
  int fstat(struct stat *buf);
  int close(void);
  int ioctl(unsigned long request, ...);

  // set_node() sets the pointer to the mem_node
  // associated with the file handle
  virtual void set_node(Node *node) { node_ = node; }

  virtual Node *node(void) { return node_; }

  // set_flags() sets the flags of this file handle
  void set_flags(int flags) { flags_ = flags; }

  // set_used() sets the used indicator for this
  // file handle
  void set_used(int used) { used_ = used; }

  // set_offset() sets the offset for this
  // file handle
  void set_offset(int offset) { offset_ = offset; }

  void set_mount(Mount *mount) { mount_ = mount; }

 private:
  Mount *mount_;
  Node *node_;
  int used_;
  off_t offset_;
  int flags_;
};

#endif  // PACKAGES_SCRIPTS_FILESYS_MEMORY_MEMFILEHANDLE_H_
