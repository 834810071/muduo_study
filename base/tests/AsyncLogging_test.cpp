//
// Created by jxq on 19-11-18.
//

#include "../AsyncLogging.h"
#include "../Logging.h"

using namespace muduo;

AsyncLogging* logptr = NULL;

void asyncoutput(const char* msg, int len)
{
    logptr->append(msg, len);   // 前台进程传递日志
}

int main(int argc, char** argv)
{
    LOG_INFO << "hello";
    char name[256];
    strcpy(name, argv[0]);
    AsyncLogging asynclog("asynclog", 500*1000*1000);
    asynclog.start();

    logptr = &asynclog;
    Logger::setOutput(asyncoutput);
    for (int i = 0; i < 10; ++i)
    {
        LOG_INFO << "ASYNC LOG";
        struct timespec ts = {0, 500*1000*1000};
        nanosleep(&ts, NULL);
    };

    return 0;
}