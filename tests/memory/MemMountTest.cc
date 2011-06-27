/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */

#include "../../base/MountManager.h"
#include "../../memory/MemMount.h"
#include "../common/common.h"
#include "TestHelpCommon.h"



TEST(MemMountTest, Locks) {
  MemMount *mount = new MemMount();
  mount->AcquireLock();
  mount->ReleaseLock();
  delete mount;
}

TEST(MemMountTest, GetMemNode) {
  MemMount *mount = new MemMount();
  MemNode *node1, *node2, *node3, *node4;
  mode_t mode = 0755;

  Node2* node21 = mount->Creat("/node1", mode);
  Node2* node22 = mount->Creat("/node2", mode);

  CHECK(node21);
  CHECK(node22);

  node1 = mount->ToMemNode(node21);
  node2 = mount->ToMemNode(node22);

  CHECK(node1);
  CHECK(node2);

  node1->set_is_dir(true);
  node2->set_is_dir(true);

  Node2* node23 = mount->Creat("/node1/node3", mode);
  Node2* node24 = mount->Creat("/node2/node4", mode);

  CHECK(node23);
  CHECK(node24);

  node3 = mount->ToMemNode(node23);
  node4 = mount->ToMemNode(node24);

  CHECK(node3);
  CHECK(node4);

  EXPECT_EQ(node1, mount->ToMemNode(mount->GetNode("/node1")));
  EXPECT_NE(node2, mount->ToMemNode(mount->GetNode("/node1")));
  EXPECT_NE(node3, mount->ToMemNode(mount->GetNode("/node1")));
  EXPECT_NE(node4, mount->ToMemNode(mount->GetNode("/node1")));

  EXPECT_EQ(node2, mount->ToMemNode(mount->GetNode("/node2")));
  EXPECT_NE(node1, mount->ToMemNode(mount->GetNode("/node2")));
  EXPECT_NE(node3, mount->ToMemNode(mount->GetNode("/node2")));
  EXPECT_NE(node4, mount->ToMemNode(mount->GetNode("/node2")));

  EXPECT_EQ(node3, mount->ToMemNode(mount->GetNode("/node1/node3")));
  EXPECT_NE(node1, mount->ToMemNode(mount->GetNode("/node1/node3")));
  EXPECT_NE(node2, mount->ToMemNode(mount->GetNode("/node1/node3")));
  EXPECT_NE(node4, mount->ToMemNode(mount->GetNode("/node1/node3")));

  EXPECT_EQ(node4, mount->ToMemNode(mount->GetNode("/node2/node4")));
  EXPECT_NE(node1, mount->ToMemNode(mount->GetNode("/node2/node4")));
  EXPECT_NE(node2, mount->ToMemNode(mount->GetNode("/node2/node4")));
  EXPECT_NE(node3, mount->ToMemNode(mount->GetNode("/node2/node4")));

  EXPECT_EQ(NULL, mount->GetNode("/hi"));
  EXPECT_EQ(NULL, mount->GetNode("/node2/node4/node5"));
  EXPECT_EQ(NULL, mount->GetNode(""));
  EXPECT_EQ(mount->root(), mount->ToMemNode(mount->GetNode("/")));

  delete mount;
}

TEST(MemMountTest, GetParentNode) {
  MemMount *mount = new MemMount();
  MemNode *node1, *node2, *node3, *node4;
  mode_t mode = 0755;

  Node2* node21 = mount->Creat("/node1", mode);
  Node2* node22 = mount->Creat("/node2", mode);

  CHECK(node21);
  CHECK(node22);

  node1 = mount->ToMemNode(node21);
  node2 = mount->ToMemNode(node22);

  CHECK(node1);
  CHECK(node2);

  node1->set_is_dir(true);
  node2->set_is_dir(true);

  Node2* node23 = mount->Creat("/node1/node3", mode);
  Node2* node24 = mount->Creat("/node2/node4", mode);

  CHECK(node23);
  CHECK(node24);

  node3 = mount->ToMemNode(node23);
  node4 = mount->ToMemNode(node24);

  CHECK(node1);
  CHECK(node2);

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
  MemNode *node1 = CreateMemNode("node1", NULL, mount);
  MemNode *node2 = CreateMemNode("node2", NULL, mount);
  Node2 *node23;
  mode_t mode = 0755;

  EXPECT_EQ(0, mount->mkdir("/hello/", 0));
  EXPECT_EQ(-1, mount->mkdir("/hello/", 0));
  EXPECT_EQ(0, mount->mkdir("/hello/world", 0));

  Node2 *node21 = mount->GetNode("/hello/world");
  CHECK(node21);
  node1 = mount->ToMemNode(node21);
  CHECK(node1);
  node1->set_is_dir(false);

  EXPECT_EQ(-1, mount->mkdir("/hello/world/again/", 0));

  node1->set_is_dir(true);

  EXPECT_EQ(0, mount->mkdir("/hello/world/again/", 0));

  node23 = mount->Creat("/hello/world/again/../../world/again/again", mode);
  CHECK(node23);

  MemNode* node3 = mount->ToMemNode(node23);
  CHECK(node3);

  delete node1;
  delete node2;
  delete node3;
  delete mount;
}

TEST(MemMountTest, Creat) {
  MemMount *mount = new MemMount();
  mode_t mode = 0755;

  EXPECT_EQ(0, mount->mkdir("/node1", 0755));
  CHECK(mount->Creat("/node2", mode));
  CHECK(mount->Creat("/node1/node3", mode));
  CHECK(mount->Creat("/node1/node4/", mode));
  EXPECT_EQ((Node2*)NULL, mount->GetNode("/node5/"));
  CHECK(mount->Creat("/node1/node4/../../node1/./node5", mode));
  CHECK(mount->GetNode("/node1/node3/../../node1/./"));

  delete mount;
}

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

TEST(MemMountTest, Stat) {
  KernelProxy *kp = MountManager::MMInstance()->kp();
  ASSERT_LE(0, kp->mkdir("/MemMountTest_Stat", 0755));
  struct stat st;
  ASSERT_EQ(0, kp->stat("/MemMountTest_Stat", &st));
  ASSERT_TRUE(S_ISDIR(st.st_mode));
  ASSERT_FALSE(S_ISREG(st.st_mode));
  ASSERT_EQ(-1, kp->stat("/MemMountTest_Stat2", &st));
  int fd = kp->open("/MemMountTest_Stat/file", O_CREAT, 644);
  ASSERT_LE(0, fd);
  ASSERT_EQ(0, kp->stat("/MemMountTest_Stat/file", &st));
  ASSERT_FALSE(S_ISDIR(st.st_mode));
  ASSERT_TRUE(S_ISREG(st.st_mode));
}
