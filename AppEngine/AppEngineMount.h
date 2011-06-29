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
#include "AppEngineUrlLoader.h"

class AppEngineNode;

namespace pp {
  class Instance;
};

// mem_mount is a storage mount representing local memory.  The node
// class is AppEngineNode.
class AppEngineMount: public Mount {
 public:
  AppEngineMount(pp::Instance *instance, std::string base_url);
  ~AppEngineMount();

  Node2 *Creat(std::string path, mode_t mode);

  // mkdir() creates a new directory at path
  // and mimics the mkdir sys call.  This mkdir
  // implementation ignores the mode parameter.
  int mkdir(std::string path, mode_t mode);

  // Given a path, GetNode() returns the Node
  // corresponding to that path.  The MemMounnt class
  // will return a AppEngineNode (subclass of MountNode).
  // If a node cannot be found at path, NULL is returned.
  Node2 *GetNode(std::string path);

  AppEngineNode *ToAppEngineNode(Node2* node);

  // Temp methods for Node -> Node2 -> ino_t transition period.
  int chmod(Node2* node, mode_t mode);
  int stat(Node2* node, struct stat *buf);
  int remove(Node2* node);
  int rmdir(Node2* node);
  void DecrementUseCount(Node2* node);
  int Getdents(Node2* node, off_t offset, struct dirent *dirp, unsigned int count);
  int Fsync(Node2* node);

  virtual ssize_t Read(Node2* node, off_t offset, void *buf, size_t count);
  virtual ssize_t Write(Node2* node, off_t offset, const void *buf, size_t count);

  pp::AppEngineUrlRequest *url_request() { return &url_request_; }

 private:
  PathHandle *path_handle_;
  pp::AppEngineUrlRequest url_request_;
};

#endif  // PACKAGES_SCRIPTS_FILESYS_APPENGINE_APPENGINEMOUNT_H_
