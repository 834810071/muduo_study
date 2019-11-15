#include "../Buffer.h"
#include <string>

//#define BOOST_TEST_MODULE BufferTest
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/included/unit_test.hpp>

//using muduo::string;
using namespace std;
using muduo::Buffer;

static const size_t kCheapPrepend = 8;
static const size_t kInitialSize = 1024;

BOOST_AUTO_TEST_CASE(testBufferAppendRetrieve)
{
  Buffer buf;
  BOOST_CHECK_EQUAL(buf.readableBytes(), 0);
  BOOST_CHECK_EQUAL(buf.writableBytes(), kInitialSize);
  BOOST_CHECK_EQUAL(buf.prependableBytes(), kCheapPrepend);

  const string str(200, 'x');
  buf.append(str);
  BOOST_CHECK_EQUAL(buf.readableBytes(), str.size());
  BOOST_CHECK_EQUAL(buf.writableBytes(), kInitialSize - str.size());
  BOOST_CHECK_EQUAL(buf.prependableBytes(), kCheapPrepend);
}
