/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */

#include <string>
#include "../../memory/MemMount.h"
#include "../../memory/MemNode.h"
#include "../common/common.h"
#include "TestHelpCommon.h"


TEST(MemNodeTest, AddChildren) {
  MemMount *mnt = new MemMount();
  Node *node1 = CreateNode("node1", NULL, mnt, true);
  Node *node2 = CreateNode("node2", node1, mnt, true);
  Node *node3 = CreateNode("node3", node1, mnt, false);
  Node *node4 = CreateNode("node4", node1, mnt, false);
  Node *node5 = CreateNode("node5", node2, mnt, false);
  node1->AddChild(node2);
  node1->AddChild(node3);
  node1->AddChild(node4);
  node2->AddChild(node5);

  std::list<Node *> *children;
  std::list<Node *> *children2;
  std::list<Node *> *children3;
  children = node1->children();
  EXPECT_EQ(3, static_cast<int>(children->size()));

  std::list<Node *>::iterator it;
  it = children->begin();
  EXPECT_EQ(node2->name(), (*it)->name());
  children2 = (*it)->children();
  EXPECT_EQ(1, static_cast<int>(children2->size()));

  ++it;
  EXPECT_EQ(node3->name(), (*it)->name());
  children3 = (*it)->children();
  EXPECT_EQ(NULL, children3);

  it = children2->begin();
  EXPECT_EQ(node5->name(), (*it)->name());
  children2 = (*it)->children();
  EXPECT_EQ(NULL, children2);

  // can't add children to non-directory
  children = node4->children();
  EXPECT_EQ(NULL, children);
  node4->AddChild(new MemNode());
  node4->AddChild(new MemNode());
  node4->AddChild(new MemNode());
  children = node4->children();
  EXPECT_EQ(NULL, children);

  delete mnt;
  delete node1;
  delete node2;
  delete node3;
  delete node4;
  delete node5;
}

TEST(MemNodeTest, RemoveChildren) {
  MemMount *mnt = new MemMount();
  Node *node1 = CreateNode("node1", NULL, mnt, true);
  Node *node2 = CreateNode("node2", node1, mnt, true);
  Node *node3 = CreateNode("node3", node1, mnt, true);
  Node *node4 = CreateNode("node4", node1, mnt, true);
  Node *node5 = CreateNode("node5", node2, mnt, false);
  Node *node6 = CreateNode("node6", node4, mnt);

  node1->AddChild(node2);
  node1->AddChild(node3);
  node1->AddChild(node4);
  node2->AddChild(node5);

  node1->RemoveChild(node2);
  node1->RemoveChild(node3);

  std::list<Node *> *children;
  std::list<Node *>::iterator it;

  children = node1->children();
  EXPECT_EQ(1, static_cast<int>(children->size()));
  it = children->begin();
  EXPECT_EQ(node4, *it);

  node2->RemoveChild(node4);
  children = node2->children();
  EXPECT_EQ(1, static_cast<int>(children->size()));

  node2->RemoveChild(node5);
  children = node2->children();
  EXPECT_EQ(0, static_cast<int>(children->size()));

  // can't remove child from non-directory
  node4->AddChild(node6);
  node4->set_is_dir(false);
  node4->RemoveChild(node6);
  children = node4->children();
  EXPECT_EQ(NULL, children);
  node4->set_is_dir(true);
  node4->RemoveChild(node6);
  children = node4->children();
  EXPECT_EQ(0, static_cast<int>(children->size()));

  node4->set_is_dir(true);

  delete mnt;
  delete node1;
  delete node2;
  delete node3;
  delete node4;
  delete node5;
  delete node6;
}

TEST(MemNodeTest, Stat) {
  MemMount *mnt = new MemMount();
  struct stat *buf = (struct stat *)malloc(sizeof(struct stat));
  Node *node = CreateNode("node", NULL, mnt);
  node->set_is_dir(false);
  int size = 128;
  node->ReallocData(size);
  EXPECT_EQ(size, node->capacity());
  node->stat(buf);
  EXPECT_EQ(0, buf->st_size);
  delete mnt;
  delete node;
}

TEST(MemNodeTest, remove) {
  MemMount *mnt = new MemMount();
  Node *node1 = CreateNode("node1", NULL, mnt, true);
  Node *node2 = CreateNode("node2", node1, mnt);
  Node *node3 = CreateNode("node3", node1, mnt, true);
  Node *node4 = CreateNode("node3", node3, mnt);
  node1->AddChild(node2);

  std::list<Node *> *children;
  children = node1->children();
  EXPECT_EQ(1, static_cast<int>(children->size()));

  node1->set_is_dir(true);
  node2->set_is_dir(true);
  EXPECT_EQ(-1, node2->remove());
  node2->set_is_dir(false);
  EXPECT_EQ(0, node2->remove());

  children = node1->children();
  EXPECT_EQ(0, static_cast<int>(children->size()));

  node3->AddChild(node4);
  node4->IncrementUseCount();
  EXPECT_EQ(-1, node3->remove());
  node4->DecrementUseCount();
  EXPECT_EQ(0, node4->remove());

  delete mnt;
  delete node1;
}

TEST(MemNodeTest, rmdir) {
  MemMount *mnt = new MemMount();
  Node *node1 = CreateNode("node1", NULL, mnt, true);
  Node *node2 = CreateNode("node2", node1, mnt, true);
  Node *node3 = CreateNode("node3", node1, mnt);
  Node *node4 = CreateNode("node4", node2, mnt, true);

  node1->AddChild(node2);
  node1->AddChild(node3);
  EXPECT_EQ(-1, node1->rmdir());

  std::list<Node *> *children;
  node2->AddChild(node4);
  children = node2->children();
  EXPECT_EQ(1, static_cast<int>(children->size()));

  EXPECT_EQ(0, node4->rmdir());

  children = node2->children();
  EXPECT_EQ(0, static_cast<int>(children->size()));

  node1->RemoveChild(node2);
  node1->RemoveChild(node3);
  node1->set_is_dir(false);
  EXPECT_EQ(-1, node1->rmdir());
  node1->set_is_dir(true);
  EXPECT_EQ(0, node1->rmdir());

  delete mnt;
  delete node2;
  delete node3;
}

TEST(MemNodeTest, UseCount) {
  MemMount *mnt = new MemMount();
  Node *node1 = CreateNode("node1", NULL, mnt);
  int i;

  for (i = 0; i < 10; i++) {
    node1->IncrementUseCount();
  }

  for (i = 0; i < 8; i++) {
    node1->DecrementUseCount();
  }

  EXPECT_EQ(2, node1->use_count());

  delete mnt;
  delete node1;
}

TEST(MemNodeTest, Stubs) {
  MemMount *mnt = new MemMount();
  Node *node1 = CreateNode("node1", NULL, mnt);
  EXPECT_EQ(0, node1->access(0));
  EXPECT_EQ(0, node1->utime(NULL));
  EXPECT_EQ(0, node1->unlink());

  delete mnt;
  delete node1;
}

TEST(MemNodeTest, ReallocData) {
  MemMount *mnt = new MemMount();
  Node *node1 = CreateNode("node1", NULL, mnt);

  node1->ReallocData(10);
  EXPECT_EQ(10, node1->capacity());
  node1->ReallocData(5);
  EXPECT_EQ(5, node1->capacity());
  node1->ReallocData(400);
  EXPECT_EQ(400, node1->capacity());

  delete mnt;
  delete node1;
}

TEST(MemNodeTest, Access) {
  MemMount *mnt = new MemMount();
  Node *node1 = CreateNode("node1", NULL, mnt);
  int amode;

  amode = F_OK;
  EXPECT_EQ(0, node1->access(amode));

  amode |= R_OK;
  EXPECT_EQ(0, node1->access(amode));

  amode |= W_OK;
  EXPECT_EQ(0, node1->access(amode));

  amode |= X_OK;
  EXPECT_EQ(0, node1->access(amode));

  amode = F_OK | X_OK;
  EXPECT_EQ(0, node1->access(amode));

  amode = R_OK | W_OK;
  EXPECT_EQ(0, node1->access(amode));

  delete mnt;
  delete node1;
}
