/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */

#include "../../base/FileHandle.h"
#include "../../base/MountManager.h"
#include "../../memory/MemMount.h"
#include "../common/common.h"

MemNode *CreateNode(std::string name, MemNode *parent,
  MemMount *mount, bool is_dir = false) {
  MemNode *node = new MemNode();
  node->set_name(name);
  node->set_parent(parent);
  node->set_mount(mount);
  node->set_is_dir(is_dir);
  return node;
}

TEST(MemMountTest, Locks) {
  MemMount *mount = new MemMount();
  mount->AcquireLock();
  mount->ReleaseLock();
  delete mount;
}

TEST(MemMountTest, GetNode) {
  MemMount *mount = new MemMount();
  MemFileHandle *fh1, *fh2, *fh3, *fh4;
  MemFileHandle *fh5 = NULL;
  MemNode *node1, *node2, *node3, *node4;

  fh1 = reinterpret_cast<MemFileHandle *>(mount->MountOpen("/node1", O_CREAT));
  fh2 = reinterpret_cast<MemFileHandle *>(mount->MountOpen("/node2", O_CREAT));

  EXPECT_NE(fh5, fh1);
  EXPECT_NE(fh5, fh2);

  node1 = fh1->node();
  node1->set_is_dir(true);
  node2 = fh2->node();
  node2->set_is_dir(true);

  fh3 = reinterpret_cast<MemFileHandle *>(
        mount->MountOpen("/node1/node3", O_CREAT));
  fh4 = reinterpret_cast<MemFileHandle *>(
        mount->MountOpen("/node2/node4", O_CREAT));

  EXPECT_NE(fh5, fh3);
  EXPECT_NE(fh5, fh4);

  node3 = fh3->node();
  node4 = fh4->node();

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
  MemFileHandle *fh1, *fh2, *fh3, *fh4;
  MemFileHandle *fh5 = NULL;
  MemNode *node1, *node2, *node3, *node4;

  fh1 = reinterpret_cast<MemFileHandle *>(mount->MountOpen("/node1", O_CREAT));
  fh2 = reinterpret_cast<MemFileHandle *>(mount->MountOpen("/node2", O_CREAT));

  EXPECT_NE(fh5, fh1);
  EXPECT_NE(fh5, fh2);

  node1 = fh1->node();
  node1->set_is_dir(true);
  node2 = fh2->node();
  node2->set_is_dir(true);

  fh3 = reinterpret_cast<MemFileHandle *>(
        mount->MountOpen("/node1/node3", O_CREAT));
  fh4 = reinterpret_cast<MemFileHandle *>(
        mount->MountOpen("/node2/node4", O_CREAT));

  EXPECT_NE(fh5, fh3);
  EXPECT_NE(fh5, fh4);

  node3 = fh3->node();
  node4 = fh4->node();

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
  MemNode *node = CreateNode("node", NULL, mount);
  MemNode *node2 = CreateNode("node2", NULL, mount);
  FileHandle *fh;
  FileHandle *fh_null = NULL;

  EXPECT_EQ(0, mount->mkdir("/hello/", 0));
  EXPECT_EQ(-1, mount->mkdir("/hello/", 0));
  EXPECT_EQ(0, mount->mkdir("/hello/world", 0));

  node = mount->GetMemNode("/hello/world");
  node->set_is_dir(false);

  EXPECT_EQ(-1, mount->mkdir("/hello/world/again/", 0));

  node->set_is_dir(true);

  EXPECT_EQ(0, mount->mkdir("/hello/world/again/", 0));

  fh = reinterpret_cast<MemFileHandle *>(
    mount->MountOpen("/hello/world/again/../../world/again/again", O_CREAT));

  EXPECT_NE(fh_null, fh);

  delete node;
  delete node2;
  delete mount;
}

TEST(MemMountTest, MountOpen) {
  MemMount *mount = new MemMount();
  FileHandle *fh = NULL;

  EXPECT_EQ(0, mount->mkdir("/node1", O_CREAT));
  EXPECT_NE(fh, mount->MountOpen("/node2", O_CREAT));
  EXPECT_NE(fh, mount->MountOpen("/node1/node3", O_CREAT));
  EXPECT_NE(fh, mount->MountOpen("/node1/node4/", O_CREAT));
  EXPECT_EQ(fh, mount->MountOpen("/node1/", O_CREAT));
  EXPECT_NE(fh, mount->MountOpen("/node1/node4/../../node1/./node5", O_CREAT));
  EXPECT_NE(fh, mount->MountOpen("/node1/node3/../../node1/./", 0));
  EXPECT_EQ(fh, mount->MountOpen("/node1/node3/../../node1/./",
            O_CREAT | O_EXCL));

  delete mount;
}

TEST(MemMountTest, DefaultMount) {
  MountManager* mm = MountManager::MMInstance();
  int fd = mm->open("/lala.txt", O_WRONLY | O_CREAT, 0644);
  if (fd == -1) {
    perror("mm->open: ");
  }
  ASSERT_LE(0, fd);
  ASSERT_EQ(0, mm->close(fd));
}
