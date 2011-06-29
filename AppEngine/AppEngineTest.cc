// Copyright (c) 2011 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstdio>
#include <cstring>
#include <string>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/var.h>
#include "AppEngineMount.h"
#include "../base/MountManager.h"
#include <stdio.h>

class AppEngineTestInstance : public pp::Instance {
 public:
  explicit AppEngineTestInstance(PP_Instance instance)
    : pp::Instance(instance) {
    MountManager *mm = MountManager::MMInstance();
    AppEngineMount *mount = new AppEngineMount(this, "/_file");
    mm->RemoveMount("/");
    mm->AddMount(mount, "/");
    mm_ = mm;
  }
  virtual ~AppEngineTestInstance() {}

  virtual void HandleMessage(const pp::Var& var_message);

 private:
  MountManager *mm_; 
};

void AppEngineTestInstance::HandleMessage(const pp::Var& var_message) {
  if (!var_message.is_string()) {
    return;
  }

  fprintf(stderr, "before first open\n");
  int fd = mm_->kp()->open("/increment.txt", O_CREAT | O_RDONLY, 0);
  int32_t count=0;
  fprintf(stderr, "before read\n");
  mm_->kp()->read(fd, &count, 1);
  ++count;
  fprintf(stderr, "before first close\n");
  mm_->kp()->close(fd);
  fprintf(stderr, "before second open\n");
  fd = mm_->kp()->open("/increment.txt", O_CREAT | O_RDWR, 0);
  fprintf(stderr, "before write\n");
  mm_->kp()->write(fd, &count, sizeof(int));
  fprintf(stderr, "before second close\n");
  mm_->kp()->close(fd);

  //int32_t count=52;
  fprintf(stderr, "before PostMessage() call\n");
  PostMessage(pp::Var(count));
}

class AppEngineTestModule : public pp::Module {
 public:
  AppEngineTestModule() : pp::Module() {
  }
  virtual ~AppEngineTestModule() {}

  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new AppEngineTestInstance(instance);
  }
};

namespace pp {
  Module* CreateModule() {
    return new AppEngineTestModule();
  }
}  // namespace pp


