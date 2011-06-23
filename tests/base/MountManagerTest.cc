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
  ret = mm->GetMount("/", NULL);
  EXPECT_EQ(ret.first, mnt);
  EXPECT_EQ(0, static_cast<int>(ret.second.length()));
  ret = mm->GetMount("/usr/local/hi", NULL);
  EXPECT_EQ(mnt, ret.first);
  EXPECT_STREQ("usr/local/hi", ret.second.c_str());
  Mount *mnt2 = new Mount();
  EXPECT_EQ(0, mm->AddMount(mnt2, "/home/hi/mount2"));
  ret = mm->GetMount("/home/hi/mount2", NULL);
  EXPECT_EQ(mnt2, ret.first);
  EXPECT_EQ(0, static_cast<int>(ret.second.length()));
  ret = mm->GetMount("/home/hi/mount2/go/down/deeper", NULL);
  EXPECT_EQ(ret.first, mnt2);
  EXPECT_STREQ("/go/down/deeper", ret.second.c_str());
  ret = mm->GetMount("/home/hi", NULL);
  EXPECT_EQ(mnt, ret.first);
  EXPECT_STREQ("home/hi", ret.second.c_str());
  std::string s;
  ret = mm->GetMount(s, NULL);
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

  EXPECT_EQ(-1, mm->chmod(path, 0));
  EXPECT_EQ(-1, mm->remove(path));
  EXPECT_EQ(-1, mm->stat(path, NULL));
  EXPECT_EQ(-1, mm->access(path, 0));
  EXPECT_EQ(0, mm->mkdir(path, 0));
  EXPECT_EQ(-1, mm->rmdir(path));
  EXPECT_EQ(-1, mm->open(path, 0));
  EXPECT_EQ(-1, mm->open(path, 0, 0));  // open has varargs

  EXPECT_EQ(-1, mm->close(fd));
  EXPECT_EQ(-1, mm->close(-10));
  EXPECT_EQ(-1, mm->read(fd, NULL, 0));
  EXPECT_EQ(-1, mm->write(fd, NULL, 0));
  EXPECT_EQ(-1, mm->fstat(fd, NULL));
  EXPECT_EQ(-1, mm->isatty(fd));
  EXPECT_EQ(-1, mm->getdents(fd, NULL, 0));
  EXPECT_EQ(-1, mm->lseek(fd, 0, 0));
  EXPECT_EQ(-1, mm->ioctl(fd, 0, 0));

  EXPECT_EQ(0, mm->RemoveMount("/"));

  mm->ClearMounts();
}

TEST(MountManagerTest, chdir_cwd_wd) {
  // put in a mount
  MemMount *mnt = new MemMount();
  EXPECT_EQ(0, mm->AddMount(mnt, "/"));

  EXPECT_EQ(0, mm->mkdir("/hello/", 0));
  EXPECT_EQ(0, mm->mkdir("/hello/world", 0));

  // char *buf = reinterpret_cast<char *>(malloc(256));
  EXPECT_EQ(0, mm->chdir("hello/world"));
  EXPECT_EQ(0, mm->chdir(".."));
  EXPECT_EQ(0, mm->chdir("world"));
  EXPECT_EQ(0, mm->chdir("."));
  EXPECT_EQ(-1, mm->chdir("hi"));
  EXPECT_EQ(-1, mm->chdir("/hi"));
  EXPECT_EQ(0, mm->chdir(".."));
}

/*
  EXPECT_STREQ("/hello/world/", mm->getwd(buf));
  EXPECT_STREQ("/hello/world/", mm->getcwd(buf, 80));
  EXPECT_STREQ(NULL, mm->getcwd(NULL, 0));

  EXPECT_EQ(0, mm->chdir("hello/world/"));
  EXPECT_STREQ("/hello/world/", mm->getwd(buf));
  EXPECT_STREQ("/hello/world/", mm->getcwd(buf, 80));

  EXPECT_EQ(0, mm->chdir(".."));
  EXPECT_STREQ("/hello/", mm->getwd(buf));
  EXPECT_STREQ("/hello/", mm->getcwd(buf, 80));

  EXPECT_EQ(0, mm->chdir("./world"));
  EXPECT_STREQ("/hello/world/", mm->getwd(buf));
  EXPECT_STREQ("/hello/world/", mm->getcwd(buf, 80));

  EXPECT_EQ(0, mm->chdir(".."));
  EXPECT_STREQ("/", mm->getwd(buf));
  EXPECT_STREQ("/", mm->getcwd(buf, 80));

  EXPECT_EQ(-1, mm->chdir(".."));
  EXPECT_STREQ("/", mm->getwd(buf));
  EXPECT_STREQ("/", mm->getcwd(buf, 80));

  EXPECT_EQ(0, mm->chdir("."));
  EXPECT_STREQ("/", mm->getwd(buf));
  EXPECT_STREQ("/", mm->getcwd(buf, 80));

  // now let's add more mounts
  Mount *mnt2 = new Mount();
  Mount *mnt2b = new Mount();
  Mount *mnt3 = new Mount();

  mm->AddMount(mnt2, "/mounts/mount2");
  mm->AddMount(mnt2b, "/mounts/mount2/extras/mount2b");
  mm->AddMount(mnt3, "/mounts/mount4");

  EXPECT_EQ(0, mm->chdir("./mounts/mount2"));
  EXPECT_STREQ("/mounts/mount2/", mm->getwd(buf));
  EXPECT_STREQ("/mounts/mount2/", mm->getcwd(buf, 80));

  EXPECT_EQ(0, mm->chdir("/mounts/mount2"));
  EXPECT_STREQ("/mounts/mount2/", mm->getwd(buf));
  EXPECT_STREQ("/mounts/mount2/", mm->getcwd(buf, 80));

  EXPECT_EQ(0, mm->chdir("/mounts/mount2/extras/mount2b"));
  EXPECT_STREQ("/mounts/mount2/extras/mount2b/", mm->getwd(buf));
  EXPECT_STREQ("/mounts/mount2/extras/mount2b/", mm->getcwd(buf, 80));

  EXPECT_EQ(0, mm->chdir("/mounts/mount4"));
  EXPECT_STREQ("/mounts/mount4/", mm->getwd(buf));
  EXPECT_STREQ("/mounts/mount4/", mm->getcwd(buf, 80));

  EXPECT_EQ(0, mm->chdir("../mount2/extras/mount2b"));
  EXPECT_STREQ("/mounts/mount2/extras/mount2b/", mm->getwd(buf));
  EXPECT_STREQ("/mounts/mount2/extras/mount2b/", mm->getcwd(buf, 80));

  // now let's cook up more annoying paths
  EXPECT_EQ(0, mm->chdir("/hello/world/"));
  EXPECT_EQ(0, mm->chdir(
    "././././../../hello/world/../world/../world/../../hello/"));
  EXPECT_STREQ("/hello/world/", mm->getwd(buf));
  EXPECT_STREQ("/hello/world/", mm->getcwd(buf, 120));

  EXPECT_EQ(0, mm->chdir(
    "./mounts/mount2/extras/mount2b/down/further/../further/and/further"));
  EXPECT_EQ(0, mm->chdir(".."));
  EXPECT_STREQ("/mounts/mount2/extras/mount2b/down/further/and/",
               mm->getwd(buf));
  EXPECT_STREQ("/mounts/mount2/extras/mount2b/down/further/and/",
               mm->getcwd(buf, 120));

  mm->ClearMounts();
  delete mm;
}

*/
