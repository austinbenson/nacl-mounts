/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#include "MountManager.h"

static pthread_once_t mount_manager_once_ = PTHREAD_ONCE_INIT;
MountManager *MountManager::mm_instance_;

MountManager::MountManager() {
  Init();
}

MountManager::~MountManager() {
  mount_map_.clear();
  symlinks_.clear();
  file_handles_.clear();
}

MountManager *MountManager::MMInstance() {
  pthread_once(&mount_manager_once_, Instantiate);
  return mm_instance_;
}

void MountManager::Instantiate() {
  if (!mm_instance_)
    mm_instance_ = new MountManager();
}

int MountManager::chdir(const char *path) {
  Node *node;
  std::pair<Mount *, std::string> m_and_p;

  node = GetNode(path);

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

  m_and_p = GetMount(cwd_.FormulatePath());

  if (!(m_and_p.first)) {
    ReleaseLock();
    return -1;
  }

  cwd_mount_ = m_and_p.first;
  cwd_ = ph;

  ReleaseLock();
  return 0;
}

void MountManager::Init() {
  if (pthread_mutex_init(&lock_, NULL)) assert(false);
  max_path_len_ = 256;
  // add a memory mount at "/" to function
  // as the deafault mount
  MemMount *default_mount = new MemMount();
  assert(default_mount);
  AddMount(default_mount, "/");
  cwd_.set_is_absolute(true);
  cwd_.SetPath("/");
  cwd_mount_ = default_mount;
}

int MountManager::AddMount(Mount *m, const char *path) {
  if (!path)
    return -3;
  std::string p(path);
  AcquireLock();
  Mount *mount = mount_map_[path];
  if (mount) {
    ReleaseLock();
    return -1;
  }
  if (!m) {
    ReleaseLock();
    return -2;
  }
  if (p.length() == 0) {
    ReleaseLock();
    return -3;
  }
  mount_map_[path] = m;
  if (!cwd_mount_) {
    cwd_.SetPath(path);
    cwd_mount_ = m;
  }
  ReleaseLock();
  return 0;
}

int MountManager::RemoveMount(const char *path) {
  AcquireLock();
  std::string p(path);
  std::map<std::string, Mount *>::iterator it;
  it = mount_map_.find(p);
  if (it == mount_map_.end()) {
    ReleaseLock();
    return -1;
  } else {
    if (cwd_mount_ == it->second)
      cwd_mount_ = NULL;
    // erase() calls the destructor
    mount_map_.erase(it);
    ReleaseLock();
    return 0;
  }
}

void MountManager::ClearMounts(void) {
  mount_map_.clear();
  cwd_mount_ = NULL;
}

int MountManager::RegisterFileHandle(FileHandle *fh) {
  size_t fildes;

  AcquireLock();
  // get first available fd
  for (fildes = 0; fildes < file_handles_.size(); ++fildes) {
    if (!(file_handles_[fildes])) {
      fh->set_fd(fildes);
      fh->set_in_use(true);
      file_handles_[fildes] = fh;
      return fildes;
    } else if (!(file_handles_[fildes]->in_use())) {
      delete file_handles_[fildes];
      fh->set_fd(fildes);
      fh->set_in_use(true);
      file_handles_[fildes] = fh;
      return fildes;
    }
  }

  file_handles_.push_back(fh);
  fh->set_fd(fildes);
  fh->set_in_use(true);
  ReleaseLock();
  return fildes;
}

char *MountManager::getcwd(char *buf, size_t size) {
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
  strncpy(buf, cwd_.FormulatePath().c_str(), max_path_len_);
  ReleaseLock();
  return buf;
}

char *MountManager::getwd(char *buf) {
  return getcwd(buf, max_path_len_);
}

int MountManager::link(const char *path1, const char *path2) {
  // check if path1 exists (if not, err)
  // check if path1 is a directory (if, err)
  // check if path2 exists (if, err)
  // check if
  errno = EMLINK;
  fprintf(stderr, "link has not been implemented!\n");
  assert(0);
  return -1;
}

int MountManager::symlink(const char *path1, const char *path2) {
  symlinks_[path1] = path2;
  fprintf(stderr, "symlink has not been implemented!\n");
  assert(0);
  return -1;
}

int MountManager::open(const char *path, int oflag, ...) {
  FileHandle *handle;
  std::string p(path);

  if (p.length() == 0)
    return -1;
  
  if (p[0] != '/')
    p = cwd_.FormulatePath() + "/" + p;

  PathHandle ph(p);

  AcquireLock();
  
  std::pair<Mount *, std::string> m_and_p =
    GetMount(ph.FormulatePath());
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

int MountManager::close(int fd) {
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

ssize_t MountManager::read(int fd, void *buf, size_t nbyte) {
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

ssize_t MountManager::write(int fd, const void *buf, size_t nbyte) {
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

int MountManager::fstat(int fd, struct stat *buf) {
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

int MountManager::isatty(int fd) {
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

int MountManager::ioctl(int fd, unsigned long request, ...) {
  FileHandle *handle;
  AcquireLock();
  // check if fd is valid and handle exists
  if (!(handle = GetFileHandle(fd))) {
    ReleaseLock();
    errno = EBADF;
    return -1;
  }
  va_list argp;
  void *p;
  va_start(argp, request);
  p = va_arg(argp, void *);
  va_end(argp);
  ReleaseLock();
  return handle->ioctl(request, p);
}

int MountManager::getdents(int fd, void *buf, unsigned int count) {
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

off_t MountManager::lseek(int fd, off_t offset, int whence) {
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

int MountManager::chmod(const char *path, mode_t mode) {
  Node *node;

  AcquireLock();
  node = GetNode(path);
  if (!node) {
    errno = ENOENT;
    ReleaseLock();
    return -1;
  }
  ReleaseLock();
  return node->chmod(mode);
}

int MountManager::remove(const char *path) {
  Node *node;

  AcquireLock();
  node = GetNode(path);
  if (!node) {
    errno = ENOENT;
    ReleaseLock();
    return -1;
  }
  ReleaseLock();
  return node->remove();
}

int MountManager::stat(const char *path, struct stat *buf) {
  Node *node;

  AcquireLock();
  node = GetNode(path);
  if (!node) {
    errno = ENOENT;
    ReleaseLock();
    return -1;
  }
  ReleaseLock();
  return node->stat(buf);
}

int MountManager::access(const char *path, int amode) {
  Node *node;

  AcquireLock();
  node = GetNode(path);
  if (!node) {
    errno = ENOENT;
    ReleaseLock();
    return -1;
  }
  ReleaseLock();
  return node->access(amode);
}

int MountManager::mkdir(const char *path, mode_t mode) {
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
    GetMount(ph.FormulatePath());
  if (!(m_and_p.first)) {
    errno = ENOTDIR;
    ReleaseLock();
    return -1;
  } else {
    ReleaseLock();
    return m_and_p.first->mkdir(path, mode);
  }
}

int MountManager::rmdir(const char *path) {
  Node *node;

  AcquireLock();
  node = GetNode(path);
  if (!node) {
    errno = ENOENT;
    ReleaseLock();
    return -1;
  }
  ReleaseLock();
  return node->rmdir();
}

void MountManager::AcquireLock(void) {
  if (pthread_mutex_lock(&lock_)) assert(0);
}

void MountManager::ReleaseLock(void) {
  if (pthread_mutex_unlock(&lock_)) assert(0);
}

Node *MountManager::GetNode(std::string path) {
  std::pair<Mount *, std::string> m_and_p;

  // check if path is of length zero
  if (path.length() == 0)
    return NULL;

  // check if the path is an absoulte path
  if (path[0] == '/') {
    m_and_p = GetMount(path);
  } else {
    // attach the path to the cwd
    PathHandle ph = cwd_;
    ph.AppendPath(path);
    m_and_p = GetMount(ph.FormulatePath());
  }

  if ((m_and_p.second).length() == 0) {
    return NULL;
  } else {
    if (!m_and_p.first)
      return NULL;
    return m_and_p.first->GetNode(m_and_p.second);
  }
}

FileHandle *MountManager::GetFileHandle(int fd) {
  if (fd < 0) return NULL;
  if (static_cast<size_t>(fd) + 1 > file_handles_.size()) return NULL;
  return file_handles_[fd];
}

std::pair<Mount *, std::string> MountManager::GetMount(std::string path) {

  std::pair<Mount *, std::string> ret;
  std::map<std::string, Mount *>::iterator it;
  std::string curr_best = "";
  ret.first = NULL;
  ret.second = path;

  if (path.length() == 0)
    return ret;

  // Find the longest path in the map that matches
  // the start of path
  for (it = mount_map_.begin();
       it != mount_map_.end() && path.length() != 0; ++it)
    if (path.find(it->first) == 0)
      if (it->first.length() > curr_best.length())
        curr_best = it->first;

  if (curr_best.length() == 0)
    return ret;

  ret.first = mount_map_[curr_best];
  // if the path matches exactly, returned path is empty string
  if (curr_best.compare(path) == 0)
    ret.second = "";
  else
    ret.second = path.substr(curr_best.length());
  return ret;
}

