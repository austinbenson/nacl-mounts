/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */


#ifndef PACKAGES_SCRIPTS_FILESYS_TESTS_MEMORY_TESTHELPCOMMON_H_
#define PACKAGES_SCRIPTS_FILESYS_TESTS_MEMORY_TESTHELPCOMMON_H_

#define CHECK(x) { \
  ASSERT_NE((void*)NULL, x); \
} while(0)

Node *CreateNode(std::string name, Node *parent,
                    MemMount *mount, bool is_dir = false) {
  Node *node = new MemNode();
  node->set_name(name);
  node->set_parent(parent);
  node->set_mount(mount);
  node->set_is_dir(is_dir);
  return node;
}

#endif  // PACKAGES_SCRIPTS_FILESYS_TESTS_MEMORY_TESTHELPCOMMON_H_
