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
  mm->ClearMounts();
  std::pair<Mount *, std::string> ret;
  Mount *mnt = new Mount();

  EXPECT_EQ(0, mm->AddMount(mnt, "/"));
  ret = mm->GetMount("/");
  EXPECT_EQ(ret.first, mnt);
  EXPECT_EQ("/", ret.second);
  ret = mm->GetMount("/usr/local/hi");
  EXPECT_EQ(mnt, ret.first);
  EXPECT_EQ("usr/local/hi", ret.second);
  Mount *mnt2 = new Mount();
  EXPECT_EQ(0, mm->AddMount(mnt2, "/home/hi/mount2"));
  ret = mm->GetMount("/home/hi/mount2");
  EXPECT_EQ(mnt2, ret.first);
  EXPECT_EQ("/", ret.second);
  ret = mm->GetMount("/home/hi/mount2/go/down/deeper");
  EXPECT_EQ(ret.first, mnt2);
  EXPECT_EQ("/go/down/deeper", ret.second);
  ret = mm->GetMount("/home/hi");
  EXPECT_EQ(mnt, ret.first);
  EXPECT_EQ("home/hi", ret.second);
  std::string s;
  ret = mm->GetMount(s);
  EXPECT_EQ(0, static_cast<int>(ret.second.length()));

  mm->ClearMounts();
}

TEST(MountManagerTest, RoutedSysCalls) {
  // put in a mount
  mm->ClearMounts();
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
  EXPECT_EQ(-1, mm->kp()->mkdir(path, 0));
  EXPECT_EQ(-1, mm->kp()->rmdir(path));
  EXPECT_EQ(-1, mm->kp()->open(path, 0, 0));

  EXPECT_EQ(-1, mm->kp()->close(fd));
  EXPECT_EQ(-1, mm->kp()->close(-10));
  EXPECT_EQ(-1, mm->kp()->read(fd, NULL, 0));
  EXPECT_EQ(-1, mm->kp()->write(fd, NULL, 0));
  EXPECT_EQ(-1, mm->kp()->fstat(fd, NULL));
  EXPECT_EQ(-1, mm->kp()->getdents(fd, NULL, 0));
  EXPECT_EQ(-1, mm->kp()->lseek(fd, 0, 0));
  EXPECT_EQ(-1, mm->kp()->ioctl(fd, 0));

  EXPECT_EQ(0, mm->RemoveMount("/"));

  mm->ClearMounts();
}

#define CHECK_WD(want) { \
  std::string wd; \
  EXPECT_TRUE(mm->kp()->getcwd(&wd, 300)); \
  EXPECT_EQ(want, wd); \
  wd.clear(); \
  EXPECT_TRUE(mm->kp()->getwd(&wd)); \
  EXPECT_EQ(want, wd); \
}

TEST(MountManagerTest, chdir_cwd_wd) {
  mm->ClearMounts();
  MemMount *mnt1, *mnt2, *mnt3;
  mnt1 = new MemMount();
  // put in a mount
  EXPECT_EQ(0, mm->AddMount(mnt1, "/"));

  CHECK_WD("/");

  EXPECT_EQ(0, mm->kp()->mkdir("/hello/", 0));
  EXPECT_EQ(0, mm->kp()->mkdir("/hello/world", 0));


  EXPECT_EQ(0, mm->kp()->chdir("/hello/world"));
  CHECK_WD("/hello/world");
  EXPECT_EQ(0, mm->kp()->chdir("/hello"));
  CHECK_WD("/hello");
  EXPECT_EQ(0, mm->kp()->chdir("/hello/world"));
  CHECK_WD("/hello/world");
  EXPECT_EQ(-1, mm->kp()->chdir("/hello/world/hi"));
  CHECK_WD("/hello/world");
  EXPECT_EQ(-1, mm->kp()->chdir("/hi"));
  EXPECT_EQ(0, mm->kp()->chdir("/"));
  CHECK_WD("/");

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
  CHECK_WD("/usr/mount2/hello/world");

  EXPECT_EQ(0, mm->kp()->chdir("/"));
  CHECK_WD("/");

  EXPECT_EQ(0, mm->kp()->chdir("/usr/mount2/hello"));
  CHECK_WD("/usr/mount2/hello");

  // Now we try some relative paths and ".."
  EXPECT_EQ(0, mm->kp()->chdir(".."));
  CHECK_WD("/usr/mount2");
  EXPECT_EQ(0, mm->kp()->chdir("/../..//"));
  CHECK_WD("/");
  EXPECT_EQ(0, mm->kp()->chdir("usr/mount2/hello/./"));
  CHECK_WD("/usr/mount2/hello");
  EXPECT_EQ(0, mm->kp()->mkdir("/usr/mount3/hello", 0));
  EXPECT_EQ(0, mm->kp()->chdir("../../mount3/hello"));
  CHECK_WD("/usr/mount3/hello");
  EXPECT_EQ(0, mm->kp()->mkdir("world", 0));
  EXPECT_EQ(0, mm->kp()->chdir("world"));
  CHECK_WD("/usr/mount3/hello/world");
  EXPECT_EQ(0, mm->kp()->chdir("."));
  CHECK_WD("/usr/mount3/hello/world");
  EXPECT_EQ(0, mm->kp()->chdir("../../../../../"));
  CHECK_WD("/");

  mm->ClearMounts();
}


TEST(MountManagerTest, BasicOpen) {
  mm->ClearMounts();
  MemMount *mnt = new MemMount();
  EXPECT_EQ(0, mm->AddMount(mnt, "/"));
  mm->kp()->chdir("/");

  EXPECT_EQ(0, mm->kp()->open("/test.txt", O_CREAT, 0));
  EXPECT_EQ(1, mm->kp()->open("hi.txt", O_CREAT, 0));

  mm->ClearMounts();
}

TEST(MountManagerTest, access) {
  mm->ClearMounts();
  MemMount *mnt = new MemMount();
  EXPECT_EQ(0, mm->AddMount(mnt, "/"));
  mm->kp()->chdir("/");

  EXPECT_EQ(0, mm->kp()->mkdir("/hello", 0));
  EXPECT_EQ(0, mm->kp()->mkdir("/hello/world", 0));
  EXPECT_NE(-1, mm->kp()->open("/hello/world/test.txt", O_CREAT, 0));

  int amode = F_OK;
  EXPECT_EQ(0, mm->kp()->access("/hello/world/test.txt", amode));
  EXPECT_EQ(0, mm->kp()->access("/", amode));

  amode |= R_OK;
  EXPECT_EQ(0, mm->kp()->access("/hello/world/test.txt", amode));
  EXPECT_EQ(0, mm->kp()->access("/", amode));

  amode |= W_OK;
  EXPECT_EQ(0, mm->kp()->access("/hello/world/test.txt", amode));
  EXPECT_EQ(0, mm->kp()->access("/", amode));

  amode |= X_OK;
  EXPECT_EQ(0, mm->kp()->access("/hello/world/test.txt", amode));
  EXPECT_EQ(0, mm->kp()->access("/", amode));

  EXPECT_EQ(0, mm->kp()->chdir("/"));
  EXPECT_EQ(0, mm->kp()->access("/", amode));
  EXPECT_EQ(0, mm->kp()->access("hello/world/test.txt", amode));

}
