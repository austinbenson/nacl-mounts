/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#include "Entry.h"

int __wrap_chdir(const char *path) {
  return mm->kp()->chdir(path);
}

int __wrap_link(const char *path1, const char *path2) {
  return mm->kp()->link(path1, path2);
}

int __wrap_symlink(const char *path1, const char *path2) {
  return mm->kp()->symlink(path1, path2);
}

static char *to_c(const std::string& b, char *buf) {
  memset(buf, 0, b.length()+1);
  strncpy(buf, b.c_str(), b.length());  
  return buf;
}

char *__wrap_getcwd(char *buf, size_t size) {
  std::string b;
  if (!mm->kp()->getcwd(&b, size-1)) {
    return NULL;
  }
  return to_c(b, buf);
}

char *__wrap_getwd(char *buf) {
  std::string b;
  if (!mm->kp()->getwd(&b) || b.length() >= MAXPATHLEN) {
    return NULL;
  }
  return to_c(b, buf);
}


int __wrap_chmod(const char *path, mode_t mode) {
  return mm->kp()->chmod(path, mode);
}

int __wrap_remove(const char *path) {
  return mm->kp()->remove(path);
}

int __wrap_stat(const char *path, struct stat *buf) {
  return mm->kp()->stat(path, buf);
}

int __wrap_access(const char *path, int amode) {
  return mm->kp()->access(path, amode);
}

int __wrap_mkdir(const char *path, mode_t mode) {
  return mm->kp()->mkdir(path, mode);
}

int __wrap_rmdir(const char *path) {
  return mm->kp()->chdir(path);
}

int __wrap_open(const char *path, int oflag, ...) {
  if (oflag & O_CREAT) {
    va_list argp;
    mode_t mode;
    va_start(argp, oflag);
    mode = va_arg(argp, int);
    va_end(argp);
    return mm->kp()->open(path, oflag, mode);
  }

  return mm->kp()->open(path, oflag);
}


int __wrap_close(int fd) {
  return mm->kp()->close(fd);
}

ssize_t __wrap_read(int fd, void *buf, size_t nbyte) {
  return mm->kp()->read(fd, buf, nbyte);
}

ssize_t __wrap_write(int fd, const void *buf, size_t nbyte) {
  return mm->kp()->write(fd, buf, nbyte);
}

int __wrap_fstat(int fd, struct stat *buf) {
  return mm->kp()->fstat(fd, buf);
}

int __wrap_isatty(int fd) {
  return mm->kp()->isatty(fd);
}

int __wrap_getdents(int fd, void *buf, unsigned int count) {
  return mm->kp()->getdents(fd, buf, count);
}

off_t __wrap_lseek(int fd, off_t offset, int whence) {
  return mm->kp()->lseek(fd, offset, whence);
}

int __wrap_ioctl(int fd, unsigned long request, ...) {
  // TODO(arbenson): handle varargs
  return mm->kp()->ioctl(fd, request);
}

