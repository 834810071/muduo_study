// Copyright (c) 2015
// Author: Chrono Law
#include <iostream>
using namespace std;

#include <boost/format.hpp>
#include <boost/test/minimal.hpp> // 最小化测试头文件

int test_main( int argc, char* argv[] )
{
    using namespace boost;
    format fmt("%d-%d");

    BOOST_CHECK(fmt.size() != 0); // 断言测试通过, 如不通过不影响程序执行
    fmt % 12 % 34;
    BOOST_REQUIRE(fmt.str() == "12-34"); // 要求断言必须通过, 否则程序停止执行

    BOOST_ERROR("演示一条错误消息"); // 给出错误信息, 程序继续执行

    fmt.clear();
    fmt % 12;
    try
    {
        std::cout << fmt; // 输入参数不完整, 抛出异常
    }
    catch (...)
    {
        BOOST_FAIL("致命错误，测试终止"); // 给出错误信息, 程序运行停止
    }
    return 0; // 必须返回值
}