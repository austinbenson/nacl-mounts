/*
 * Copyright (c) 2011 The Native Client Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that be
 * found in the LICENSE file.
 */
#ifndef PACKAGES_SCRIPTS_FILESYS_APPENGINE_APPENGINEURLLOADER_H_
#define PACKAGES_SCRIPTS_FILESYS_APPENGINE_APPENGINEURLLOADER_H_

#include <algorithm>
#include <list>
#include <string>
#include <vector>
#include <semaphore.h>
#include <string.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/completion_callback.h>
#include <ppapi/cpp/url_loader.h>
#include <ppapi/cpp/url_request_info.h>
#include <ppapi/cpp/url_response_info.h>
#include <ppapi/cpp/var.h>
#include <ppapi/c/pp_errors.h>
#include <stdio.h>


#define BOUNDARY_STRING "--------------4789341488943"
#define BOUNDARY_STRING_HEADER BOUNDARY_STRING "\n"
#define BOUNDARY_STRING_LF "--" BOUNDARY_STRING "\n"
#define BOUNDARY_STRING_END_LF BOUNDARY_STRING "--\n"

typedef std::pair< std::string, const std::vector<char>* > KeyValue;
typedef std::list<KeyValue> KeyValueList;

namespace pp {
  class AppEnginePost {
  public:
    AppEnginePost(Instance* instance, sem_t done, int *result)
      : factory_(this),
      loader_(instance),
      did_open_(false),
      pepper_instance_(instance),
      request(instance),
      done_(done) {
    }

    ~AppEnginePost() { fprintf(stderr, "In Post destructor\n"); }

    void Post(const std::string& url, const KeyValueList& fields);

    const std::vector<char>& get_data(void) const {
      return data_;
    }

  private:
    //URLRequestInfo MakeRequest(const std::string& url, const KeyValueList& fields);
    void MakeRequest(const std::string& url, const KeyValueList& fields);
    void OnOpen(int32_t result);
    void OnRead(int32_t result);
    void ReadMore();
    void ProcessResponseInfo(const URLResponseInfo& response_info);

    void ProcessBytes(const char* bytes, int32_t length);

    pp::CompletionCallbackFactory<AppEnginePost> factory_;
    pp::URLLoader loader_;
    std::vector<char> data_;
    char buf_[4096];
    bool did_open_;
    pp::Instance* pepper_instance_;
    sem_t done_;
    int *result_;
    pp::URLRequestInfo request;
  };


  class AppEngineUrlRequest {
  public:
    AppEngineUrlRequest(Instance* instance, const std::string& base_url)
      : pepper_instance_(instance),
      base_url_(base_url) {
    }
      
    int Read(const std::string& path, std::vector<char>& dst);
    
    int Write(const std::string& path, const std::vector<char>& data);
    
    int List(const std::string& path, std::vector<std::string>& dst);
    
    int Remove(const std::string& path);

  private:
    pp::Instance* pepper_instance_;
    std::string base_url_;
  };
}

#endif  // PACKAGES_SCRIPTS_FILESYS_APPENGINE_APPENGINEURLLOADER_H_
