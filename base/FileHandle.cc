/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#include "FileHandle.h"
#include "Mount.h"

FileHandle::FileHandle() {
  mount = NULL;
  node = NULL;
  used_ = 0;
  offset = 0;
  flags = 0;
}

FileHandle::~FileHandle() {
}
