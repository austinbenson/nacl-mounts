/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#include "KernelProxy.h"
#include "MountManager.h"

void KernelProxy::Init(MountManager *mm) {
  max_path_len_ = 256;
  cwd_.set_is_absolute(true);
  cwd_.SetPath("/");
  mm_ = mm;
}

KernelProxy::~KernelProxy() {
  file_handles_.clear();
}

static bool is_dir(Mount* mount, Node2* node) {
  struct stat st;
  if (0 != mount->stat(node, &st)) {
    return false;
  }
  return S_ISDIR(st.st_mode);
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
  if (!is_dir(mnode.first, mnode.second)) {
    errno = ENOTDIR;
    return -1;
  }

  // update path
  m_and_p = mm_->GetMount(cwd_.FormulatePath());
  if (!(m_and_p.first)) {
    return -1;
  }
  cwd_ = ph;

  return 0;
}

int KernelProxy::RegisterFileHandle(FileHandle *fh) {
  size_t fd;

  // get first available fd
  for (fd = 0; fd < file_handles_.size(); ++fd) {
    if (!(file_handles_[fd])) {
      fh->in_use = true;
      file_handles_[fd] = fh;
      return fd;
    } else if (!(file_handles_[fd]->in_use)) {
      delete file_handles_[fd];
      fh->in_use = true;
      file_handles_[fd] = fh;
      return fd;
    }
  }

  file_handles_.push_back(fh);
  fh->in_use = true;

  return fd;
}

bool KernelProxy::getcwd(std::string *buf, size_t size) {
  if (size <= 0) {
    errno = EINVAL;
    return false;
  }
  if (size < cwd_.FormulatePath().length()) {
    errno = ERANGE;
    return false;
  }
  *buf = cwd_.FormulatePath();
  return true;
}

bool KernelProxy::getwd(std::string *buf) {
  return getcwd(buf, max_path_len_);
}

int KernelProxy::link(const std::string& path1, const std::string& path2) {
  errno = ENOSYS;
  fprintf(stderr, "link has not been implemented!\n");
  assert(0);
  return -1;
}

int KernelProxy::symlink(const std::string& path1, const std::string& path2) {
  errno = ENOSYS;
  fprintf(stderr, "symlink has not been implemented!\n");
  assert(0);
  return -1;
}

static ssize_t GetFileLen(Mount* mount, Node2 *node) {
  struct stat st;
  if (0 != mount->stat(node, &st)) {
    return -1;
  }
  return (ssize_t) st.st_size;
}

FileHandle* KernelProxy::OpenHandle(Mount* mount, const std::string& path,
                                    int flags, mode_t mode) {
  Node2* node = mount->GetNode(path);
  if (node != NULL && (flags & O_CREAT) && (flags & O_EXCL)) {
    errno = EEXIST;
    return NULL;
  }
  if (node == NULL && (flags & O_CREAT)) {
    node = mount->Creat(path, mode);
  }
  if (node == NULL) {
    return NULL;
  }
  // Setup file handle.
  FileHandle* handle = new FileHandle();
  handle->mount = mount;
  handle->node = node;
  handle->flags = flags;
  handle->in_use = true;

  if (flags & O_APPEND) {
    ssize_t off = GetFileLen(mount, node);
    if (off == -1) {
      return NULL;
    }
    handle->offset = (size_t)off;
  } else {
    handle->offset = 0;
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
  std::pair<Mount *, std::string> m_and_p =
    mm_->GetMount(ph.FormulatePath());

  if (!(m_and_p.first)) {
    errno = ENOENT;
    return -1;
  } else {
    handle = OpenHandle(m_and_p.first, m_and_p.second, oflag, 0);
  }
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
  std::pair<Mount *, std::string> m_and_p =
    mm_->GetMount(ph.FormulatePath());

  if (!(m_and_p.first)) {
    errno = ENOENT;
    return -1;
  } else {
    if (oflag & O_CREAT) {
      handle = OpenHandle(m_and_p.first, m_and_p.second, oflag, mode);
    } else {
      handle = OpenHandle(m_and_p.first, m_and_p.second, oflag, 0);
    }
  }
  return (!handle) ? -1 : RegisterFileHandle(handle);
}

int KernelProxy::close(int fd) {
  FileHandle *handle;

  // check if fd is valid and handle exists
  if (!(handle = GetFileHandle(fd))) {
    errno = EBADF;
    return -1;
  }
  handle->mount->DecrementUseCount(handle->node);
  handle->in_use = false;
  return 0;
}

ssize_t KernelProxy::read(int fd, void *buf, size_t count) {
  FileHandle *handle;

  // check if fd is valid and handle exists
  if (!(handle = GetFileHandle(fd))) {
    errno = EBADF;
    return -1;
  }
  // Check that this file handle can be read from.
  if ((handle->flags & O_ACCMODE) == O_WRONLY ||
      is_dir(handle->mount, handle->node)) {
    errno = EBADF;
    return -1;
  }

  ssize_t n = handle->mount->Read(handle->node, handle->offset, buf, count);
  if (n > 0) {
    handle->offset += n;
  }
  return n;
}

ssize_t KernelProxy::write(int fd, const void *buf, size_t count) {
  FileHandle *handle;

  // TODO(krasin): fix locking here.

  // check if fd is valid and handle exists
  if (!(handle = GetFileHandle(fd))) {
    errno = EBADF;
    return -1;
  }
  // Check that this file handle can be written to.
  if ((handle->flags & O_ACCMODE) == O_RDONLY ||
      is_dir(handle->mount, handle->node)) {
    errno = EBADF;
    return -1;
  }

  ssize_t n = handle->mount->Write(handle->node, handle->offset, buf, count);
  if (n > 0) {
    handle->offset += n;
  }
  return n;
}

int KernelProxy::fstat(int fd, struct stat *buf) {
  FileHandle *handle;

  // check if fd is valid and handle exists
  if (!(handle = GetFileHandle(fd))) {
    errno = EBADF;
    return -1;
  }

  return handle->mount->stat(handle->node, buf);
}

int KernelProxy::ioctl(int fd, unsigned long request) {
  errno = ENOSYS;
  return -1;
}

int KernelProxy::getdents(int fd, void *buf, unsigned int count) {
  FileHandle *handle;

  // check if fd is valid and handle exists
  if (!(handle = GetFileHandle(fd))) {
    errno = EBADF;
    return -1;
  }

  // TODO(Krasin): support offset for getdents
  return handle->mount->Getdents(handle->node, 0, (struct dirent*)buf, count);
}

int KernelProxy::fsync(int fd) {
  FileHandle *handle;

  if (!(handle = GetFileHandle(fd))) {
    errno = EBADF;
    return -1;
  }

  return handle->mount->Fsync(handle->node);
}

off_t KernelProxy::lseek(int fd, off_t offset, int whence) {
  FileHandle *handle;

  // check if fd is valid and handle exists
  if (!(handle = GetFileHandle(fd))) {
    errno = EBADF;
    return -1;
  }
  off_t next;
  ssize_t len;

  // Check that it isn't a directory.
  if (is_dir(handle->mount, handle->node)) {
    errno = EBADF;
    return -1;
  }
  switch (whence) {
  case SEEK_SET:
    next = offset;
    break;
  case SEEK_CUR:
    next = handle->offset + offset;
    // TODO(arbenson): handle EOVERFLOW if too big.
    break;
  case SEEK_END:
    // TODO(krasin): FileHandle should store file len.
    len = GetFileLen(handle->mount, handle->node);
    if (len == -1) {
      return -1;
    }
    next = (size_t)len - offset;
    // TODO(arbenson): handle EOVERFLOW if too big.
    break;
  default:
    errno = EINVAL;
    return -1;
  }
  // Must not seek beyond the front of the file.
  if (next < 0) {
    errno = EINVAL;
    return -1;
  }
  // Go to the new offset.
  handle->offset = next;
  return handle->offset;
}

int KernelProxy::chmod(const std::string& path, mode_t mode) {
  std::pair<Mount*, Node2*> mnode = mm_->GetNode(path);
  if (!mnode.first) {
    errno = ENOENT;
    return -1;
  }
  return mnode.first->chmod(mnode.second, mode);
}

int KernelProxy::remove(const std::string& path) {
  std::pair<Mount*, Node2*> mnode = mm_->GetNode(path);
  if (!mnode.first) {
    errno = ENOENT;
    return -1;
  }
  return mnode.first->remove(mnode.second);
}

int KernelProxy::stat(const std::string& path, struct stat *buf) {
  std::pair<Mount*, Node2*> mnode = mm_->GetNode(path);
  if (!mnode.first) {
    errno = ENOENT;
    return -1;
  }
  return mnode.first->stat(mnode.second, buf);
}

int KernelProxy::access(const std::string& path, int amode) {
  struct stat *buf = (struct stat *)malloc(sizeof(struct stat));
  
  if (path.empty()) {
    errno = ENOENT;
    return -1;
  }
  PathHandle ph = cwd_;
  if (path[0] == '/') {
    ph.SetPath(path);
  } else {
    ph.AppendPath(path);
  }

  std::list<std::string> path_components = ph.path();
  std::list<std::string>::iterator it;
  std::list<std::string> incremental_paths;

  // All components of the path are checked for
  // access permissions.
  std::string curr_path = "/";
  incremental_paths.push_back(curr_path);

  // Formulate the path of each component.
  for (it = path_components.begin(); 
       it != path_components.end(); ++it) {
    curr_path += "/";
    curr_path += *it;
    incremental_paths.push_back(curr_path);
  }

  // Loop over each path component and
  // check access permissions.
  for (it = incremental_paths.begin(); 
       it != incremental_paths.end(); ++it) {
    curr_path = *it;
    // first call stat on the file
    if (stat(curr_path, buf) == -1) {
      return -1;  // stat should take care of errno
    }
    mode_t mode = buf->st_mode;

    // We know that the file exists at this point.
    // Thus, we don't have to check F_OK.
    if (amode & R_OK && !(mode & R_OK) ||
        amode & W_OK && !(mode & W_OK) ||
	amode & X_OK && !(mode & X_OK)) {
	errno = EACCES;
	return -1;
    }
  }
  // By now we have checked access permissions for
  // each component of the path.
  return 0;
}

int KernelProxy::mkdir(const std::string& path, mode_t mode) {
  std::string p(path);
  if (p.length() == 0)
    return -1;

  PathHandle ph = cwd_;
  if (p[0] == '/') {
    ph.SetPath(p);
  } else {
    ph.AppendPath(p);
  }

  std::pair<Mount *, std::string> m_and_p =
    mm_->GetMount(ph.FormulatePath());
  if (!(m_and_p.first)) {
    errno = ENOTDIR;
    return -1;
  } else {
    return m_and_p.first->mkdir(m_and_p.second, mode);
  }
}

int KernelProxy::rmdir(const std::string& path) {
  std::pair<Mount*, Node2*> mnode = mm_->GetNode(path);
  if (!mnode.first) {
    errno = ENOENT;
    return -1;
  }
  return mnode.first->rmdir(mnode.second);
}

FileHandle *KernelProxy::GetFileHandle(int fd) {
  if (fd < 0) return NULL;
  if (static_cast<size_t>(fd) + 1 > file_handles_.size()) return NULL;
  return file_handles_[fd];
}
