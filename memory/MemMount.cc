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
  slots_.Alloc();
  root_ = slots_.At(0);
  root_->set_mount(this);
  root_->set_is_dir(true);
  root_->set_name("/");
}

MemNode* MemMount::ToMemNode(Node2 *node) {
  if (node == NULL) {
    return NULL;
  }
  return reinterpret_cast<MemNode2*>(node)->node();
}

Node2 *MemMount::Creat(std::string path, mode_t mode) {
  MemNode *child;
  MemNode *parent;

  // Get the directory its in.
  int parent_slot = GetParentSlot(path);
  if (parent_slot == -1) {
    errno = ENOTDIR;
    return NULL;
  }
  parent = GetParentMemNode(path);
  // It must be a directory.
  if (!(parent->is_dir())) {
    errno = ENOTDIR;
    return NULL;
  }
  // See if file exists.
  child = GetMemNode(path);
  if (child) {
    errno = EEXIST;
    return NULL;
  }

  // Create it.
  int slot = slots_.Alloc();
  child = slots_.At(slot);
  child->set_is_dir(false);
  child->set_mount(this);
  std::string p(path);
  PathHandle ph(p);
  child->set_name(ph.Last());
  child->set_parent(parent_slot);
  parent->AddChild(slot);
  child->IncrementUseCount();

  return new MemNode2(&slots_, slot);
}

int MemMount::mkdir(std::string path, mode_t mode) {
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
  child->set_mount(this);
  child->set_is_dir(true);
  PathHandle ph(path);
  child->set_name(ph.Last());
  child->set_parent(parent_slot);
  parent->AddChild(slot);

  return 0;
}

Node2 *MemMount::GetNode(std::string path) {
  int slot = GetSlot(path);
  if (slot == -1) {
    return NULL;
  }
  return new MemNode2(&slots_, slot);
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

int MemMount::chmod(Node2 *node, mode_t mode) {
  return ToMemNode(node)->chmod(mode);
}

int MemMount::stat(Node2* node, struct stat *buf) {
  return ToMemNode(node)->stat(buf);
}

int MemMount::remove(Node2* node2) {
  int slot = reinterpret_cast<MemNode2*>(node2)->slot();
  if (slot == 0) {
    // Can't delete root.
    errno = EBUSY;
    return -1;
  }
  MemNode* node = slots_.At(slot);
  if (node == NULL) {
    errno = ENOENT;
    return -1;
  }
  // Check that it's a file.
  if (node->is_dir()) {
    errno = EISDIR;
    return -1;
  }
  // Check that it's not busy.
  if (node->use_count() > 0) {
    errno = EBUSY;
    return -1;
  }
  // Get the node's parent.
  slots_.At(node->parent())->RemoveChild(slot);
  slots_.Free(slot);

  return 0;
}

int MemMount::rmdir(Node2* node2) {
  int slot = reinterpret_cast<MemNode2*>(node2)->slot();
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

void MemMount::DecrementUseCount(Node2* node) {
  return ToMemNode(node)->DecrementUseCount();
}

int MemMount::Getdents(Node2* node2, off_t offset,
                       struct dirent *dir, unsigned int count) {
  MemNode* node = reinterpret_cast<MemNode2*>(node2)->node();
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

ssize_t MemMount::Read(Node2* node, off_t offset, void *buf, size_t count) {
  MemNode* mnode = ToMemNode(node);
  // Limit to the end of the file.
  size_t len = count;
  if (len > mnode->len() - offset) {
    len = mnode->len() - offset;
  }

  // Do the read.
  memcpy(buf, mnode->data() + offset, len);
  return len;
}

ssize_t MemMount::Write(Node2* node, off_t offset, const void *buf, size_t count) {
  MemNode* mnode = ToMemNode(node);

  size_t len;
  // Grow the file if needed.
  if (offset + static_cast<off_t>(count) > mnode->capacity()) {
    len = offset + count;
    size_t next = (mnode->capacity() + 1) * 2;
    if (next > len) {
      len = next;
    }
    mnode->ReallocData(len);
  }
  // Pad any gap with zeros.
  if (offset > static_cast<off_t>(mnode->len())) {
    memset(mnode->data()+len, 0, offset);
  }

  // Write out the block.
  memcpy(mnode->data() + offset, buf, count);
  offset += count;
  if (offset > static_cast<off_t>(mnode->len())) {
    mnode->set_len(offset);
  }
  return count;
}
