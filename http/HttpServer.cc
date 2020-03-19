// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include <boost/any.hpp>
#include "HttpServer.h"

#include "../base/Logging.h"
#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

using namespace muduo;
using namespace muduo::net;

namespace muduo
{
namespace net
{
namespace detail
{

// 默认http回调，返回错误码
void defaultHttpCallback(const HttpRequest&, HttpResponse* resp)
{
  resp->setStatusCode(HttpResponse::k404NotFound);
  resp->setStatusMessage("Not Found");
  resp->setCloseConnection(true);
}

}  // namespace detail
}  // namespace net
}  // namespace muduo

HttpServer::HttpServer(EventLoop* loop,
                       const InetAddress& listenAddr,
                       const string& name,
                       TcpServer::Option option)
  : server_(loop, listenAddr, name, option),  // TcpServer
    httpCallback_(detail::defaultHttpCallback)
{
  // 通过设置server_ ，最终设置到TcpConnection 的回调函数
  server_.setConnectionCallback(
      std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
  server_.setMessageCallback(
      std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void HttpServer::start()
{
  LOG_WARN << "HttpServer[" << server_.name()
    << "] starts listenning on " << server_.ipPort();
  server_.start();  // 启动线程池
}

// 新连接回调
void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
  // 当客户端如浏览器连接上来，根据以前的分析可知，
  // 调用HttpServer::onConnection(), 绑定HttpContext到TcpConnection中的 boost::any context_;
  if (conn->connected())
  {
    conn->setContext(HttpContext());  // TcpConnection与一个HttpContext绑定
  }
}

// 消息回调
void HttpServer::onMessage(const TcpConnectionPtr& conn,
                           muduo::Buffer* buf,
                           Timestamp receiveTime)
{
  HttpContext* context = boost::any_cast<HttpContext>(conn->getMutableContext());

  // 解析请求
  if (!context->parseRequest(buf, receiveTime))
  {
    conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->shutdown(); // 关闭连接
  }

  // 请求消息解析完毕
  if (context->gotAll())  // state_ == gotAll
  {
    onRequest(conn, context->request());
    context->reset(); // 本次请求处理完毕，重置HttpContext，适用于长连接(一个连接多次请求)
  }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
  const string& connection = req.getHeader("Connection");
  bool close = connection == "close" ||
    (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
  HttpResponse response(close); // 构造响应
  httpCallback_(req, &response);  // 用户回调
  muduo::Buffer buf;
  // 此时response已经构造好，将向客户发送Response添加到buffer中
  response.appendToBuffer(&buf);
  conn->send(&buf);
  // 如果非Keep-Alive则直接关掉
  if (response.closeConnection())
  {
    conn->shutdown();
  }
}

