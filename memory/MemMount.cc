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
#include <stdio.h>

MemMount::MemMount() {
  slots_.Alloc();
  root_ = slots_.At(0);
  root_->slot = 0;
  root_->set_mount(this);
  root_->set_is_dir(true);
  root_->set_name("/");
}

int MemMount::Creat(const std::string& path, mode_t mode, struct stat* buf) {
  MemNode *child;
  MemNode *parent;

  // Get the directory its in.
  int parent_slot = GetParentSlot(path);
  if (parent_slot == -1) {
    errno = ENOTDIR;
    return -1;
  }
  parent = slots_.At(parent_slot);
  if (!parent) {
    errno = EINVAL;
    return -1;
  }
  // It must be a directory.
  if (!(parent->is_dir())) {
    errno = ENOTDIR;
    return -1;
  }
  // See if file exists.
  child = GetMemNode(path);
  if (child) {
    errno = EEXIST;
    return -1;
  }

  // Create it.
  int slot = slots_.Alloc();
  child = slots_.At(slot);
  child->slot = slot;
  child->set_is_dir(false);
  child->set_mount(this);
  std::string p(path);
  PathHandle ph(p);
  child->set_name(ph.Last());
  child->set_parent(parent_slot);
  parent->AddChild(slot);
  child->IncrementUseCount();

  if (!buf) {
    return 0;
  }
  return Stat(slot, buf);
}

int MemMount::Mkdir(const std::string& path, mode_t mode, struct stat* buf) {
  MemNode* parent;
  MemNode* child;

  // Make sure it doesn't already exist.
  child = GetMemNode(path);
  if (child) {
    errno = EEXIST;
    return -1;
  }
  // Get the parent node.
  int parent_slot = GetParentSlot(path);
  if (parent_slot == -1) {
    errno = ENOENT;
    return -1;
  }
  parent = slots_.At(parent_slot);

  if (!parent->is_dir()) {
    errno = ENOTDIR;
    return -1;
  }
  // Create a new node
  int slot = slots_.Alloc();
  child = slots_.At(slot);
  child->slot = slot;
  child->set_mount(this);
  child->set_is_dir(true);
  PathHandle ph(path);
  child->set_name(ph.Last());
  child->set_parent(parent_slot);
  parent->AddChild(slot);
  if (!buf) {
    return 0;
  }

  return Stat(slot, buf);
}

int MemMount::GetNode(const std::string& path, struct stat* buf) {
  int slot = GetSlot(path);
  if (slot == -1) {
    errno = ENOENT;
    return -1;
  }
  if (!buf) {
    return 0;
  }
  return Stat(slot, buf);
}

MemNode *MemMount::GetParentNode(std::string path) {
  return GetParentMemNode(path);
}

MemNode *MemMount::GetMemNode(std::string path) {
  int slot = GetSlot(path);
  if (slot == -1) {
    return NULL;
  }
  return slots_.At(GetSlot(path));
}

int MemMount::GetSlot(std::string path) {
  int slot;
  std::list<std::string> path_components;
  std::list<int>::iterator it;
  std::list<int> *children;

  // Get in canonical form.
  if (path.length() == 0) {
    return -1;
  }
  // Check if it is an absolute path
  PathHandle ph(path);
  path_components = ph.path();

  // Walk up from root.
  slot = 0;
  std::list<std::string>::iterator path_it;
  // loop through path components
  for (path_it = path_components.begin();
       path_it != path_components.end(); ++path_it) {
    // check if we are at a non-directory
    if (!(slots_.At(slot)->is_dir())) {
      errno = ENOTDIR;
      return -1;
    }
    // loop through children
    children = slots_.At(slot)->children();
    for (it = children->begin(); it != children->end(); ++it) {
      if ((slots_.At(*it)->name()).compare(*path_it) == 0) {
        break;
      }
    }
    // check for failure
    if (it == children->end()) {
      errno = ENOENT;
      return -1;
    } else {
      slot = *it;
    }
  }
  // We should now have completed the walk.
  if (slot == 0 && path_components.size() > 1) {
    return -1;
  }
  return slot;
}

MemNode *MemMount::GetParentMemNode(std::string path) {
  return GetMemNode(path + "/..");
}

int MemMount::GetParentSlot(std::string path) {
  return GetSlot(path + "/..");
}

int MemMount::Chmod(ino_t slot, mode_t mode) {
  MemNode* node = slots_.At(slot);
  if (node == NULL) {
    errno = ENOENT;
    return -1;
  }
  return node->chmod(mode);
}

int MemMount::Stat(ino_t slot, struct stat *buf) {
  MemNode* node = slots_.At(slot);
  if (node == NULL) {
    errno = ENOENT;
    return -1;
  }
  return node->stat(buf);
}

int MemMount::Unlink(const std::string& path) {
  MemNode* node = GetMemNode(path);
  if (node == NULL) {
    errno = ENOENT;
    return -1;
  }
  MemNode* parent = GetParentMemNode(path);
  if (parent == NULL) {
    // Can't delete root
    errno = EBUSY;
    return -1;
  }
  // Check that it's a file.
  if (node->is_dir()) {
    errno = EISDIR;
    return -1;
  }
  parent->RemoveChild(node->slot);
  Unref(node->slot);
  return 0;
}

int MemMount::Rmdir(ino_t slot) {
  MemNode* node = slots_.At(slot);
  if (node == NULL) {
    return ENOENT;
  }
  // Check if it's a directory.
  if (!node->is_dir()) {
    errno = ENOTDIR;
    return -1;
  }
  // Check if it's empty.
  if (node->children()->size() > 0) {
    errno = ENOTEMPTY;
    return -1;
  }
  // if this isn't the root node, remove from parent's
  // children list
  if (slot != 0) {
    slots_.At(node->parent())->RemoveChild(slot);
  }
  slots_.Free(slot);
  return 0;
}

void MemMount::Ref(ino_t slot) {
  MemNode* node = slots_.At(slot);
  if (node == NULL) {
    return;
  }
  node->IncrementUseCount();
}

void MemMount::Unref(ino_t slot) {
  MemNode* node = slots_.At(slot);
  if (node == NULL) {
    return;
  }
  if (node->is_dir()) {
    return;
  }
  node->DecrementUseCount();
  if (node->use_count() > 0) {
    return;
  }
  // If Ref/Unref misused by KernelProxy, it's possible
  // that parent will have a dangling inode to the deleted child
  // TODO(krasin): remove the possibility to misuse this API.
  slots_.Free(node->slot);
}

int MemMount::Getdents(ino_t slot, off_t offset,
                       struct dirent *dir, unsigned int count) {
  MemNode* node = slots_.At(slot);
  if (node == NULL) {
    errno = ENOTDIR;
    return -1;
  }
  // Check that it is a directory.
  if (!(node->is_dir())) {
    errno = ENOTDIR;
    return -1;
  }

  std::list<int>* children = node->children();
  int pos;
  int bytes_read;

  pos = 0;
  bytes_read = 0;
  assert(children);
  // Skip to the child at the current offset.
  std::list<int>::iterator children_it;

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
    strncpy(dir->d_name, slots_.At(*children_it)->name().c_str(), sizeof(dir->d_name));
    ++dir;
    ++pos;
    bytes_read += sizeof(struct dirent);
  }
  return bytes_read;
}

ssize_t MemMount::Read(ino_t slot, off_t offset, void *buf, size_t count) {
  MemNode* node = slots_.At(slot);
  if (node == NULL) {
    errno = ENOENT;
    return -1;
  }
  // Limit to the end of the file.
  size_t len = count;
  if (len > node->len() - offset) {
    len = node->len() - offset;
  }

  // Do the read.
  memcpy(buf, node->data() + offset, len);
  return len;
}

ssize_t MemMount::Write(ino_t slot, off_t offset, const void *buf, size_t count) {
  MemNode* node = slots_.At(slot);
  if (node == NULL) {
    errno = ENOENT;
    return -1;
  }

  size_t len = node->capacity();
  // Grow the file if needed.
  if (static_cast<size_t>(offset) + count > len) {
    len = offset + count;
    size_t next = (node->capacity() + 1) * 2;
    if (next > len) {
      len = next;
    }
    node->ReallocData(len);
  }
  // Pad any gap with zeros.
  if (offset > static_cast<off_t>(node->len())) {
    memset(node->data()+len, 0, offset);
  }

  // Write out the block.
  memcpy(node->data() + offset, buf, count);
  offset += count;
  if (offset > static_cast<off_t>(node->len())) {
    node->set_len(offset);
  }
  return count;
}
