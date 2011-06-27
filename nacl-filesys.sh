#!/bin/bash
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that be
# found in the LICENSE file.
#

# nacl-filesys.sh
#
# usage:  ./nacl-filesys.sh
#
# this script builds filesys
#

readonly PACKAGE_NAME=filesys

source ../common.sh

CustomBuildStep() {
  Banner "Building ${PACKAGE_NAME}"
  export PACKAGE_DIR="${NACL_PACKAGES_REPOSITORY}/${PACKAGE_NAME}"
  MakeDir ${PACKAGE_DIR}
  ChangeDir ${PACKAGE_DIR}
  ${NACLCC} -c ${START_DIR}/base/MountManager.cc -o MountManager.o
  ${NACLCC} -c ${START_DIR}/base/KernelProxy.cc -o KernelProxy.o
  ${NACLCC} -c ${START_DIR}/base/PathHandle.cc -o PathHandle.o
  ${NACLCC} -c ${START_DIR}/base/Entry.cc -o Entry.o
  ${NACLCC} -c ${START_DIR}/memory/MemMount.cc -o MemMount.o
  ${NACLCC} -c ${START_DIR}/memory/MemNode.cc -o MemNode.o
  ${NACLAR} rcs filesys.a \
      MountManager.o \
      KernelProxy.o \
      PathHandle.o \
      Entry.o \
      MemMount.o \
      MemNode.o \

  ${NACLRANLIB} filesys.a
}

CustomInstallStep() {
  Banner "Installing ${PACKAGE_NAME}"
  export PACKAGE_DIR="${NACL_PACKAGES_REPOSITORY}/${PACKAGE_NAME}"
  cp ${PACKAGE_DIR}/filesys.a ${NACL_SDK_USR_LIB}
  mkdir -p ${NACL_SDK_USR_LIB}/filesys
}

CustomTestBuildStep() {
  Banner "Building Tests for ${PACKAGE_NAME}/base"
  ${NACLCC} -c ${START_DIR}/base/MountManager.cc -o MountManager.o
  ${NACLCC} -c ${START_DIR}/base/Entry.cc -o Entry.o
}

CustomPackageInstall() {
  DefaultPreInstallStep
  CustomBuildStep
  CustomInstallStep
  DefaultCleanUpStep
}

CustomPackageInstall
exit 0
