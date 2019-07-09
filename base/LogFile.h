//
// Created by jxq on 19-7-4.
//

#ifndef MYMUDUO_LOGFILE_H
#define MYMUDUO_LOGFILE_H

#include "Mutex.h"
#include "Types.h"

#include <memory>

namespace muduo
{
    namespace FileUtil
    {
        class AppendFile;
    }

    class LogFile : noncopyable
    {
    public:
        LogFile(const string& basename,
                off_t rollSize,
                bool threadSafe = true,
                int flushInterval = 3,
                int checkEveryN = 1024);
        ~LogFile();

        void append(const char* logline, int len);
        void flush();
        bool rollFile();

    private:
        void append_unlock(const char* logline, int len);

        static string getLogFileName(const string& basename, time_t *now);

        const string basename_;
        const off_t rollSize_;   // off_t 文件操作指针移动
        const int flushInterval_;
        const int checkEveryN_;

        int count_;

        std::unique_ptr<MutexLock> mutex_;
        time_t startOfPeriod_;  // time_t 这种类型就是用来存储从1970年到现在经过了多少秒
        time_t lastRoll_;
        time_t lastFlush_;
        std::unique_ptr<FileUtil::AppendFile> file_;

        const static int kRollPerSeconds_ = 60 * 60 * 24;
    };
}   // namespace muduo


#endif //MYMUDUO_LOGFILE_H
