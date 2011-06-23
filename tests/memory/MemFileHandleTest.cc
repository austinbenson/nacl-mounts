/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */

#include "../../memory/MemFileHandle.h"
#include "../../memory/MemMount.h"
#include "../../memory/MemNode.h"
#include "../common/common.h"
#include "TestHelpCommon.h"

TEST(MemFileHandleTest, lseek) {
  MemMount *mount = new MemMount();
  MemNode *node = CreateNode("node", NULL, mount, true);
  MemFileHandle *handle = CreateMemFileHandle(mount, node, 1, 0, 0);

  int whence = 0;
  // check for when node is a directory
  EXPECT_EQ(-1, handle->lseek(0, whence));

  node->set_is_dir(false);
  whence = SEEK_SET;
  EXPECT_EQ(-1, handle->lseek(-8, whence));
  EXPECT_EQ(5, handle->lseek(5, whence));
  EXPECT_EQ(0, handle->lseek(0, whence));
  EXPECT_EQ(50, handle->lseek(50, whence));

  // offset should now be set to 50
  whence = SEEK_CUR;
  EXPECT_EQ(99, handle->lseek(49, whence));
  EXPECT_EQ(-1, handle->lseek(-200, whence));
  EXPECT_EQ(99, handle->lseek(0, whence));
  EXPECT_EQ(2, handle->lseek(-97, whence));

  // offset should now be set to 2
  whence = SEEK_END;
  node->set_len(400);
  EXPECT_EQ(201, handle->lseek(199, whence));
  EXPECT_EQ(196, handle->lseek(204, whence));
  EXPECT_EQ(-1, handle->lseek(401, whence));
  EXPECT_EQ(0, handle->lseek(400, whence));

  delete mount;
  delete node;
  delete handle;
}


TEST(MemFileHandleTest, getdents) {
  MemMount *mount = new MemMount();
  MemNode *node = CreateNode("node", NULL, mount, true);
  MemNode *child1 = CreateNode("child1", node, mount, false);
  MemNode *child2 = CreateNode("child2", node, mount, false);
  MemNode *child3 = CreateNode("child3", node, mount, false);
  MemNode *child4 = CreateNode("child4", node, mount, false);
  MemNode *child5 = CreateNode("child5", node, mount, false);
  node->AddChild(child1);
  node->AddChild(child2);
  node->AddChild(child3);
  node->AddChild(child4);
  node->AddChild(child5);
  MemFileHandle *handle = CreateMemFileHandle(mount, node, 1, 0, 0);
  struct dirent *buf;

  int size = 5*sizeof(struct dirent);
  buf = reinterpret_cast<struct dirent *>(calloc(size, 1));

  EXPECT_EQ(size, handle->getdents(buf, size));
  EXPECT_STREQ(child3->name().c_str(), (buf+2)->d_name);

  free(buf);
  size = 6*sizeof(struct dirent);
  buf = reinterpret_cast<struct dirent *>(malloc(size));

  node->RemoveChild(child4);
  node->RemoveChild(child5);

  EXPECT_EQ(static_cast<int>(3*sizeof(struct dirent)),
            handle->getdents(buf, size));
  EXPECT_STREQ(child3->name().c_str(), (buf+2)->d_name);
  EXPECT_STRNE(child4->name().c_str(), (buf+3)->d_name);

  free(buf);
  size = 2*sizeof(struct dirent);
  buf = reinterpret_cast<struct dirent *>(malloc(size));
  EXPECT_EQ(size, handle->getdents(buf, size));
  EXPECT_STREQ(child1->name().c_str(), buf->d_name);

  delete child1;
  delete child2;
  delete child3;
  delete child4;
  delete child5;
  delete mount;
  delete node;
  delete handle;
}

TEST(MemFileHandleTest, fstat) {
  MemMount *mount = new MemMount();
  MemNode *node = CreateNode("node", NULL, mount, true);
  MemFileHandle *handle = CreateMemFileHandle(mount, node, 1, 0, 0);

  struct stat *buf = (struct stat *)malloc(sizeof(struct stat));
  // fstat should handle zero-ing out the buffer
  EXPECT_EQ(0, handle->fstat(buf));
  EXPECT_EQ(0, buf->st_size);
  node->set_is_dir(false);
  int len = 10;
  node->set_len(len);
  EXPECT_EQ(0, handle->fstat(buf));
  EXPECT_EQ(len, buf->st_size);

  free(buf);
  delete mount;
  delete node;
  delete handle;
}

TEST(MemFileHandleTest, close) {
  MemMount *mount = new MemMount();
  MemNode *node = CreateNode("node", NULL, mount, true);
  MemFileHandle *handle = CreateMemFileHandle(mount, node, 1, 0, 0);

  handle->set_fd(7);
  EXPECT_EQ(0, handle->close());
  EXPECT_EQ(false, handle->in_use());

  delete mount;
  delete node;
  delete handle;
}

TEST(MemFileHandleTest, ioctl) {
  MemMount *mount = new MemMount();
  MemNode *node = CreateNode("node", NULL, mount, true);
  MemFileHandle *handle = CreateMemFileHandle(mount, node, 1, 0, 0);

  // expecting ioctl to not be implemented
  EXPECT_EQ(-1, handle->ioctl(0, NULL));

  delete mount;
  delete node;
  delete handle;
}

TEST(MemFileHandleTest, write) {
  MemMount *mount = new MemMount();
  MemNode *node = CreateNode("node", NULL, mount, true);
  MemFileHandle *handle = CreateMemFileHandle(mount, node, 1, 0, O_RDWR);
  char *data;
  int num_to_match = 4;
  char *buf;

  buf = reinterpret_cast<char *>(malloc(120));
  for (int i = 0; i < num_to_match; i++) {
    buf[i] = (i * 31 + 17) % 256;
  }

  EXPECT_EQ(-1, handle->write(buf, 120));

  node->set_is_dir(false);
  int whence = SEEK_SET;
  handle->lseek(50, whence);
  node->ReallocData(300);
  EXPECT_EQ(120, handle->write(buf, 120));
  data = node->data();
  for (int i = 0; i < num_to_match; i++) {
    EXPECT_EQ(buf[i], data[i+50]);
  }

  node->ReallocData(100);
  handle->lseek(75, whence);

  EXPECT_EQ(120, handle->write(buf, 120));
  data = node->data();
  for (int i = 0; i < num_to_match; i++) {
    EXPECT_EQ(buf[i], data[i+75]);
  }

  free(buf);
  buf = reinterpret_cast<char *>(malloc(400));
  for (int i = 0; i < num_to_match; i++) {
    buf[i] = (i * 47 + 23) % 256;
  }

  node->ReallocData(140);
  handle->lseek(100, whence);
  EXPECT_EQ(400, handle->write(buf, 400));
  data = node->data();
  for (int i = 0; i < num_to_match; i++) {
    EXPECT_EQ(buf[i], data[i+100]);
  }

  free(buf);
  delete node;
  delete handle;
  //  delete mount;
}


TEST(MemFileHandleTest, read) {
  MemMount *mount = new MemMount();
  MemNode *node = CreateNode("node", NULL, mount, true);
  MemFileHandle *handle = CreateMemFileHandle(mount, node, 1, 0, O_RDWR);
  int num_to_match = 4;
  char *buf_r, *buf_w, *data;

  buf_r = reinterpret_cast<char *>(malloc(120));
  buf_w = reinterpret_cast<char *>(malloc(40));
  for (int i = 0; i < num_to_match; ++i) {
    buf_w[i] = (i * 31 + 17) % 256;
  }

  EXPECT_EQ(-1, handle->read(buf_r, 120));

  node->set_is_dir(false);
  node->ReallocData(50);
  EXPECT_EQ(40, handle->write(buf_w, 40));
  handle->lseek(0, SEEK_SET);
  EXPECT_EQ(40, handle->read(buf_r, 120));
  data = node->data();
  for (int i = 0; i < num_to_match; ++i) {
    EXPECT_EQ(buf_r[i], buf_w[i]);
    EXPECT_EQ(buf_r[i], data[i]);
  }

  int whence = SEEK_SET;
  node->ReallocData(40);
  handle->lseek(10, whence);
  free(buf_r);
  free(buf_w);
  buf_r = reinterpret_cast<char *>(malloc(10));
  buf_w = reinterpret_cast<char *>(malloc(20));
  for (int i = 0; i < num_to_match; ++i) {
    buf_w[i] = (i * 47 + 23) % 256;
  }

  handle->write(buf_w, 20);
  handle->lseek(8, whence);

  EXPECT_EQ(10, handle->read(buf_r, 10));
  data = node->data();
  for (int i = 0; i < num_to_match; ++i) {
    EXPECT_EQ(buf_r[i+2], buf_w[i]);
    EXPECT_EQ(buf_r[i+2], data[i+10]);
  }

  free(buf_r);
  free(buf_w);
  delete mount;
  delete node;
  delete handle;
}
