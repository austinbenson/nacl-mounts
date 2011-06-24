/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */

#include "../../base/PathHandle.h"
#include "../common/common.h"

TEST(PathHandleTest, SmallPathParsingChecks) {
  std::string s1(".");
  std::string s2("..");
  std::string s3("/");
  std::string s4("//");
  std::string s5("");
  std::string s6("hello, world!");
  
  EXPECT_EQ(true, PathHandle::IsDot(s1));
  EXPECT_EQ(false, PathHandle::IsDot(s2));
  EXPECT_EQ(true, PathHandle::IsDotDot(s2));
  EXPECT_EQ(false, PathHandle::IsDotDot(s1));
  EXPECT_EQ(true, PathHandle::IsSlash(s3));
  EXPECT_EQ(false, PathHandle::IsSlash(s4));

  EXPECT_STREQ(s3.c_str(), PathHandle::AppendSlash(s3).c_str());
  EXPECT_STREQ(s4.c_str(), PathHandle::AppendSlash(s4).c_str());
  EXPECT_STREQ(s3.c_str(), PathHandle::AppendSlash(s5).c_str());
  EXPECT_STREQ("hello, world!/", PathHandle::AppendSlash(s6).c_str());
}

TEST(PathHandleTest, Split) {
  std::list<std::string> splitter;
  std::list<std::string>::iterator it;

  std::string s1("/simple/splitter/test");
  splitter = PathHandle::Split(s1, '/');
  EXPECT_EQ(3, static_cast<int>(splitter.size()));
  it = splitter.begin();
  EXPECT_STREQ("simple", it->c_str());
  ++it;
  EXPECT_STREQ("splitter", it->c_str());
  ++it;
  EXPECT_STREQ("test", it->c_str());

  std::string s2("///simple//splitter///test/");
  splitter = PathHandle::Split(s2, '/');
  EXPECT_EQ(3, static_cast<int>(splitter.size()));
  it = splitter.begin();
  EXPECT_STREQ("simple", it->c_str());
  ++it;
  EXPECT_STREQ("splitter", it->c_str());
  ++it;
  EXPECT_STREQ("test", it->c_str());

  std::string s3("/sim/ple//spli/tter/te/st/");
  splitter = PathHandle::Split(s3, '/');
  EXPECT_EQ(6, static_cast<int>(splitter.size()));
  it = splitter.begin();
  EXPECT_STREQ("sim", it->c_str());
  ++it;
  EXPECT_STREQ("ple", it->c_str());
  ++it;
  EXPECT_STREQ("spli", it->c_str());
  ++it;
  EXPECT_STREQ("tter", it->c_str());
  ++it;
  EXPECT_STREQ("te", it->c_str());
  ++it;
  EXPECT_STREQ("st", it->c_str());

  std::string s4("ha.ha.ha.ha");
  splitter = PathHandle::Split(s4, '.');
  EXPECT_EQ(4, static_cast<int>(splitter.size()));
  for (it = splitter.begin(); it != splitter.end(); ++it)
    EXPECT_STREQ("ha", it->c_str());

  std::string s5("");
  splitter = PathHandle::Split(s5, '/');
  EXPECT_EQ(0, static_cast<int>(splitter.size()));

  std::string s6("hello, world!");
  splitter = PathHandle::Split(s6, '/');
  EXPECT_EQ(1, static_cast<int>(splitter.size()));
  splitter = PathHandle::Split(s6, '!');
  EXPECT_EQ(1, static_cast<int>(splitter.size()));
  it = splitter.begin();
  EXPECT_STREQ("hello, world", it->c_str());
  
  std::string s7("!hello, world");
  splitter = PathHandle::Split(s7, '!');
  EXPECT_EQ(1, static_cast<int>(splitter.size()));
  it = splitter.begin();
  EXPECT_STREQ("hello, world", it->c_str());
 
  std::string s8("!hello, world!");
  splitter = PathHandle::Split(s8, '!');
  EXPECT_EQ(1, static_cast<int>(splitter.size()));
  it = splitter.begin();
  EXPECT_STREQ("hello, world", it->c_str());
}

// Helper function for PathHandleTest.MergePaths test
void MergePathsChecker (std::list<std::string> path, int sequence) {
 std::list<std::string>::iterator it;
 it = path.begin();

 switch (sequence) {
   case 123456: {
     EXPECT_STREQ("1", it->c_str());
     ++it;
     EXPECT_STREQ("2", it->c_str());
     ++it;
     EXPECT_STREQ("3", it->c_str());
     ++it;
     EXPECT_STREQ("4", it->c_str());
     ++it;
     EXPECT_STREQ("5", it->c_str());
     ++it;
     EXPECT_STREQ("6", it->c_str());
     break;
   }
   case 456123: {
     EXPECT_STREQ("4", it->c_str());
     ++it;
     EXPECT_STREQ("5", it->c_str());
     ++it;
     EXPECT_STREQ("6", it->c_str());
     ++it;
     EXPECT_STREQ("1", it->c_str());
     ++it;
     EXPECT_STREQ("2", it->c_str());
     ++it;
     EXPECT_STREQ("3", it->c_str());
     break;
   }
   case 123: {
     EXPECT_STREQ("1", it->c_str());
     ++it;
     EXPECT_STREQ("2", it->c_str());
     ++it;
     EXPECT_STREQ("3", it->c_str());
     break;
   }
   case 456: {
     EXPECT_STREQ("4", it->c_str());
     ++it;
     EXPECT_STREQ("5", it->c_str());
     ++it;
     EXPECT_STREQ("6", it->c_str());
     break;
   }
 }
}

TEST(PathHandleTest, MergePaths) {
  std::list<std::string> p1, p2, p3, p4;
  std::list<std::string>::iterator it;
  std::string s1("1/2/3/");
  std::string s2("//4/5/6");
  std::string s3("");
  p1.push_back("1");
  p1.push_back("2");
  p1.push_back("3");
  p2.push_back("4");
  p2.push_back("5");
  p2.push_back("6");

  // Test cases for two lists
  p3 = PathHandle::MergePaths(p1, p2);
  EXPECT_EQ(6, static_cast<int>(p3.size()));
  it = p3.begin();
  MergePathsChecker(p3, 123456);

  p3 = PathHandle::MergePaths(p2, p1);
  EXPECT_EQ(6, static_cast<int>(p3.size()));
  it = p3.begin();
  MergePathsChecker(p3, 456123);

  p3 = PathHandle::MergePaths(p3, p4);
  EXPECT_EQ(6, static_cast<int>(p3.size()));
  MergePathsChecker(p3, 456123);

  p3 = PathHandle::MergePaths(p4, p3);
  EXPECT_EQ(6, static_cast<int>(p3.size()));
  MergePathsChecker(p3, 456123);

  p4 = PathHandle::MergePaths(p4, p4);
  EXPECT_EQ(0, static_cast<int>(p4.size()));

  // Test cases for one list, one string
  p3 = PathHandle::MergePaths(p1, s2);
  EXPECT_EQ(6, static_cast<int>(p3.size()));
  MergePathsChecker(p3, 123456);

  p3 = PathHandle::MergePaths(s2, p1);
  EXPECT_EQ(6, static_cast<int>(p3.size()));
  MergePathsChecker(p3, 456123);

  p3 = PathHandle::MergePaths(p3, s3);
  EXPECT_EQ(6, static_cast<int>(p3.size()));
  MergePathsChecker(p3, 456123);

  p3 = PathHandle::MergePaths(s3, p3);
  EXPECT_EQ(6, static_cast<int>(p3.size()));
  MergePathsChecker(p3, 456123);
  
  // Test cases for two strings
  p3 = PathHandle::MergePaths(s1, s2);
  EXPECT_EQ(6, static_cast<int>(p3.size()));
  MergePathsChecker(p3, 123456);

  p3 = PathHandle::MergePaths(s2, s1);
  EXPECT_EQ(6, static_cast<int>(p3.size()));
  MergePathsChecker(p3, 456123);

  p3 = PathHandle::MergePaths(s3, s2);
  EXPECT_EQ(3, static_cast<int>(p3.size()));
  MergePathsChecker(p3, 456);

  p3 = PathHandle::MergePaths(s1, s3);
  EXPECT_EQ(3, static_cast<int>(p3.size()));
  MergePathsChecker(p3, 123);

  p4 = PathHandle::MergePaths(s3, s3);
  EXPECT_EQ(0, static_cast<int>(p4.size()));
}

TEST(PathHandleTest, AppendPathAndFormulateRawPath) {
  PathHandle *ph1, *ph2;
  ph1 = new PathHandle("/usr");
  ph1->AppendPath("/local/share//");
  ph1->AppendPath("/");
  ph1->AppendPath("/..");
  ph1->AppendPath("/..");
  ph1->AppendPath("/./");
  ph1->AppendPath("");
  ph1->AppendPath("");
  EXPECT_STREQ("/usr/local/share/../../.", ph1->FormulateRawPath().c_str());
  EXPECT_STREQ("/usr/local/share/../../.", ph1->FormulateRawPath().c_str());
  EXPECT_STREQ("/usr", ph1->FormulatePath().c_str());
  ph1->AppendPath("local/bin/..");
  EXPECT_STREQ("/usr/local/bin/..", ph1->FormulateRawPath().c_str());
  ph1->AppendPath(".././../../../../../");
  EXPECT_STREQ("/", ph1->FormulatePath().c_str());

  ph2 = new PathHandle("");
  ph2->set_is_absolute(true);
  EXPECT_STREQ("/", ph2->FormulateRawPath().c_str());
  ph2->AppendPath("////////////");
  EXPECT_STREQ("/", ph2->FormulateRawPath().c_str());

  ph2->SetPath("");
  EXPECT_STREQ("", ph2->FormulateRawPath().c_str());

  delete ph1;
  delete ph2;
}

TEST(PathHandleTest, FormulatePath) {
  PathHandle *ph1, *ph2, *ph3, *ph4;
  ph1 = new PathHandle("/usr/local/hi/there");
  EXPECT_STREQ("/usr/local/hi/there", ph1->FormulatePath().c_str());
  ph1->AppendPath("..");
  EXPECT_STREQ("/usr/local/hi", ph1->FormulatePath().c_str());
  ph1->AppendPath(".././././hi/there/../.././././");
  EXPECT_STREQ("/usr/local", ph1->FormulatePath().c_str());
  ph1->AppendPath("../../../../../../../../././../");
  EXPECT_STREQ("/", ph1->FormulatePath().c_str());
  ph1->AppendPath("usr/lib/../bin/.././etc/../local/../share");
  EXPECT_STREQ("/usr/share", ph1->FormulatePath().c_str());
  delete ph1;

  ph2= new PathHandle("./");
  EXPECT_STREQ("", ph2->FormulatePath().c_str());
  ph2->set_is_absolute(true);
  EXPECT_STREQ("/", ph2->FormulatePath().c_str());
  ph2->AppendPath("");
  EXPECT_STREQ("/", ph2->FormulatePath().c_str());
  ph2->AppendPath("USR/local/SHARE");
  EXPECT_STREQ("/USR/local/SHARE", ph2->FormulatePath().c_str());
  ph2->AppendPath("/////////////////////////////////////////////");
  EXPECT_STREQ("/USR/local/SHARE", ph2->FormulatePath().c_str());
  ph3 = new PathHandle("..");
  EXPECT_STREQ("", ph3->FormulatePath().c_str());
  ph3->set_is_absolute(true);
  EXPECT_STREQ("/", ph3->FormulatePath().c_str());
  ph4 = new PathHandle("/node1/node3/../../node1/./");
  EXPECT_STREQ("/node1", ph4->FormulatePath().c_str());
  ph4->AppendPath("node4/../../node1/./node5");
  EXPECT_STREQ("/node1/node5", ph4->FormulatePath().c_str());

  delete ph2;
  delete ph3;
  delete ph4;
}

