#include "MainThreadRunner.h"
#include <stdio.h>
#include "../AppEngine/AppEngineUrlLoader.h"

MainThreadJobOpen::MainThreadJobOpen(pp::URLLoader* loader, const pp::URLRequestInfo& request_info) {
  loader_ = loader;
  request_info_ = &request_info;
}

void MainThreadJobOpen::Run(MainThreadJobEntry* e) {
  loader_->Open(*request_info_, pp::CompletionCallback(MainThreadRunner::StuffResult, e));
  delete this;
}

MainThreadRunner::MainThreadRunner(pp::Instance *instance) { 
  pepper_instance_ = instance;
  DoWork();
  pthread_mutex_init(&lock_, NULL);
}

MainThreadRunner::~MainThreadRunner() { 
  pthread_mutex_destroy(&lock_);
}

int32_t MainThreadRunner::RunJob(MainThreadJob* job) {
  MainThreadJobEntry e;
 
  e.runner = this; 
  e.pepper_instance = pepper_instance_;
  e.job = job;
  sem_init(&e.done, 0, 0);
  fprintf(stderr, "Waiting for lock...\n");
  pthread_mutex_lock(&lock_);
  fprintf(stderr, "Got lock\n");
  job_queue_.push_back(&e);
  pthread_mutex_unlock(&lock_);
  fprintf(stderr, "Released lock, waiting on sem...\n");
  sem_wait(&e.done);
  fprintf(stderr, "Sem signaled\n");
  sem_destroy(&e.done);
  return e.result;
}

void MainThreadRunner::StuffResult(void *arg, int32_t result) {
  MainThreadJobEntry *e = reinterpret_cast<MainThreadJobEntry*>(arg);

  e->result = result;
  DoWorkShim(e->runner, 0);
  sem_post(&e->done);
}

void MainThreadRunner::DoWorkShim(void *p, int32_t unused) {
  MainThreadRunner *mtr = (MainThreadRunner *)p;
  mtr->DoWork();
}

void MainThreadRunner::DoWork(void) {
  //fprintf(stderr, "in DoWork()\n");
  pthread_mutex_lock(&lock_);
  if (!job_queue_.empty()) { 
    MainThreadJobEntry *e = job_queue_.front();
    job_queue_.pop_front();
    if (e == NULL || e->job == NULL) fprintf(stderr, "NULL POINTERS\n");
    fprintf(stderr, "about to Run\n");
    e->job->Run(e);
  } else {
    //fprintf(stderr, "Creating callback to DoWorkShim\n");
    pp::Module::Get()->core()->CallOnMainThread(10, pp::CompletionCallback(&DoWorkShim, this), PP_OK);
  }
  pthread_mutex_unlock(&lock_);
}


