/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#ifndef PACKAGES_SCRIPTS_FILESYS_BASE_MAINTHREADRUNNER_H_
#define PACKAGES_SCRIPTS_FILESYS_BASE_MAINTHREADRUNNER_H_

#include <pthread.h>
#include <list>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/url_loader.h>
#include <ppapi/cpp/url_request_info.h>
#include <ppapi/cpp/url_response_info.h>
#include <ppapi/cpp/var.h>
#include <ppapi/c/pp_errors.h>
#include <semaphore.h>

struct MainThreadJobEntry;
class MainThreadRunner;

class MainThreadJob {
 public:
  virtual ~MainThreadJob() {}
  virtual void Run(MainThreadJobEntry* e) = 0;
};

struct MainThreadJobEntry {
  pp::Instance *pepper_instance;
  MainThreadRunner *runner;
  MainThreadJob *job;
  sem_t done;
  int32_t result;
};

class MainThreadJobOpen : public MainThreadJob {
 public:
  MainThreadJobOpen(pp::URLLoader* loader, const pp::URLRequestInfo& request_info);

  void Run(MainThreadJobEntry* e) = 0;

 private:
  pp::URLLoader *loader_;
  const pp::URLRequestInfo *request_info_;
};


class MainThreadRunner {
 public:
  MainThreadRunner(pp::Instance *instance);
  ~MainThreadRunner();

  int32_t RunJob(MainThreadJob* job);
  static void StuffResult(void *arg, int32_t result);

 private:
  static void DoWorkShim(void *p, int32_t unused);
  void DoWork(void);

  pthread_mutex_t lock_;
  std::list<MainThreadJobEntry*> job_queue_;
  pp::Instance *pepper_instance_;
};

#endif // PACKAGES_SCRIPTS_FILESYS_BASE_MAINTHREADRUNNER_H_
