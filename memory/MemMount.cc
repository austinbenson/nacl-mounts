/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#include "MemMount.h"

MemMount::MemMount() {
  pthread_mutex_init(&lock_, NULL);
  root_ = new MemNode();
  root_->set_is_dir(true);
  root_->set_name("/");
}

MemMount::~MemMount() {
  delete root_;
}

Node *MemMount::MountOpen(std::string path, int oflag, ...) {
  Node *node;
  Node *parent;

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
  return node;
}

int MemMount::mkdir(std::string path, mode_t mode) {
  Node *node;
  Node *nnode;

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

Node *MemMount::GetNode(std::string path) {
  return GetMemNode(path);
}

Node *MemMount::GetParentNode(std::string path) {
  return GetParentMemNode(path);
}

Node *MemMount::GetMemNode(std::string path) {
  Node *node;
  std::list<std::string> path_components;
  std::list<Node *>::iterator it;
  std::list<Node *> *children;

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
    for (it = children->begin(); it != children->end(); ++it)
      if (((*it)->name()).compare(*path_it) == 0) break;
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

Node *MemMount::GetParentMemNode(std::string path) {
  return GetMemNode(path + "/..");
}
