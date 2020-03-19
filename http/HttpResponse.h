// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_HTTP_HTTPRESPONSE_H
#define MUDUO_NET_HTTP_HTTPRESPONSE_H

#include "../base/copyable.h"
#include "../base/Types.h"
#include "../reactor/Buffer.h"

#include <map>

// 解析出来请求头后就需要按照不同的请求方法、类型等构造响应了，
// muduo封装了HttpResponse类将响应实体保存到Buffer对象中，最后通过TcpConnection发送给客户端。

namespace muduo
{
namespace net
{

class Buffer;
class HttpResponse : public muduo::copyable
{
 public:
  enum HttpStatusCode
  {
    kUnknown,
    k200Ok = 200, // 请求正常
    k301MovedPermanently = 301, // 永久重定向
    k400BadRequest = 400, // 服务器无法理解客户端请求
    k404NotFound = 404,
  };

  explicit HttpResponse(bool close)
    : statusCode_(kUnknown),
      closeConnection_(close)
  {
  }

  void setStatusCode(HttpStatusCode code)
  { statusCode_ = code; }

  void setStatusMessage(const string& message)
  { statusMessage_ = message; }

  void setCloseConnection(bool on)
  { closeConnection_ = on; }

  bool closeConnection() const
  { return closeConnection_; }

  void setContentType(const string& contentType)
  { addHeader("Content-Type", contentType); } // 在响应中，Content-Type标头告诉客户端实际返回的内容的内容类型。

  // FIXME: replace string with StringPiece
  void addHeader(const string& key, const string& value)
  { headers_[key] = value; }

  void setBody(const string& body)
  { body_ = body; }

  void appendToBuffer(muduo::Buffer* output) const; // 将HttpResponse添加到Buffer

 private:
  std::map<string, string> headers_;  // 响应头
  HttpStatusCode statusCode_; // 状态响应码
  // FIXME: add http version
  string statusMessage_;  // 状态响应码对应的文本信息
  bool closeConnection_;  // 是否关闭连接
  string body_; // 响应报文
};

}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_HTTP_HTTPRESPONSE_H
