/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */

#include "../../base/MountManager.h"
#include "../../memory/MemMount.h"
#include "../common/common.h"
#include "TestHelpCommon.h"

/*

TEST(MemMountTest, Locks) {
  MemMount *mount = new MemMount();
  mount->AcquireLock();
  mount->ReleaseLock();
  delete mount;
}

TEST(MemMountTest, GetNode) {
  MemMount *mount = new MemMount();
  Node *node1, *node2, *node3, *node4;
  mode_t mode = 0755;

  node1 = mount->MountOpen("/node1", O_CREAT, mode);
  node2 = mount->MountOpen("/node2", O_CREAT, mode);

  CHECK(node1);
  CHECK(node2);

  node1->set_is_dir(true);
  node2->set_is_dir(true);

  node3 = mount->MountOpen("/node1/node3", O_CREAT, mode);
  node4 = mount->MountOpen("/node2/node4", O_CREAT, mode);

  CHECK(node3);
  CHECK(node4);

  EXPECT_EQ(node1, mount->GetNode("/node1"));
  EXPECT_NE(node2, mount->GetNode("/node1"));
  EXPECT_NE(node3, mount->GetNode("/node1"));
  EXPECT_NE(node4, mount->GetNode("/node1"));

  EXPECT_EQ(node2, mount->GetNode("/node2"));
  EXPECT_NE(node1, mount->GetNode("/node2"));
  EXPECT_NE(node3, mount->GetNode("/node2"));
  EXPECT_NE(node4, mount->GetNode("/node2"));

  EXPECT_EQ(node3, mount->GetNode("/node1/node3"));
  EXPECT_NE(node1, mount->GetNode("/node1/node3"));
  EXPECT_NE(node2, mount->GetNode("/node1/node3"));
  EXPECT_NE(node4, mount->GetNode("/node1/node3"));

  EXPECT_EQ(node4, mount->GetNode("/node2/node4"));
  EXPECT_NE(node1, mount->GetNode("/node2/node4"));
  EXPECT_NE(node2, mount->GetNode("/node2/node4"));
  EXPECT_NE(node3, mount->GetNode("/node2/node4"));

  EXPECT_EQ(NULL, mount->GetNode("/hi"));
  EXPECT_EQ(NULL, mount->GetNode("/node2/node4/node5"));
  EXPECT_EQ(NULL, mount->GetNode(""));
  EXPECT_EQ(mount->root(), mount->GetNode("/"));

  delete mount;
}

TEST(MemMountTest, GetParentNode) {
  MemMount *mount = new MemMount();
  Node *node1, *node2, *node3, *node4;
  mode_t mode = 0755;

  node1 = mount->MountOpen("/node1", O_CREAT, mode);
  node2 = mount->MountOpen("/node2", O_CREAT, mode);

  CHECK(node1);
  CHECK(node2);

  node1->set_is_dir(true);
  node2->set_is_dir(true);

  node3 = mount->MountOpen("/node1/node3", O_CREAT, mode);
  node4 = mount->MountOpen("/node2/node4", O_CREAT, mode);

  CHECK(node3);
  CHECK(node4);

  EXPECT_EQ(mount->root(), mount->GetParentNode("/hi"));
  EXPECT_EQ(node1, mount->GetParentNode("/node1/node3"));
  EXPECT_EQ(node2, mount->GetParentNode("/node2/hi/"));
  EXPECT_EQ(node3, mount->GetParentNode("/node1/node3/hi"));
  EXPECT_EQ(node4, mount->GetParentNode("/node2/node4/hi"));
  EXPECT_EQ(NULL, mount->GetParentNode("/node2/node4/node5/hi"));

  delete mount;
}

TEST(MemMountTest, mkdir) {
  MemMount *mount = new MemMount();
  Node *node = CreateNode("node", NULL, mount);
  Node *node2 = CreateNode("node2", NULL, mount);
  Node *node3;
  mode_t mode = 0755;

  EXPECT_EQ(0, mount->mkdir("/hello/", 0));
  EXPECT_EQ(-1, mount->mkdir("/hello/", 0));
  EXPECT_EQ(0, mount->mkdir("/hello/world", 0));

  node = mount->GetNode("/hello/world");
  node->set_is_dir(false);

  EXPECT_EQ(-1, mount->mkdir("/hello/world/again/", 0));

  node->set_is_dir(true);

  EXPECT_EQ(0, mount->mkdir("/hello/world/again/", 0));

  node3 = mount->MountOpen("/hello/world/again/../../world/again/again", O_CREAT, mode);

  CHECK(node3);

  delete node;
  delete node2;
  delete mount;
}

TEST(MemMountTest, MountOpen) {
  MemMount *mount = new MemMount();
  Node *node = NULL;
  mode_t mode = 0755;

  EXPECT_EQ(0, mount->mkdir("/node1", O_CREAT));
  EXPECT_NE(node, mount->MountOpen("/node2", O_CREAT, mode));
  EXPECT_NE(node, mount->MountOpen("/node1/node3", O_CREAT, mode));
  EXPECT_NE(node, mount->MountOpen("/node1/node4/", O_CREAT, mode));
  EXPECT_EQ(node, mount->MountOpen("/node1/", O_CREAT, mode));
  EXPECT_NE(node, mount->MountOpen("/node1/node4/../../node1/./node5", O_CREAT, mode));
  EXPECT_NE(node, mount->MountOpen("/node1/node3/../../node1/./", 0, mode));
  EXPECT_EQ(node, mount->MountOpen("/node1/node3/../../node1/./",
                                   O_CREAT | O_EXCL, mode));

  delete mount;
}
*/

/*
static const char* kTestFileName = "/lala.txt";

static void test_write() {
  KernelProxy* kp = MountManager::MMInstance()->kp();
  int fd = kp->open(kTestFileName, O_WRONLY | O_CREAT, 0644);
  if (fd == -1) {
    perror("mm->open: ");
  }
  ASSERT_LE(0, fd);
  ASSERT_EQ(5, kp->write(fd, "hello", 5));
  ASSERT_EQ(0, kp->close(fd));
}

static void test_read(int* out) {
  KernelProxy *kp = MountManager::MMInstance()->kp();

  int fd = kp->open(kTestFileName, O_RDONLY);
  if (fd == -1) {
    perror("mm->open: ");
  }
  ASSERT_LE(0, fd);
  char buf[6];
  buf[5] = 0;
  ASSERT_EQ(5, kp->read(fd, buf, 5));
  ASSERT_STREQ("hello", buf);
  *out = fd;
}

static void test_close(int fd) {
  ASSERT_EQ(0, MountManager::MMInstance()->kp()->close(fd));
}

TEST(MemMountTest, DefaultMount) {
  int fds[1];
  for (int i = 0; i < 1; i++) {
    test_write();
    test_read(&fds[i]);
  }
  for (int i = 0; i < 1; i++) {
    test_close(fds[i]);
  }
}
*/
