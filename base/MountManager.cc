/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#include "MountManager.h"

static pthread_once_t mount_manager_once_ = PTHREAD_ONCE_INIT;
MountManager *MountManager::mm_instance_;

MountManager::MountManager() {
  Init();
}

MountManager::~MountManager() {
  mount_map_.clear();
}

MountManager *MountManager::MMInstance() {
  pthread_once(&mount_manager_once_, Instantiate);
  return mm_instance_;
}

void MountManager::Instantiate() {
  if (!mm_instance_)
    mm_instance_ = new MountManager();
}

void MountManager::Init() {
  if (pthread_mutex_init(&lock_, NULL)) assert(false);
  // add a memory mount at "/" to function
  // as the deafault mount
  MemMount *default_mount = new MemMount();
  assert(default_mount);
  AddMount(default_mount, "/");
  cwd_mount_ = default_mount;
  kp_.Init(this);
}

int MountManager::AddMount(Mount *m, const char *path) {
  if (!path)
    return -3;
  std::string p(path);
  AcquireLock();
  Mount *mount = mount_map_[path];
  if (mount) {
    ReleaseLock();
    return -1;
  }
  if (!m) {
    ReleaseLock();
    return -2;
  }
  if (p.length() == 0) {
    ReleaseLock();
    return -3;
  }
  mount_map_[path] = m;
  ReleaseLock();
  return 0;
}

int MountManager::RemoveMount(const char *path) {
  AcquireLock();
  std::string p(path);
  std::map<std::string, Mount *>::iterator it;
  it = mount_map_.find(p);
  if (it == mount_map_.end()) {
    ReleaseLock();
    return -1;
  } else {
    if (cwd_mount_ == it->second)
      cwd_mount_ = NULL;
    // erase() calls the destructor
    mount_map_.erase(it);
    ReleaseLock();
    return 0;
  }
}

void MountManager::ClearMounts(void) {
  mount_map_.clear();
  cwd_mount_ = NULL;
}

void MountManager::AcquireLock(void) {
  if (pthread_mutex_lock(&lock_)) assert(0);
}

void MountManager::ReleaseLock(void) {
  if (pthread_mutex_unlock(&lock_)) assert(0);
}

Node *MountManager::GetNode(std::string path) {
  std::pair<Mount *, std::string> m_and_p;

  // check if path is of length zero
  if (path.length() == 0)
    return NULL;

  // check if the path is an absoulte path
  if (path[0] == '/') {
    m_and_p = GetMount(path);
  }

  if ((m_and_p.second).length() == 0) {
    return NULL;
  } else {
    if (!m_and_p.first)
      return NULL;
    return m_and_p.first->GetNode(m_and_p.second);
  }
}

std::pair<Mount *, std::string> MountManager::GetMount(std::string path) {
  std::pair<Mount *, std::string> ret;
  std::map<std::string, Mount *>::iterator it;
  std::string curr_best = "";
  ret.first = NULL;
  ret.second = path;

  if (path.length() == 0)
    return ret;

  // Find the longest path in the map that matches
  // the start of path
  for (it = mount_map_.begin();
       it != mount_map_.end() && path.length() != 0; ++it)
    if (path.find(it->first) == 0)
      if (it->first.length() > curr_best.length())
        curr_best = it->first;

  if (curr_best.length() == 0)
    return ret;

  ret.first = mount_map_[curr_best];
  // if the path matches exactly, returned path is empty string
  if (curr_best.compare(path) == 0)
    ret.second = "/";
  else
    ret.second = path.substr(curr_best.length());
  return ret;
}
