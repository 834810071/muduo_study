// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include "../reactor/Buffer.h"
#include "HttpContext.h"
#include <algorithm>

using namespace muduo;
using namespace muduo::net;

// 解析请求行
bool HttpContext::processRequestLine(const char* begin, const char* end)
{
  // 请求方法 请求地址 协议版本
  bool succeed = false;
  const char* start = begin;
  const char* space = std::find(start, end, ' ');

  if (space != end && request_.setMethod(start, space)) // 设置请求方法
  {
    start = space+1;
    space = std::find(start, end, ' ');
    if (space != end)
    {
      // 解析URI
      const char* question = std::find(start, space, '?');
      if (question != space)
      {
        request_.setPath(start, question);
        request_.setQuery(question, space);
      }
      else
      {
        request_.setPath(start, space);
      }
      // 解析HTTP版本号
      start = space+1;
      succeed = end-start == 8 && std::equal(start, end-1, "HTTP/1.");
      if (succeed)
      {
        if (*(end-1) == '1')
        {
          request_.setVersion(HttpRequest::kHttp11);
        }
        else if (*(end-1) == '0')
        {
          request_.setVersion(HttpRequest::kHttp10);
        }
        else
        {
          succeed = false;
        }
      }
    }
  }
  return succeed;
}

// 解析请求
// return false if any error
bool HttpContext::parseRequest(muduo::Buffer* buf, Timestamp receiveTime)
{
  bool ok = true;
  bool hasMore = true;
  while (hasMore)
  {
    // 解析请求行
    if (state_ == kExpectRequestLine)
    {
      const char* crlf = buf->findCRLF();   //查找回车换行
      if (crlf)
      {
        // 开始解析请求行
        ok = processRequestLine(buf->peek(), crlf);
        if (ok)
        {
          // 解析成功
          request_.setReceiveTime(receiveTime);
          // 回收请求行buffer
          buf->retrieveUntil(crlf + 2);
          state_ = kExpectHeaders;
        }
        else
        {
          hasMore = false;
        }
      }
      else
      {
        hasMore = false;
      }
    }
    // 解析请求头
    else if (state_ == kExpectHeaders)
    {
      const char* crlf = buf->findCRLF();
      if (crlf)
      {
        // 请求头格式   Key : value
        // 冒号
        const char* colon = std::find(buf->peek(), crlf, ':');
        if (colon != crlf)
        {
            // 添加请求头键值对
          request_.addHeader(buf->peek(), colon, crlf);
        }
        else    // 空行
        {
          // empty line, end of header
          // FIXME:
          state_ = kGotAll;
          hasMore = false;
        }
        buf->retrieveUntil(crlf + 2); // 回收
      }
      else
      {
        hasMore = false;
      }
    }
    else if (state_ == kExpectBody)
    {
      // FIXME:
    }
  }
  return ok;
}
