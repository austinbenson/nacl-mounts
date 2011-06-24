/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */

#ifndef PACKAGES_SCRIPTS_FILESYS_BASE_PATHHANDLE_H_
#define PACKAGES_SCRIPTS_FILESYS_BASE_PATHHANDLE_H_

#include <list>
#include <string>

// PathHandle is a class for manipulating file paths.  This is
// a utility class for the nacl-mounts pluggable file I/O.
// Fundamentally, PathHandle assumes directories are split
// by the character '/' in a path name.  In addition,
// PathHandle assumes that the character '/' is not part of
// a directory name.
class PathHandle {
 public:
  PathHandle() {}
  // This constructor splits path by '/' as a starting
  // point for this PathHandle.  If the path begins
  // with the character '/', is_absolute_ is set to true.
  explicit PathHandle(std::string path);
  ~PathHandle() {}

  // FormulateRawPath() returns a string representation
  // of this PathHandle.  If no path components have
  // been added to this class, then an empty string
  // is returned.  No path collapsing is performed
  // by this method.  The returned path does NOT
  // end with '/'.
  std::string FormulateRawPath(void);

  // FormulatePath() is like FormulateRawPath() except
  // that this method first simplifies the path by
  // placing it in canonical form.  This is done in place,
  // and you cannot retrieve an old, unsimplified path
  // after calling FormulatePath().  The returned
  // path does NOT end with '/'.
  std::string FormulatePath(void);

  // AppendPath() puts path at the end of the current path
  // represented by this PathHandle.
  void AppendPath(std::string path);

  // Last() returns the last path component in the path
  // represented by this PathHandle.  This can represent
  // the file or directory name of the path.
  std::string Last(void) { return path_.back(); }

  // MergePaths will take two paths and create a list of strings
  // of the components of these paths.  Strings will be split
  // by '/' in order to merge into a a single list.
  static std::list<std::string> MergePaths(std::list<std::string> path1,
                                    std::list<std::string> path2);

  static std::list<std::string> MergePaths(std::list<std::string> path1,
                                    std::string path2);

  static std::list<std::string> MergePaths(std::string path1,
                                    std::list<std::string> path2);

  static std::list<std::string> MergePaths(std::string path1,
                                    std::string path2);

  // Split() divides a string into pieces separated by c.  Each occurrence
  // of c is not included in any member of the returned list.
  static std::list<std::string> Split(std::string path, char c);

  // IsDot() checks if s is "."
  static bool IsDot(std::string s) { return s.length() == 1 && s[0] == '.'; }

  // IsDotDot() checks if s is ".."
  static bool IsDotDot(std::string s) { return s.length() == 2 &&
                                        s[0] == '.' && s[1] == '.'; }

  // IsSlash() checks if s is "/"
  static bool IsSlash(std::string s) { return s.length() == 1 && s[0] == '/'; }

  // If p ends with the char '/', AppendSlash() returns a copy of
  // the string p.  If p does not end with the char '/',
  // AppendSlash() returns a copy of p with the char '/' appeneded
  // at the end.
  static std::string AppendSlash(std::string p);

  // SetPath() resets the internal path representation to match
  // the string path.  This removes all previous internal path state
  // and sets is_absolute_ according to path.
  void SetPath(std::string path);

  void set_is_absolute(bool is_absolute) { is_absolute_ = is_absolute; }
  bool is_absolute() { return is_absolute_; }
  std::list<std::string> path(void) { return path_; }

 private:
  // internal representation of the path
  std::list<std::string> path_;
  // whether or not this path is considered to be absolute
  bool is_absolute_;

  // PathCollapse() puts path_ into its canonical form. This is
  // done in place.
  void PathCollapse(void);
};

#endif  // PACKAGES_SCRIPTS_FILESYS_BASE_PATHHANDLE_H_



