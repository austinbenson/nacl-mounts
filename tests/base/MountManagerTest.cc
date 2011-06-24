/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */

#include "../../base/Mount.h"
#include "../../base/MountManager.h"
#include "../../memory/MemMount.h"
#include "../common/common.h"

MountManager *mm = MountManager::MMInstance();

TEST(MountManagerTest, AddRemoveMount) {
  Mount *mnt = new Mount();

  // should start with a default mount at "/"
  EXPECT_EQ(-1, mm->AddMount(mnt, "/"));
  EXPECT_EQ(0, mm->RemoveMount("/"));

  EXPECT_EQ(0, mm->AddMount(mnt, "/"));
  EXPECT_EQ(0, mm->AddMount(mnt, "/hi"));
  EXPECT_EQ(-1, mm->AddMount(mnt, "/hi"));
  EXPECT_EQ(0, mm->RemoveMount("/hi"));
  EXPECT_EQ(-1, mm->RemoveMount("/hi"));
  EXPECT_EQ(-1, mm->RemoveMount("/hi"));
  EXPECT_EQ(0, mm->AddMount(mnt, "/hi"));
  EXPECT_EQ(0, mm->RemoveMount("/hi"));
  EXPECT_EQ(0, mm->RemoveMount("/"));
  EXPECT_EQ(-1, mm->RemoveMount("/"));
  EXPECT_EQ(0, mm->AddMount(mnt, "/"));
  Mount *mnt2 = new Mount();
  EXPECT_EQ(-1, mm->AddMount(mnt2, "/"));
  EXPECT_EQ(0, mm->AddMount(mnt2, "/usr/local/mount2"));
  Mount *mnt3 = new Mount();
  EXPECT_EQ(0, mm->AddMount(mnt3, "/usr/local/mount3"));
  // remove all of the mounts
  EXPECT_EQ(0, mm->RemoveMount("/"));
  EXPECT_EQ(0, mm->RemoveMount("/usr/local/mount2"));
  EXPECT_EQ(0, mm->RemoveMount("/usr/local/mount3"));

  EXPECT_EQ(-2, mm->AddMount(NULL, "/usr/share"));
  EXPECT_EQ(-3, mm->AddMount(mnt, ""));
  EXPECT_EQ(-3, mm->AddMount(mnt, NULL));
  EXPECT_NE(0, mm->AddMount(NULL, ""));

  mm->ClearMounts();
}

TEST(MountManagerTest, GetMount) {
  std::pair<Mount *, std::string> ret;
  Mount *mnt = new Mount();

  EXPECT_EQ(0, mm->AddMount(mnt, "/"));
  ret = mm->GetMount("/");
  EXPECT_EQ(ret.first, mnt);
  EXPECT_STREQ("/", ret.second.c_str());
  ret = mm->GetMount("/usr/local/hi");
  EXPECT_EQ(mnt, ret.first);
  EXPECT_STREQ("usr/local/hi", ret.second.c_str());
  Mount *mnt2 = new Mount();
  EXPECT_EQ(0, mm->AddMount(mnt2, "/home/hi/mount2"));
  ret = mm->GetMount("/home/hi/mount2");
  EXPECT_EQ(mnt2, ret.first);
  EXPECT_STREQ("/", ret.second.c_str());
  ret = mm->GetMount("/home/hi/mount2/go/down/deeper");
  EXPECT_EQ(ret.first, mnt2);
  EXPECT_STREQ("/go/down/deeper", ret.second.c_str());
  ret = mm->GetMount("/home/hi");
  EXPECT_EQ(mnt, ret.first);
  EXPECT_STREQ("home/hi", ret.second.c_str());
  std::string s;
  ret = mm->GetMount(s);
  EXPECT_EQ(0, static_cast<int>(ret.second.length()));

  mm->ClearMounts();
}

TEST(MountManagerTest, RoutedSysCalls) {
  // put in a mount
  Mount *mnt = new Mount();
  EXPECT_EQ(0, mm->AddMount(mnt, "/"));

  // Run through each sys call
  // No call should do anything useful, but
  // they should be able to run
  const char *path = "/hi/there";
  int fd = 5;

  EXPECT_EQ(-1, mm->kp()->chmod(path, 0));
  EXPECT_EQ(-1, mm->kp()->remove(path));
  EXPECT_EQ(-1, mm->kp()->stat(path, NULL));
  EXPECT_EQ(-1, mm->kp()->access(path, 0));
  EXPECT_EQ(0, mm->kp()->mkdir(path, 0));
  EXPECT_EQ(-1, mm->kp()->rmdir(path));
  EXPECT_EQ(-1, mm->kp()->open(path, 0));
  EXPECT_EQ(-1, mm->kp()->open(path, 0, 0));  // open has varargs

  EXPECT_EQ(-1, mm->kp()->close(fd));
  EXPECT_EQ(-1, mm->kp()->close(-10));
  EXPECT_EQ(-1, mm->kp()->read(fd, NULL, 0));
  EXPECT_EQ(-1, mm->kp()->write(fd, NULL, 0));
  EXPECT_EQ(-1, mm->kp()->fstat(fd, NULL));
  EXPECT_EQ(-1, mm->kp()->isatty(fd));
  EXPECT_EQ(-1, mm->kp()->getdents(fd, NULL, 0));
  EXPECT_EQ(-1, mm->kp()->lseek(fd, 0, 0));
  EXPECT_EQ(-1, mm->kp()->ioctl(fd, 0, 0));

  EXPECT_EQ(0, mm->RemoveMount("/"));

  mm->ClearMounts();
}

TEST(MountManagerTest, chdir_cwd_wd) {
  MemMount *mnt1, *mnt2, *mnt3;
  char *buf = reinterpret_cast<char *>(malloc(300));
  mnt1 = new MemMount();
  // put in a mount
  EXPECT_EQ(0, mm->AddMount(mnt1, "/"));
  EXPECT_STREQ("/", mm->kp()->getcwd(buf, 300));
  EXPECT_STREQ("/", mm->kp()->getwd(buf));

  EXPECT_EQ(0, mm->kp()->mkdir("/hello/", 0));
  EXPECT_EQ(0, mm->kp()->mkdir("/hello/world", 0));


  EXPECT_EQ(0, mm->kp()->chdir("/hello/world"));
  EXPECT_STREQ("/hello/world", mm->kp()->getcwd(buf, 300));
  EXPECT_STREQ("/hello/world", mm->kp()->getwd(buf));
  EXPECT_EQ(0, mm->kp()->chdir("/hello"));
  EXPECT_STREQ("/hello", mm->kp()->getcwd(buf, 300));
  EXPECT_STREQ("/hello", mm->kp()->getwd(buf));
  EXPECT_EQ(0, mm->kp()->chdir("/hello/world"));
  EXPECT_STREQ("/hello/world", mm->kp()->getcwd(buf, 300));
  EXPECT_STREQ("/hello/world", mm->kp()->getwd(buf));
  EXPECT_EQ(0, mm->kp()->chdir("/hello/world"));
  EXPECT_STREQ("/hello/world", mm->kp()->getcwd(buf, 300));
  EXPECT_STREQ("/hello/world", mm->kp()->getwd(buf));
  EXPECT_EQ(-1, mm->kp()->chdir("/hello/world/hi"));
  EXPECT_EQ(-1, mm->kp()->chdir("/hi"));
  EXPECT_EQ(0, mm->kp()->chdir("/"));
  EXPECT_STREQ("/", mm->kp()->getcwd(buf, 300));
  EXPECT_STREQ("/", mm->kp()->getwd(buf));

  mnt2 = new MemMount();
  EXPECT_EQ(0, mm->AddMount(mnt2, "/usr/mount2"));
  EXPECT_EQ(-2, mm->AddMount(NULL, "/usr/mount3"));
  mnt3 = new MemMount();
  EXPECT_EQ(0, mm->AddMount(mnt3, "/usr/mount3"));
  EXPECT_EQ(0, mm->kp()->mkdir("/usr", 0));
  EXPECT_EQ(-1, mm->kp()->mkdir("/usr/mount2", 0));
  EXPECT_EQ(0, mm->kp()->mkdir("/usr/mount2/hello", 0));
  EXPECT_EQ(0, mm->kp()->mkdir("/usr/mount2/hello/world", 0));
  EXPECT_EQ(-1, mm->kp()->mkdir("/usr/mount3", 0));
  EXPECT_EQ(0, mm->kp()->chdir("/usr/mount2/hello/world"));
  EXPECT_STREQ("/usr/mount2/hello/world", mm->kp()->getcwd(buf, 300));
  EXPECT_STREQ("/usr/mount2/hello/world", mm->kp()->getwd(buf));
  EXPECT_EQ(0, mm->kp()->chdir("/"));
  EXPECT_STREQ("/", mm->kp()->getcwd(buf, 300));
  EXPECT_STREQ("/", mm->kp()->getwd(buf));
  EXPECT_EQ(0, mm->kp()->chdir("/usr/mount2/hello"));
  EXPECT_STREQ("/usr/mount2/hello", mm->kp()->getcwd(buf, 300));
  EXPECT_STREQ("/usr/mount2/hello", mm->kp()->getwd(buf));

  mm->ClearMounts();
}


TEST(MountManagerTest, BasicOpen) {
  MemMount *mnt = new MemMount();
  EXPECT_EQ(0, mm->AddMount(mnt, "/"));
  EXPECT_EQ(0, mm->kp()->open("/test.txt", O_CREAT, 0));
}

