/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#include "../../base/Node.h"
#include "../common/common.h"

TEST(NodeTest, Sanity) {
  Node *n = new Node();

  EXPECT_EQ(0, n->open(0));
  EXPECT_EQ(0, n->unlink());
  EXPECT_EQ(0, n->chmod(0));
  EXPECT_EQ(0, n->remove());
  EXPECT_EQ(0, n->utime(NULL));
  EXPECT_EQ(0, n->rmdir());

  delete n;
}

