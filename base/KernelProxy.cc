/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#include "KernelProxy.h"
#include "MountManager.h"

void KernelProxy::Init(MountManager *mm) {
  if (pthread_mutex_init(&lock_, NULL)) assert(false);
  max_path_len_ = 256;
  cwd_.set_is_absolute(true);
  cwd_.SetPath("/");
  mm_ = mm;
}

KernelProxy::~KernelProxy() {
  file_handles_.clear();
}

int KernelProxy::chdir(const char *path) {
  Node *node;
  std::pair<Mount *, std::string> m_and_p;

  node = mm_->GetNode(path);

  // check if node exists
  if (!node) {
    errno = ENOENT;
    return -1;
  }
  // check that node is a directory
  if (!node->is_dir()) {
    errno = ENOTDIR;
    return -1;
  }

  // update path
  AcquireLock();
  std::string p(path);
  PathHandle ph = cwd_;

  if (p.length() == 0)
    return -1;
  if (p[0] == '/')
    ph.SetPath(p);
  else
    ph.AppendPath(p);

  m_and_p = mm_->GetMount(cwd_.FormulatePath());

  if (!(m_and_p.first)) {
    ReleaseLock();
    return -1;
  }

  cwd_ = ph;

  ReleaseLock();
  return 0;
}




int KernelProxy::RegisterFileHandle(FileHandle *fh) {
  size_t fildes;

  AcquireLock();
  // get first available fd
  for (fildes = 0; fildes < file_handles_.size(); ++fildes) {
    if (!(file_handles_[fildes])) {
      fh->set_in_use(true);
      file_handles_[fildes] = fh;
      return fildes;
    } else if (!(file_handles_[fildes]->in_use())) {
      delete file_handles_[fildes];
      fh->set_in_use(true);
      file_handles_[fildes] = fh;
      return fildes;
    }
  }

  file_handles_.push_back(fh);
  fh->set_in_use(true);
  ReleaseLock();
  return fildes;
}

char *KernelProxy::getcwd(char *buf, size_t size) {
  if (size <= 0) {
    errno = EINVAL;
    return NULL;
  }
  AcquireLock();
  if (size < cwd_.FormulatePath().length() + 1) {
    errno = ERANGE;
    ReleaseLock();
    return NULL;
  }
  strncpy(buf, cwd_.FormulatePath().c_str(), size);
  ReleaseLock();
  return buf;
}

char *KernelProxy::getwd(char *buf) {
  return getcwd(buf, max_path_len_);
}

int KernelProxy::link(const char *path1, const char *path2) {
  // check if path1 exists (if not, err)
  // check if path1 is a directory (if, err)
  // check if path2 exists (if, err)
  // check if
  errno = EMLINK;
  fprintf(stderr, "link has not been implemented!\n");
  assert(0);
  return -1;
}

int KernelProxy::symlink(const char *path1, const char *path2) {
  fprintf(stderr, "symlink has not been implemented!\n");
  assert(0);
  return -1;
}

int KernelProxy::open(const char *path, int oflag, ...) {
  FileHandle *handle;
  std::string p(path);

  if (p.length() == 0)
    return -1;

  if (p[0] != '/')
    p = cwd_.FormulatePath() + "/" + p;

  PathHandle ph(p);
  AcquireLock();
  std::pair<Mount *, std::string> m_and_p =
    mm_->GetMount(ph.FormulatePath());

  if (!(m_and_p.first)) {
    errno = ENOENT;
    ReleaseLock();
    return -1;
  } else {
    if (oflag & O_CREAT) {
      va_list argp;
      mode_t mode;
      va_start(argp, oflag);
      mode = va_arg(argp, int);
      va_end(argp);
      handle = m_and_p.first->MountOpen(m_and_p.second,
                                   oflag, mode);
    } else {
      handle = m_and_p.first->MountOpen(m_and_p.second, oflag);
    }
  }
  ReleaseLock();
  return (!handle) ? -1 : RegisterFileHandle(handle);
}

int KernelProxy::close(int fd) {
  FileHandle *handle;

  // check if fd is valid and handle exists
  AcquireLock();
  if (!(handle = GetFileHandle(fd))) {
    errno = EBADF;
    ReleaseLock();
    return -1;
  }
  ReleaseLock();
  return handle->close();
}

ssize_t KernelProxy::read(int fd, void *buf, size_t nbyte) {
  FileHandle *handle;

  AcquireLock();
  // check if fd is valid and handle exists
  if (!(handle = GetFileHandle(fd))) {
    errno = EBADF;
    ReleaseLock();
    return -1;
  }
  int ret = handle->read(buf, nbyte);
  ReleaseLock();
  return ret;
}

ssize_t KernelProxy::write(int fd, const void *buf, size_t nbyte) {
  FileHandle *handle;

  AcquireLock();
  // check if fd is valid and handle exists
  if (!(handle = GetFileHandle(fd))) {
    ReleaseLock();
    errno = EBADF;
    return -1;
  }
  ReleaseLock();
  return handle->write(buf, nbyte);
}

int KernelProxy::fstat(int fd, struct stat *buf) {
  FileHandle *handle;

  AcquireLock();
  // check if fd is valid and handle exists
  if (!(handle = GetFileHandle(fd))) {
    ReleaseLock();
    errno = EBADF;
    return -1;
  }
  ReleaseLock();
  return handle->fstat(buf);
}

int KernelProxy::isatty(int fd) {
  FileHandle *handle;

  AcquireLock();
  // check if fd is valid and handle exists
  if (!(handle = GetFileHandle(fd))) {
    errno = EBADF;
    ReleaseLock();
    return -1;
  }
  ReleaseLock();
  return handle->isatty();
}

int KernelProxy::ioctl(int fd, unsigned long request, ...) {
  FileHandle *handle;
  AcquireLock();
  // check if fd is valid and handle exists
  if (!(handle = GetFileHandle(fd))) {
    ReleaseLock();
    errno = EBADF;
    return -1;
  }
  // TODO(arbenson): 3rd argument not necessarily required
  va_list argp;
  void *p;
  va_start(argp, request);
  p = va_arg(argp, void *);
  va_end(argp);
  ReleaseLock();
  return handle->ioctl(request, p);
}

int KernelProxy::getdents(int fd, void *buf, unsigned int count) {
  FileHandle *handle;

  AcquireLock();
  // check if fd is valid and handle exists
  if (!(handle = GetFileHandle(fd))) {
    ReleaseLock();
    errno = EBADF;
    return -1;
  }
  ReleaseLock();
  return handle->getdents(buf, count);
}

off_t KernelProxy::lseek(int fd, off_t offset, int whence) {
  FileHandle *handle;

  AcquireLock();
  // check if fd is valid and handle exists
  if (!(handle = GetFileHandle(fd))) {
    ReleaseLock();
    errno = EBADF;
    return -1;
  }
  ReleaseLock();
  return handle->lseek(offset, whence);
}

int KernelProxy::chmod(const char *path, mode_t mode) {
  Node *node;

  AcquireLock();
  node = mm_->GetNode(path);
  if (!node) {
    errno = ENOENT;
    ReleaseLock();
    return -1;
  }
  ReleaseLock();
  return node->chmod(mode);
}

int KernelProxy::remove(const char *path) {
  Node *node;

  AcquireLock();
  node = mm_->GetNode(path);
  if (!node) {
    errno = ENOENT;
    ReleaseLock();
    return -1;
  }
  ReleaseLock();
  return node->remove();
}

int KernelProxy::stat(const char *path, struct stat *buf) {
  Node *node;

  AcquireLock();
  node = mm_->GetNode(path);
  if (!node) {
    errno = ENOENT;
    ReleaseLock();
    return -1;
  }
  ReleaseLock();
  return node->stat(buf);
}

int KernelProxy::access(const char *path, int amode) {
  Node *node;

  AcquireLock();
  node = mm_->GetNode(path);
  if (!node) {
    errno = ENOENT;
    ReleaseLock();
    return -1;
  }
  ReleaseLock();
  return node->access(amode);
}

int KernelProxy::mkdir(const char *path, mode_t mode) {
  AcquireLock();
  std::string p(path);
  if (p.length() == 0)
    return -1;

  PathHandle ph = cwd_;
  if (p[0] == '/')
    ph.SetPath(p);
  else
    ph.AppendPath(p);

  std::pair<Mount *, std::string> m_and_p =
    mm_->GetMount(ph.FormulatePath());
  if (!(m_and_p.first)) {
    errno = ENOTDIR;
    ReleaseLock();
    return -1;
  } else {
    ReleaseLock();
    return m_and_p.first->mkdir(m_and_p.second, mode);
  }
}

int KernelProxy::rmdir(const char *path) {
  Node *node;

  AcquireLock();
  node = mm_->GetNode(path);
  if (!node) {
    errno = ENOENT;
    ReleaseLock();
    return -1;
  }
  ReleaseLock();
  return node->rmdir();
}

void KernelProxy::AcquireLock(void) {
  if (pthread_mutex_lock(&lock_)) assert(0);
}

void KernelProxy::ReleaseLock(void) {
  if (pthread_mutex_unlock(&lock_)) assert(0);
}

FileHandle *KernelProxy::GetFileHandle(int fd) {
  if (fd < 0) return NULL;
  if (static_cast<size_t>(fd) + 1 > file_handles_.size()) return NULL;
  return file_handles_[fd];
}

