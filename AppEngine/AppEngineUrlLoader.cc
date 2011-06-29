#include "AppEngineUrlLoader.h"

void pp::AppEnginePost::Post(const std::string& url, const KeyValueList& fields) {
  fprintf(stderr, "In AppEnginePost::Post()\n");
  pp::CompletionCallback cc = factory_.NewCallback(&AppEnginePost::OnOpen);
  //  int32_t rv = loader_.Open(MakeRequest(url, fields), cc);
  MakeRequest(url, fields);
  int32_t rv = loader_.Open(request, cc);
  if (rv != PP_OK_COMPLETIONPENDING) {
    fprintf(stderr, "not completion pending\n");
    cc.Run(rv);
  }
  fprintf(stderr, "Leaving Post()\n");
}

void pp::AppEnginePost::MakeRequest(const std::string& url, const KeyValueList& fields) {
  fprintf(stderr, "About to make a request\n");
  //  URLRequestInfo request(pepper_instance_);
  fprintf(stderr, "url is: %s\n", url.c_str());
  request.SetURL(url);
  request.SetMethod("GET");
  //  request.SetFollowRedirects(true);
  //  request.SetAllowCredentials(true);
  /*
  request.SetHeaders("Content-Type: multipart/form-data; boundary=" BOUNDARY_STRING_HEADER);
  KeyValueList::const_iterator it;
  for (it = fields.begin(); it != fields.end(); ++it) {
    request.AppendDataToBody(BOUNDARY_STRING_LF, sizeof(BOUNDARY_STRING_LF));
    std::string line = "Content-Disposition: form-data; name=\"" + it->first + "\"\n\n";
    request.AppendDataToBody(line.c_str(), line.size());
    request.AppendDataToBody(&it->second[0], it->second->size());
  }
  request.AppendDataToBody(BOUNDARY_STRING_END_LF, sizeof(BOUNDARY_STRING_END_LF));
  */
  fprintf(stderr, "Returning request\n");
  //  return request;
}

void pp::AppEnginePost::OnOpen(int32_t result) {
  fprintf(stderr, "Entering OnOpen(), result=%d\n", result);
  if (result >= 0)
    ReadMore();
}

void pp::AppEnginePost::OnRead(int32_t result) {
  fprintf(stderr, "Entering OnRead(), result=%d\n", result);
  if (result > 0) {
    ProcessBytes(buf_, result);
    ReadMore();
  } else if (result == PP_OK && !did_open_) {
    // Headers are available, and we can start reading the body.
    did_open_ = true;
    ProcessResponseInfo(loader_.GetResponseInfo());
    ReadMore();
  } else {
    // Done reading (possibly with an error given by 'result').
    *result_ = 1;
    sem_post(&done_);
  }
}

void pp::AppEnginePost::ReadMore() {
  pp::CompletionCallback cc = factory_.NewCallback(&AppEnginePost::OnRead);
  int32_t rv = loader_.ReadResponseBody(buf_, sizeof(buf_), cc);
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
  std::vector<char>::size_type pos = data_.size();
  data_.resize(pos + length);
  memcpy(&data_[pos], bytes, length);
}

int pp::AppEngineUrlRequest::Read(const std::string& path, std::vector<char>& dst) {
  fprintf(stderr, "In AppEngineUrlLoader::read\n");
  KeyValueList fields;
  std::vector<char> filename_vec(path.begin(), path.end());
  fields.push_back(KeyValue("filename", &filename_vec));

  fprintf(stderr, "fields initiatlized\n");
  sem_t done;
  sem_init(&done, 0, 0);
  fprintf(stderr, "semaphore initialized\n");
  int raw_result;
  AppEnginePost post(pepper_instance_, done, &raw_result);
  fprintf(stderr, "post initialized\n");
  post.Post(base_url_ + "/read", fields);
  fprintf(stderr, "waiting on sem\n");
  sem_wait(&done);
  fprintf(stderr, "done waiting on sem\n");
  if (!raw_result) return 0;
  fprintf(stderr, "getting data\n");
  const std::vector<char>& data = post.get_data();
  fprintf(stderr, "got data\n");
  if (data.size() < 1) return 0;
  if (data[0] != '1') return 0;
  dst = std::vector<char>(data.begin() + 1, data.end());
  return 1;
}

int pp::AppEngineUrlRequest::Write(const std::string& path, const std::vector<char>& data) {
  KeyValueList fields;
  std::vector<char> filename_vec(path.begin(), path.end());
  fields.push_back(KeyValue("filename", &filename_vec));

  fields.push_back(KeyValue("data", &data));

  sem_t done;
  sem_init(&done, 0, 0);
  int raw_result;
  AppEnginePost post(pepper_instance_, done, &raw_result);
  post.Post(base_url_ + "/write", fields);
  fprintf(stderr, "waiting on sem\n");
  sem_wait(&done);
  fprintf(stderr, "done waiting on sem\n");
  if (!raw_result) return 0;
  const std::vector<char>& post_data = post.get_data();
  return post_data.size() == 1 && post_data[0] == '1';
}

int pp::AppEngineUrlRequest::List(const std::string& path, std::vector<std::string>& dst) {
  KeyValueList fields;
  std::vector<char> filename_vec(path.begin(), path.end());
  fields.push_back(KeyValue("prefix", &filename_vec));

  sem_t done;
  sem_init(&done, 0, 0);
  int raw_result;
  AppEnginePost post(pepper_instance_, done, &raw_result);
  post.Post(base_url_ + "/list", fields);
  sem_wait(&done);
  if (!raw_result) return 0;
  const std::vector<char>& data = post.get_data();
  dst.clear();
  std::vector<char>::const_iterator i = data.begin();
  while (i != data.end()) {
    std::vector<char>::const_iterator next = std::find(i, data.end(), '\n');
    if (next == data.end()) break;
    dst.push_back(std::string(i, next - 1));
    i = next;
  }
  return 1;
}

int pp::AppEngineUrlRequest::Remove(const std::string& path) {
  KeyValueList fields;
  std::vector<char> filename_vec(path.begin(), path.end());
  fields.push_back(KeyValue("filename", &filename_vec));

  sem_t done;
  sem_init(&done, 0, 0);
  int raw_result;
  AppEnginePost post(pepper_instance_, done, &raw_result);
  post.Post(base_url_ + "/remove", fields);
  sem_wait(&done);
  if (!raw_result) return 0;
  const std::vector<char>& data = post.get_data();
  return data.size() == 1 && data[0] == '1';
}

