/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#include "FileHandle.h"
#include "Mount.h"

FileHandle::FileHandle() {
  mount_ = NULL;
  node_ = NULL;
  used_ = 0;
  offset_ = 0;
  flags_ = 0;
}

FileHandle::~FileHandle() {
}

off_t FileHandle::lseek(off_t offset, int whence) {
  off_t next;
  mount_->AcquireLock();

  // Check that it isn't a directory.
  if (mount_->is_dir(node_)) {
    errno = EBADF;
    mount_->ReleaseLock();
    return -1;
  }
  switch (whence) {
  case SEEK_SET:
    next = offset;
    break;
  case SEEK_CUR:
    next = offset_ + offset;
    // TODO(arbenson): handle EOVERFLOW if too big.
    break;
  case SEEK_END:
    next = mount_->len(node_) - offset;
    // TODO(arbenson): handle EOVERFLOW if too big.
    break;
  default:
    errno = EINVAL;
    mount_->ReleaseLock();
    return -1;
  }
  // Must not seek beyond the front of the file.
  if (next < 0) {
    errno = EINVAL;
    mount_->ReleaseLock();
    return -1;
  }
  // Go to the new offset.
  offset_ = next;
  mount_->ReleaseLock();
  return next;
}

ssize_t FileHandle::read(void *buf, size_t nbyte) {
  size_t len;
  mount_->AcquireLock();

  // Check that this file handle can be read from.
  if ((flags_ & O_ACCMODE) == O_WRONLY ||
      mount_->is_dir(node_)) {
    errno = EBADF;
    mount_->ReleaseLock();
    return -1;
  }
  // Limit to the end of the file.
  len = nbyte;
  if (len > mount_->len(node_) - offset_)
    len = mount_->len(node_) - offset_;

  // Do the read.
  memcpy(buf, mount_->data(node_) + offset_, len);
  offset_ += len;
  mount_->ReleaseLock();
  return len;
}

ssize_t FileHandle::write(const void *buf, size_t nbyte) {
  size_t len = 0;
  size_t next = 0;

  mount_->AcquireLock();

  // Check that this file handle can be written to.
  if ((flags_ & O_ACCMODE) == O_RDONLY ||
      mount_->is_dir(node_)) {
    errno = EBADF;
    mount_->ReleaseLock();
    return -1;
  }
  // Grow the file if needed.
  if (offset_ + static_cast<off_t>(nbyte) > mount_->capacity(node_)) {
    len = offset_ + nbyte;
    next = (mount_->capacity(node_) + 1) * 2;
    if (next > len) len = next;
    mount_->ReallocData(node_, len);
  }
  // Pad any gap with zeros.
  if (offset_ > static_cast<off_t>(mount_->len(node_)))
    memset(mount_->data(node_)+len, 0, offset_);

  // Write out the block.
  memcpy(mount_->data(node_) + offset_, buf, nbyte);
  offset_ += nbyte;
  if (offset_ > static_cast<off_t>(mount_->len(node_)))
    mount_->set_len(node_, offset_);
  mount_->ReleaseLock();
  return nbyte;
}

int FileHandle::fstat(struct stat *buf) {
  mount_->AcquireLock();
  mount_->raw_stat(node_, buf);
  mount_->ReleaseLock();
  return 0;
}

int FileHandle::close(void) {
  mount_->AcquireLock();
  mount_->DecrementUseCount(node_);
  in_use_ = false;
  mount_->ReleaseLock();
  return 0;
}

int FileHandle::ioctl(unsigned long request, ...) {
  return -1;
}
