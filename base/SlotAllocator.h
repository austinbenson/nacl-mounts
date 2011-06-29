/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */

#ifndef PACKAGES_SCRIPTS_FILESYS_BASE_SLOTALLOCATOR_H_
#define PACKAGES_SCRIPTS_FILESYS_BASE_SLOTALLOCATOR_H_

#include <algorithm>
#include <vector>

template <class T>
class SlotAllocator {
 public:
  SlotAllocator() {}

  virtual ~SlotAllocator() {
    for (uint32_t i = 0; i < slots_.size(); i++) {
      if (slots_[i] != NULL) {
        delete slots_[i];
        slots_[i] = NULL;
      }
    }
  }

  int Alloc() {
    if (heap_.size() == 0) {
      slots_.push_back(new T);
      return slots_.size()-1;
    }
    int index = heap_.front();
    std::pop_heap(heap_.begin(), heap_.end(), desc);
    heap_.pop_back();
    slots_[index] = new T;
    return index;
  }

  void Free(int slot) {
    if (slots_[slot] == NULL) {
      return;
    }
    delete slots_[slot];
    slots_[slot] = NULL;
    heap_.push_back(slot);
    std::push_heap(heap_.begin(), heap_.end(), desc);
  }
  T* At(int slot) {
    if (slot < 0 || static_cast<uint32_t>(slot) >= slots_.size()) {
      return NULL;
    }
    return slots_[slot];
  }

 private:
  std::vector<T*> slots_;
  std::vector<int> heap_;

  static bool desc(int i, int j) { return i < j; }

  // TODO(krasin): enable it when the macro is put in place.
  //DISALLOW_COPY_AND_ASSIGN(SlotAllocator);
};

#endif  // PACKAGES_SCRIPTS_FILESYS_BASE_SLOTALLOCATOR_H_
