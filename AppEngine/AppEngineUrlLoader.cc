#include "AppEngineUrlLoader.h"

#define BOUNDARY_STRING "4789341488943"
#define BOUNDARY_STRING_HEADER BOUNDARY_STRING "\n"
#define BOUNDARY_STRING_SEP "--" BOUNDARY_STRING "\r\n"
#define BOUNDARY_STRING_END "--" BOUNDARY_STRING "--\r\n\r\n"

void AppEnginePost::Run(MainThreadJobEntry *e) {
  fprintf(stderr, "In AppEnginePost::Run()\n");
  job_entry_ = e;
  loader_ = new pp::URLLoader(job_entry_->pepper_instance);
  factory_ = new pp::CompletionCallbackFactory<AppEnginePost>(this);
  pp::CompletionCallback cc = factory_->NewCallback(&AppEnginePost::OnOpen);
  fprintf(stderr, "About to call open on loader, url is %s\n", url_.c_str());
  int32_t rv = loader_->Open(MakeRequest(url_, *fields_), cc);
  fprintf(stderr, "rv from open is %d\n", rv);
  if (rv != PP_OK_COMPLETIONPENDING) {
    fprintf(stderr, "not PP_OK_COMPLETIONPENDING\n");
    cc.Run(rv);
  }
  fprintf(stderr, "Leaving Post()\n");
}

pp::URLRequestInfo AppEnginePost::MakeRequest(const std::string& url, const KeyValueList& fields) {
  fprintf(stderr, "About to make a request\n");
  pp::URLRequestInfo request(job_entry_->pepper_instance);
  fprintf(stderr, "url is: %s\n", url.c_str());
  request.SetURL(url);
  request.SetMethod("POST");
  request.SetFollowRedirects(true);
  request.SetAllowCredentials(true);
  request.SetHeaders("Content-Type: multipart/form-data; boundary=" BOUNDARY_STRING_HEADER);
  KeyValueList::const_iterator it;
  for (it = fields.begin(); it != fields.end(); ++it) {
    request.AppendDataToBody(BOUNDARY_STRING_SEP, sizeof(BOUNDARY_STRING_SEP));
    std::string line = "Content-Disposition: form-data; name=\"" + it->first + "\"\r\n\r\n";
    request.AppendDataToBody(line.c_str(), line.size());
    request.AppendDataToBody(&it->second[0], it->second->size());
    request.AppendDataToBody("\r\n", 2);
  }
  request.AppendDataToBody(BOUNDARY_STRING_END, sizeof(BOUNDARY_STRING_END));
  fprintf(stderr, "Returning request\n");
  return request;
}

void AppEnginePost::OnOpen(int32_t result) {
  fprintf(stderr, "Entering OnOpen()\n");
  if (result >= 0) {
    ReadMore();
  }
}

void AppEnginePost::OnRead(int32_t result) {
  fprintf(stderr, "Entering OnRead(), result=%d\n", result);
  if (result > 0) {
    ProcessBytes(buf_, result);
    ReadMore();
  } else if (result == PP_OK && !did_open_) {
    // Headers are available, and we can start reading the body.
    did_open_ = true;
    ProcessResponseInfo(loader_->GetResponseInfo());
    ReadMore();
  } else {
    // Done reading (possibly with an error given by 'result').
    MainThreadRunner::StuffResult(job_entry_, 1);
    delete this;
  }
}

void AppEnginePost::ReadMore() {
  pp::CompletionCallback cc = factory_->NewCallback(&AppEnginePost::OnRead);
  int32_t rv = loader_->ReadResponseBody(buf_, sizeof(buf_), cc);
  if (rv != PP_OK_COMPLETIONPENDING) {
    fprintf(stderr, "not PP_OK_COMPLETIONPENDING\n");
    cc.Run(rv);
  }
}

////////////

void AppEnginePost::ProcessResponseInfo(const pp::URLResponseInfo& response_info) {
  // Read response headers, etc.
}

void AppEnginePost::ProcessBytes(const char* bytes, int32_t length) {
  assert(length >= 0);
  std::vector<char>::size_type pos = dst_->size();
  dst_->resize(pos + length);
  memcpy(&(*dst_)[pos], bytes, length);
}

int AppEngineUrlRequest::Read(const std::string& path, std::vector<char>& dst) {
  fprintf(stderr, "In AppEngineUrlLoader::read\n");
  KeyValueList fields;
  int raw_result;

  std::vector<char> filename_vec(path.begin(), path.end());
  fields.push_back(KeyValue("filename", &filename_vec));
  fprintf(stderr, "fields initiatlized\n");
 
  raw_result = runner_->RunJob(new AppEnginePost(base_url_ + "/read", fields, &dst));
  if (!raw_result) return -1;
  fprintf(stderr, "getting data\n");
  if (dst.size() < 1) return -1;
  if (dst[0] != '1') return -1;
  return 0;
}

int AppEngineUrlRequest::Write(const std::string& path, const std::vector<char>& data) {
  KeyValueList fields;
  int raw_result;

  std::vector<char>::const_iterator it;
  fprintf(stderr, "###########\n");
  for (it = data.begin(); it != data.end(); ++it) {
    fprintf(stderr, "%c, ", *it);
  }
  fprintf(stderr, "\n###########\n");

  std::vector<char> filename_vec(path.begin(), path.end());
  fields.push_back(KeyValue("filename", &filename_vec));
  fields.push_back(KeyValue("data", &data));

  std::vector<char> dst;
  raw_result = runner_->RunJob(new AppEnginePost(base_url_ + "/write", fields, &dst));
  if (!raw_result) return -1;
  return dst.size() == 1 && dst[0] == '1' ? 0 : -1;
}

int AppEngineUrlRequest::List(const std::string& path, std::vector<char>& dst) {
  fprintf(stderr, "In List()\n");
  KeyValueList fields;
  int raw_result;

  std::vector<char> filename_vec(path.begin(), path.end());
  fields.push_back(KeyValue("prefix", &filename_vec));

  raw_result = runner_->RunJob(new AppEnginePost(base_url_ + "/list", fields, &dst));
  if (!raw_result) return -1;
  return 0;
}

int AppEngineUrlRequest::Remove(const std::string& path) {
  fprintf(stderr, "In Remove()\n");
  KeyValueList fields;
  int raw_result;

  std::vector<char> filename_vec(path.begin(), path.end());
  fields.push_back(KeyValue("filename", &filename_vec));

  std::vector<char> data;
  raw_result = runner_->RunJob(new AppEnginePost(base_url_ + "/remove", fields, &data));
  if (!raw_result) return -1;
  return data.size() == 1 && data[0] == '1' ? 0 : -1;
}

