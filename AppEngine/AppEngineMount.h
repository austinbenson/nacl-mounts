/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#ifndef PACKAGES_SCRIPTS_FILESYS_APPENGINE_APPENGINEMOUNT_H_
#define PACKAGES_SCRIPTS_FILESYS_APPENGINE_APPENGINEMOUNT_H_

#include <list>
#include <string>
#include "../base/Mount.h"
#include "../base/PathHandle.h"
#include "../base/SlotAllocator.h"
#include "AppEngineUrlLoader.h"
#include "AppEngineNode.h"

class MainThreadRunner;

class AppEngineMount: public Mount {
 public:
  AppEngineMount(MainThreadRunner *runner, std::string base_url);
  virtual ~AppEngineMount() {}

  void Ref(ino_t node);
  void Unref(ino_t node);

  int Creat(const std::string& path, mode_t mode, struct stat* st);
  int Mkdir(const std::string& path, mode_t mode, struct stat* st);
  int GetNode(const std::string& path, struct stat* st);

  int Rmdir(ino_t node);

  AppEngineNode *ToAppEngineNode(ino_t node) {
    return slots_.At(node);
  }

  int Chmod(ino_t node, mode_t mode);
  int Stat(ino_t node, struct stat *buf);
  int Getdents(ino_t node, off_t offset, struct dirent *dirp, unsigned int count);
  int Fsync(ino_t node);

  virtual ssize_t Read(ino_t node, off_t offset, void *buf, size_t count);
  virtual ssize_t Write(ino_t node, off_t offset, const void *buf, size_t count);

  AppEngineUrlRequest *url_request() { return &url_request_; }

 private:
  PathHandle *path_handle_;
  SlotAllocator<AppEngineNode> slots_;
  AppEngineUrlRequest url_request_;
};

#endif  // PACKAGES_SCRIPTS_FILESYS_APPENGINE_APPENGINEMOUNT_H_
