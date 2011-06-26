/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */


#ifndef PACKAGES_SCRIPTS_FILESYS_TESTS_BASE_TESTBASECOMMON_H_
#define PACKAGES_SCRIPTS_FILESYS_TESTS_BASE_TESTBASECOMMON_H_

FileHandle *CreateFileHandle(MemMount *mount, Node *node,
                                   int used, int offset, int flags) {
  FileHandle *mfh = new FileHandle();
  mfh->set_mount(mount);
  mfh->set_node(node);
  mfh->set_used(used);
  mfh->set_offset(offset);
  mfh->set_flags(flags);
  return mfh;
}

#endif  // PACKAGES_SCRIPTS_FILESYS_TESTS_BASE_TESTBASECOMMON_H_
