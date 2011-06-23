/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#include "Entry.h"

int __wrap_chdir(const char *path) {
  return mm->chdir(path);
}

int __wrap_link(const char *path1, const char *path2) {
  return mm->link(path1, path2);
}

int __wrap_symlink(const char *path1, const char *path2) {
  return mm->symlink(path1, path2);
}

char *__wrap_getcwd(char *buf, size_t size) {
  return mm->getcwd(buf, size);
}

char *__wrap_getwd(char *buf) {
  return mm->getwd(buf);
}


int __wrap_chmod(const char *path, mode_t mode) {
  return mm->chmod(path, mode);
}

int __wrap_remove(const char *path) {
  return mm->remove(path);
}

int __wrap_stat(const char *path, struct stat *buf) {
  return mm->stat(path, buf);
}

int __wrap_access(const char *path, int amode) {
  return mm->access(path, amode);
}

int __wrap_mkdir(const char *path, mode_t mode) {
  return mm->mkdir(path, mode);
}

int __wrap_rmdir(const char *path) {
  return mm->chdir(path);
}

int __wrap_open(const char *path, int oflag, ...) {
  if (oflag & O_CREAT) {
    va_list argp;
    mode_t mode;
    va_start(argp, oflag);
    mode = va_arg(argp, int);
    va_end(argp);
    return mm->open(path, oflag, mode);
  }

  return mm->open(path, oflag);
}


int __wrap_close(int fd) {
  return mm->close(fd);
}

ssize_t __wrap_read(int fd, void *buf, size_t nbyte) {
  return mm->read(fd, buf, nbyte);
}

ssize_t __wrap_write(int fd, const void *buf, size_t nbyte) {
  return mm->write(fd, buf, nbyte);
}

int __wrap_fstat(int fd, struct stat *buf) {
  return mm->fstat(fd, buf);
}

int __wrap_isatty(int fd) {
  return mm->isatty(fd);
}

int __wrap_getdents(int fd, void *buf, unsigned int count) {
  return mm->getdents(fd, buf, count);
}

off_t __wrap_lseek(int fd, off_t offset, int whence) {
  return mm->lseek(fd, offset, whence);
}

int __wrap_ioctl(int fd, unsigned long request, ...) {
  va_list argp;
  void *p;
  va_start(argp, request);
  p = va_arg(argp, void *);
  va_end(argp);
  return mm->ioctl(fd, request, p);
}

