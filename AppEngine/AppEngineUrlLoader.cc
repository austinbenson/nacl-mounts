#include "AppEngineUrlLoader.h"


void pp::AppEnginePost::Run(MainThreadJobEntry *e) {
  job_entry_ = e;
  loader_ = new pp::URLLoader(job_entry_->pepper_instance);

  fprintf(stderr, "In AppEnginePost::Post()\n");

  pp::CompletionCallback cc = factory_.NewCallback(&AppEnginePost::OnOpen);
  int32_t rv = loader_->Open(MakeRequest(url_, *fields_), cc);
  if (rv != PP_OK_COMPLETIONPENDING) {
    fprintf(stderr, "not PP_OK_COMPLETIONPENDING\n");
    cc.Run(rv);
  }
  fprintf(stderr, "Leaving Post()\n");
}

pp::URLRequestInfo pp::AppEnginePost::MakeRequest(const std::string& url, const KeyValueList& fields) {
  fprintf(stderr, "About to make a request\n");
  URLRequestInfo request(job_entry_->pepper_instance);
  fprintf(stderr, "url is: %s\n", url.c_str());
  request.SetURL(url);
  request.SetMethod("GET");
  //  request.SetFollowRedirects(true);
  //  request.SetAllowCredentials(true);
  request.SetHeaders("Content-Type: multipart/form-data; boundary=" BOUNDARY_STRING_HEADER);
  KeyValueList::const_iterator it;
  for (it = fields.begin(); it != fields.end(); ++it) {
    request.AppendDataToBody(BOUNDARY_STRING_LF, sizeof(BOUNDARY_STRING_LF));
    std::string line = "Content-Disposition: form-data; name=\"" + it->first + "\"\n\n";
    request.AppendDataToBody(line.c_str(), line.size());
    request.AppendDataToBody(&it->second[0], it->second->size());
  }
  request.AppendDataToBody(BOUNDARY_STRING_END_LF, sizeof(BOUNDARY_STRING_END_LF));
  fprintf(stderr, "Returning request\n");
  return request;
}

void pp::AppEnginePost::OnOpen(int32_t result) {
  fprintf(stderr, "Entering OnOpen()\n");
  if (result >= 0) {
    ReadMore();
  }
}

void pp::AppEnginePost::OnRead(int32_t result) {
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

void pp::AppEnginePost::ReadMore() {
  pp::CompletionCallback cc = factory_.NewCallback(&AppEnginePost::OnRead);
  int32_t rv = loader_->ReadResponseBody(buf_, sizeof(buf_), cc);
  if (rv != PP_OK_COMPLETIONPENDING) {
    fprintf(stderr, "not PP_OK_COMPLETIONPENDING\n");
    cc.Run(rv);
  }
}

void pp::AppEnginePost::ProcessResponseInfo(const URLResponseInfo& response_info) {
  // Read response headers, etc.
}

void pp::AppEnginePost::ProcessBytes(const char* bytes, int32_t length) {
  assert(length >= 0);
  std::vector<char>::size_type pos = dst_->size();
  dst_->resize(pos + length);
  memcpy(&(*dst_)[pos], bytes, length);
}

int pp::AppEngineUrlRequest::Read(const std::string& path, std::vector<char>& dst) {
  fprintf(stderr, "In AppEngineUrlLoader::read\n");
  KeyValueList fields;
  int raw_result;

  std::vector<char> filename_vec(path.begin(), path.end());
  fields.push_back(KeyValue("filename", &filename_vec));
  fprintf(stderr, "fields initiatlized\n");
 
  raw_result = runner_->RunJob(new AppEnginePost(base_url_ + "/read", fields, &dst));
  fprintf(stderr, "post initialized\n");
  if (!raw_result) return -1;
  fprintf(stderr, "getting data\n");
  if (dst.size() < 1) return -1;
  if (dst[0] != '1') return -1;
  return 0;
}

int pp::AppEngineUrlRequest::Write(const std::string& path, const std::vector<char>& data) {
  KeyValueList fields;
  int raw_result;

  std::vector<char> filename_vec(path.begin(), path.end());
  fields.push_back(KeyValue("filename", &filename_vec));
  fields.push_back(KeyValue("data", &data));

  std::vector<char> dst;
  raw_result = runner_->RunJob(new AppEnginePost(base_url_ + "/write", fields, &dst));
  if (!raw_result) return -1;
  return dst.size() == 1 && dst[0] == '1' ? 0 : -1;
}

int pp::AppEngineUrlRequest::List(const std::string& path, std::vector<std::string>& dst) {
  KeyValueList fields;
  int raw_result;

  std::vector<char> filename_vec(path.begin(), path.end());
  fields.push_back(KeyValue("prefix", &filename_vec));

  std::vector<char> data;
  raw_result = runner_->RunJob(new AppEnginePost(base_url_ + "/list", fields, &data));
  if (!raw_result) return -1;
  dst.clear();
  std::vector<char>::const_iterator i = data.begin();
  std::vector<char>::const_iterator end = data.end();
  while (i != data.end()) {
    std::vector<char>::const_iterator next = std::find(i, end, '\n');
    if (next == data.end()) break;
    dst.push_back(std::string(i, next - 1));
    i = next;
  }
  return 0;
}

int pp::AppEngineUrlRequest::Remove(const std::string& path) {
  KeyValueList fields;
  int raw_result;

  std::vector<char> filename_vec(path.begin(), path.end());
  fields.push_back(KeyValue("filename", &filename_vec));

  std::vector<char> data;
  raw_result = runner_->RunJob(new AppEnginePost(base_url_ + "/remove", fields, &data));

  if (!raw_result) return 0;
  return data.size() == 1 && data[0] == '1' ? 0 : -1;
}

