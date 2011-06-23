/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#include "Node.h"

int Node::access(int amode) {
  struct stat *buf = (struct stat *)malloc(sizeof(struct stat));
  assert(buf);
  int ret = stat(buf);
  // check for stat() failure and assume
  // that stat sets the errno
  if (ret == -1)
    return -1;

  // Check if we want existence
  if (amode & F_OK) {}     // we know this node exists

  // check if we want read permissions
  if (amode & R_OK) {
    // check for read permission
    if (!(S_IRUSR & buf->st_mode)) {
      errno = EACCES;
      return -1;
    }
  }

  // check if we want write permissions
  if (amode & W_OK) {
    // check for write permission
    if (!(S_IWUSR & buf->st_mode)) {
      errno = EACCES;
      return -1;
    }
  }

  // check if we want execute permissions
  if (amode & X_OK) {
    // check for execute permission
    if (!(S_IXUSR & buf->st_mode)) {
      errno = EACCES;
      return -1;
    }
  }
  // everything should be ok
  return 0;
}

int Node::stat(struct stat *buf) {
  fprintf(stderr, "stat not implemented!\n");
  assert(false);
  return 0;
}

