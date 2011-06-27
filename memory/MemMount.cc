/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#include "MemMount.h"
#include "MemNode.h"
#include "../base/dirent.h"
#include <assert.h>
#include <errno.h>

MemMount::MemMount() {
  pthread_mutex_init(&lock_, NULL);
  root_ = new MemNode();
  root_->set_is_dir(true);
  root_->set_name("/");
}

MemMount::~MemMount() {
  delete root_;
}

MemNode* MemMount::ToMemNode(Node2 *node) {
  if (node == NULL) {
    return NULL;
  }
  return reinterpret_cast<MemNode2*>(node)->node();
}

Node2 *MemMount::MountOpen(std::string path, int oflag, mode_t mode) {
  MemNode *node;
  MemNode *parent;

  AcquireLock();

  // Get the directory its in.
  parent = GetParentMemNode(path);
  if (!parent) {
    errno = ENOTDIR;
    ReleaseLock();
    return NULL;
  }
  // It must be a directory.
  if (!(parent->is_dir())) {
    errno = ENOTDIR;
    ReleaseLock();
    return NULL;
  }
  // See if file exists.
  node = GetMemNode(path);
  if (node) {
    // Check that it is a file if it does.
    if (node->is_dir() && oflag != 0) {
      errno = EISDIR;
      ReleaseLock();
      return NULL;
    }
    // Check that we weren't expecting to create it.
    if ((oflag & O_CREAT) && (oflag & O_EXCL)) {
      errno = EEXIST;
      ReleaseLock();
      return NULL;
    }
  } else {
    // Check that we can create it.
    if (!(oflag & O_CREAT)) {
      errno = ENOENT;
      ReleaseLock();
      return NULL;
    }
    // Create it.
    node = new MemNode();
    node->set_is_dir(false);
    node->set_mount(this);
    std::string p(path);
    PathHandle ph(p);
    node->set_name(ph.Last());
    // Add it to parent.
    parent->AddChild(node);
  }
  // Truncate the file if relevant.
  if (oflag & O_TRUNC) {
    node->Truncate();
  }

  node->IncrementUseCount();

  ReleaseLock();
  return new MemNode2(node);
}

int MemMount::mkdir(std::string path, mode_t mode) {
  MemNode *node;
  MemNode *nnode;

  AcquireLock();

  // Make sure it doesn't already exist.
  node = GetMemNode(path);
  if (node) {
    errno = EEXIST;
    ReleaseLock();
    return -1;
  }
  // Get the parent node.
  node = GetParentMemNode(path);
  if (!node) {
    errno = ENOENT;
    ReleaseLock();
    return -1;
  }
  // Check that parent is a directory.
  if (!node->is_dir()) {
    errno = ENOTDIR;
    ReleaseLock();
    return -1;
  }
  // Create a new node
  nnode = new MemNode();
  nnode->set_mount(this);
  nnode->set_is_dir(true);
  PathHandle ph(path);
  nnode->set_name(ph.Last());
  nnode->set_parent(node);
  node->AddChild(nnode);

  ReleaseLock();
  return 0;
}


void MemMount::AcquireLock(void) {
  if (pthread_mutex_lock(&lock_)) assert(0);
}

void MemMount::ReleaseLock(void) {
  if (pthread_mutex_unlock(&lock_)) assert(0);
}

Node2 *MemMount::GetNode(std::string path) {
  MemNode* node = GetMemNode(path);
  if (node == NULL) {
    return NULL;
  }
  return new MemNode2(node);
}

MemNode *MemMount::GetParentNode(std::string path) {
  return GetParentMemNode(path);
}

MemNode *MemMount::GetMemNode(std::string path) {
  MemNode *node;
  std::list<std::string> path_components;
  std::list<MemNode *>::iterator it;
  std::list<MemNode *> *children;

  // Get in canonical form.
  if (path.length() == 0)
    return NULL;
  // Check if it is an absolute path
  PathHandle ph(path);
  path_components = ph.path();

  // Walk up from root.
  node = root_;
  std::list<std::string>::iterator path_it;
  // loop through path components
  for (path_it = path_components.begin();
       path_it != path_components.end(); ++path_it) {
    // check if we are at a non-directory
    if (!(node->is_dir())) {
      errno = ENOTDIR;
      return NULL;
    }
    // loop through children
    children = node->children();
    for (it = children->begin(); it != children->end(); ++it) {
      if (((*it)->name()).compare(*path_it) == 0) {
        break;
      }
    }
    // check for failure
    if (it == children->end()) {
      errno = ENOENT;
      return NULL;
    } else {
      node = *it;
    }
  }
  // We should now have completed the walk.
  if (node == root_ && path_components.size() > 1) return NULL;
  return node;
}

MemNode *MemMount::GetParentMemNode(std::string path) {
  return GetMemNode(path + "/..");
}

bool MemMount::is_dir(Node2 *node) {
  return ToMemNode(node)->is_dir();
}

size_t MemMount::len(Node2 *node) {
  return ToMemNode(node)->len();
}

int MemMount::capacity(Node2 *node) {
  return ToMemNode(node)->capacity();
}

int MemMount::chmod(Node2 *node, mode_t mode) {
  return ToMemNode(node)->chmod(mode);
}

int MemMount::stat(Node2* node, struct stat *buf) {
  return ToMemNode(node)->stat(buf);
}

int MemMount::remove(Node2* node) {
  return ToMemNode(node)->remove();
}

int MemMount::access(Node2* node, int amode) {
  return ToMemNode(node)->access(amode);
}

int MemMount::rmdir(Node2* node) {
  return ToMemNode(node)->rmdir();
}

void MemMount::set_len(Node2* node, size_t len) {
  return ToMemNode(node)->set_len(len);
}

void MemMount::ReallocData(Node2* node, int len) {
  return ToMemNode(node)->ReallocData(len);
}

void MemMount::raw_stat(Node2* node, struct stat *buf) {
  return ToMemNode(node)->raw_stat(buf);
}

void MemMount::DecrementUseCount(Node2* node) {
  return ToMemNode(node)->DecrementUseCount();
}

int MemMount::Getdents(Node2* node2, off_t offset,
                       struct dirent *dir, unsigned int count) {
  AcquireLock();
  MemNode* node = reinterpret_cast<MemNode2*>(node2)->node();
  // Check that it is a directory.
  if (!(node->is_dir())) {
    errno = ENOTDIR;
    ReleaseLock();
    return -1;
  }

  std::list<MemNode*>* children = node->children();
  int pos;
  int bytes_read;

  pos = 0;
  bytes_read = 0;
  assert(children);
  // Skip to the child at the current offset.
  std::list<MemNode *>::iterator children_it;

  for (children_it = children->begin();
       children_it != children->end() &&
           bytes_read + sizeof(struct dirent) <= count;
       ++children_it) {
    memset(dir, 0, sizeof(struct dirent));
    // We want d_ino to be non-zero because readdir()
    // will return null if d_ino is zero.
    dir->d_ino = 0x60061E;
    dir->d_off = sizeof(struct dirent);
    dir->d_reclen = sizeof(struct dirent);
    strncpy(dir->d_name, (*children_it)->name().c_str(), sizeof(dir->d_name));
    ++dir;
    ++pos;
    bytes_read += sizeof(struct dirent);
  }
  ReleaseLock();
  return bytes_read;
}

std::string MemMount::name(Node2* node) {
  return ToMemNode(node)->name();
}

char *MemMount::data(Node2* node) {
  return ToMemNode(node)->data();
}
