/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#ifndef PACKAGES_SCRIPTS_FILESYS_BASE_FILEHANDLE_H_
#define PACKAGES_SCRIPTS_FILESYS_BASE_FILEHANDLE_H_

#include <unistd.h>

// FileHandle is the base class for implementing file handles for
// a particular mount.  FileHandle is designed to intercept sys
// calls that take a file descriptor as an argument.
// The implementations here are all do-nothing stubs. A mount file handle
// class that inherits this class will want to override important functions
// like read() and open().  However, for a read-only mount,
// write() would not need to be overridden.
class FileHandle {
 public:
  FileHandle() {}
  virtual ~FileHandle() {}

  // The following sys calls are called by the mount manager
  // When implementing a new mount, nodes should override
  // these methods as appropriate.
  virtual off_t lseek(off_t offset, int whence) { return 0; }
  virtual int close() { return 0; }
  virtual ssize_t read(void *buf, size_t nbyte) { return 0; }
  virtual ssize_t write(const void *buf, size_t nbyte) { return 0; }
  virtual int fstat(struct stat *buf) { return 0; }
  virtual int isatty() { return 0; }
  virtual int ioctl(unsigned long request, ...) { return 0; }
  virtual int getdents(void *buf, unsigned int count) { return 0; }

  void set_in_use(bool in_use) { in_use_ = in_use; }
  bool in_use(void) { return in_use_; }

 protected:
  bool in_use_;
};

#endif  // PACKAGES_SCRIPTS_FILESYS_BASE_FILEHANDLE_H_

