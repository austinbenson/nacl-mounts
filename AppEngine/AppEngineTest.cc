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
#include <pthread.h>
#include <stdio.h>

class AppEngineTestInstance : public pp::Instance {
 public:
  explicit AppEngineTestInstance(PP_Instance instance)
    : pp::Instance(instance),
      runner_(this),
      app_engine_thread_(0) {
    MountManager *mm = MountManager::MMInstance();
    AppEngineMount *mount = new AppEngineMount(&runner_, "/_file");
    mm->RemoveMount("/");
    mm->AddMount(mount, "/");
    mm_ = mm;
    fd_ = -1;
    count_ = 0;
  }
  virtual ~AppEngineTestInstance() {}

  virtual void HandleMessage(const pp::Var& var_message);
  static void *WriteTestShim(void *p);
  void WriteTest(void);


 private:
  MainThreadRunner runner_;
  MountManager *mm_; 
  int fd_;
  pthread_t app_engine_thread_;
  int32_t count_;
};

void *AppEngineTestInstance::WriteTestShim(void *p) {
  AppEngineTestInstance *inst = (AppEngineTestInstance *)p;
  inst->WriteTest();
  return NULL;
}

void AppEngineTestInstance::WriteTest(void) {
  fprintf(stderr, "Entering WriteTest()\n");
  fprintf(stderr, "Open return: %d\n", fd_ = mm_->kp()->open("/increment.txt", O_CREAT | O_RDWR, 0));
  char *buf = (char *)malloc(3);
  buf[0] = '3';
  buf[1] = '3';
  buf[2] = '0';
  char *buf2 = (char *)malloc(256);
  fprintf(stderr, "Write return: %d\n", mm_->kp()->write(fd_, buf, 2));
  fprintf(stderr, "Fsync return: %d\n", mm_->kp()->fsync(fd_));
  fprintf(stderr, "Getdents return: %d\n", mm_->kp()->getdents(fd_, buf2, 256));
  PostMessage(pp::Var(count_));
}

void AppEngineTestInstance::HandleMessage(const pp::Var& var_message) {
  if (!var_message.is_string()) {
    return;
  }

  fprintf(stderr, "About to create pthread\n");
  pthread_create(&app_engine_thread_, NULL, WriteTestShim, this);
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
