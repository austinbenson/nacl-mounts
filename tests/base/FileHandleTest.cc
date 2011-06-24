/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */

#include "../../base/FileHandle.h"
#include "../common/common.h"

TEST(FileHandleTest, Sanity) {
  FileHandle *fh = new FileHandle();

  EXPECT_EQ(0, fh->lseek(0, 0));
  EXPECT_EQ(0, fh->close());
  EXPECT_EQ(0, fh->read(NULL, 0));
  EXPECT_EQ(0, fh->write(NULL, 0));
  EXPECT_EQ(0, fh->fstat(NULL));
  EXPECT_EQ(0, fh->isatty());
  EXPECT_EQ(0, fh->ioctl(0));
  EXPECT_EQ(0, fh->ioctl(0, NULL));
  EXPECT_EQ(0, fh->getdents(NULL, 0));

  fh->set_in_use(true);
  EXPECT_EQ(true, fh->in_use());
  fh->set_in_use(false);
  EXPECT_EQ(false, fh->in_use());

  delete fh;
}

