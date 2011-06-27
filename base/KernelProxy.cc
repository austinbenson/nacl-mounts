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

int KernelProxy::chdir(const std::string& path) {
  std::pair<Mount *, std::string> m_and_p;

  // not supporting empty paths right now
  if (path.empty()) {
    return -1;
  }

  PathHandle ph = cwd_;
  if (path[0] == '/') {
    ph.SetPath(path);
  } else {
    ph.AppendPath(path);
  }

  std::pair<Mount*, Node2*> mnode = mm_->GetNode(ph.FormulatePath());

  // check if node exists
  if (!mnode.first) {
    errno = ENOENT;
    return -1;
  }
  // check that node is a directory
  if (!mnode.first->is_dir(mnode.second)) {
    errno = ENOTDIR;
    return -1;
  }

  // update path
  AcquireLock();

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

bool KernelProxy::getcwd(std::string *buf, size_t size) {
  if (size <= 0) {
    errno = EINVAL;
    return false;
  }
  AcquireLock();
  if (size < cwd_.FormulatePath().length()) {
    errno = ERANGE;
    ReleaseLock();
    return false;
  }
  *buf = cwd_.FormulatePath();
  ReleaseLock();
  return true;
}

bool KernelProxy::getwd(std::string *buf) {
  return getcwd(buf, max_path_len_);
}

int KernelProxy::link(const std::string& path1, const std::string& path2) {
  // check if path1 exists (if not, err)
  // check if path1 is a directory (if, err)
  // check if path2 exists (if, err)
  // check if
  errno = EMLINK;
  fprintf(stderr, "link has not been implemented!\n");
  assert(0);
  return -1;
}

int KernelProxy::symlink(const std::string& path1, const std::string& path2) {
  fprintf(stderr, "symlink has not been implemented!\n");
  assert(0);
  return -1;
}

FileHandle* KernelProxy::OpenHandle(Mount* mount, const std::string& path,
                                    int oflag, mode_t mode) {
  Node2* node = mount->MountOpen(path, oflag, mode);
  if (node == NULL) {
    return NULL;
  }
  // Setup file handle.
  FileHandle* handle = new FileHandle();
  handle->set_mount(mount);
  handle->set_node(node);
  handle->set_flags(oflag);
  handle->set_used(1);

  if (oflag & O_APPEND) {
    handle->set_offset(mount->len(node));
  } else {
    handle->set_offset(0);
  }

  return handle;
}

int KernelProxy::open(const std::string& path, int oflag) {
  FileHandle *handle;
  std::string p = path;
  if (p == "") {
    return -1;
  }

  if (p[0] != '/') {
    p = cwd_.FormulatePath() + "/" + p;
  }

  PathHandle ph(p);
  AcquireLock();
  std::pair<Mount *, std::string> m_and_p =
    mm_->GetMount(ph.FormulatePath());

  if (!(m_and_p.first)) {
    errno = ENOENT;
    ReleaseLock();
    return -1;
  } else {
    handle = OpenHandle(m_and_p.first, m_and_p.second, oflag, 0);
  }
  ReleaseLock();
  return (!handle) ? -1 : RegisterFileHandle(handle);
}

int KernelProxy::open(const std::string& path, int oflag, mode_t mode) {
  FileHandle *handle;
  std::string p = path;
  if (p == "") {
    return -1;
  }

  if (p[0] != '/') {
    p = cwd_.FormulatePath() + "/" + p;
  }

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
      handle = OpenHandle(m_and_p.first, m_and_p.second, oflag, mode);
    } else {
      handle = OpenHandle(m_and_p.first, m_and_p.second, oflag, 0);
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

int KernelProxy::ioctl(int fd, unsigned long request) {
  errno = ENOSYS;
  return -1;
}

int KernelProxy::getdents(int fd, void *buf, unsigned int count) {
  FileHandle *handle;
  Mount* mount;
  Node2 *node;

  AcquireLock();
  // check if fd is valid and handle exists
  if (!(handle = GetFileHandle(fd))) {
    ReleaseLock();
    errno = EBADF;
    return -1;
  }
  mount = handle->mount();
  node = handle->node();
  ReleaseLock();
  // TODO(Krasin): support offset for getdents
  return mount->Getdents(node, 0, (struct dirent*)buf, count);
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

int KernelProxy::chmod(const std::string& path, mode_t mode) {
  AcquireLock();
  std::pair<Mount*, Node2*> mnode = mm_->GetNode(path);
  if (!mnode.first) {
    errno = ENOENT;
    ReleaseLock();
    return -1;
  }
  ReleaseLock();
  return mnode.first->chmod(mnode.second, mode);
}

int KernelProxy::remove(const std::string& path) {
  AcquireLock();
  std::pair<Mount*, Node2*> mnode = mm_->GetNode(path);
  if (!mnode.first) {
    errno = ENOENT;
    ReleaseLock();
    return -1;
  }
  ReleaseLock();
  return mnode.first->remove(mnode.second);
}

int KernelProxy::stat(const std::string& path, struct stat *buf) {
  AcquireLock();
  std::pair<Mount*, Node2*> mnode = mm_->GetNode(path);
  if (!mnode.first) {
    errno = ENOENT;
    ReleaseLock();
    return -1;
  }
  ReleaseLock();
  return mnode.first->stat(mnode.second, buf);
}

int KernelProxy::access(const std::string& path, int amode) {
  AcquireLock();
  std::pair<Mount*, Node2*> mnode = mm_->GetNode(path);
  if (!mnode.first) {
    errno = ENOENT;
    ReleaseLock();
    return -1;
  }
  ReleaseLock();
  return mnode.first->access(mnode.second, amode);
}

int KernelProxy::mkdir(const std::string& path, mode_t mode) {
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

int KernelProxy::rmdir(const std::string& path) {
  AcquireLock();
  std::pair<Mount*, Node2*> mnode = mm_->GetNode(path);
  if (!mnode.first) {
    errno = ENOENT;
    ReleaseLock();
    return -1;
  }
  ReleaseLock();
  return mnode.first->rmdir(mnode.second);
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
