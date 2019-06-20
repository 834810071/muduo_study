//
// Created by jxq on 19-6-20.
//


#include <vector>
#include <stdio.h>
#include <typeinfo>
#include "../Timestamp.h"

using muduo::Timestamp;

void passByValue(Timestamp x)
{
    printf("%s\n", x.toString().c_str());
}

void passByConstReference(const Timestamp& x)
{
    printf("%s\n", x.toString().c_str());
}

void benchmark()
{
    const int kNumber = 1000 * 1000;

    std::vector<Timestamp> stamps;
    stamps.reserve(kNumber);
    for (int i = 0; i < kNumber; ++i)
    {
        stamps.push_back(Timestamp::now());
    }
    printf("%s\n", stamps.front().toString().c_str());
    printf("%s\n", stamps.back().toString().c_str());
    printf("%f\n", timeDifference(stamps.back(), stamps.front()));

    int increment[100] = {0};
    int64_t start = stamps.front().microSecondsSinceEpoch();    // 微秒为单位
    for (int i = 1; i < kNumber; ++i)
    {
        int64_t next = stamps[i].microSecondsSinceEpoch();
        int64_t inc = next - start;
        start = next;
        if (inc < 0)
        {
            printf("reverse!\n");
        }
        else if (inc < 100)
        {
            ++increment[inc];
        }
        else
        {
            printf("big gap %d\n", static_cast<int>(inc));
        }
    }

    for (int i = 0; i < 100; ++i)
    {
        printf("%2d: %d\n", i, increment[i]);
    }
}

int main(int argc, char** argv)
{
    Timestamp now(Timestamp::now());
    printf("%s\n", now.toString().c_str());
    passByValue(now);
    passByConstReference(now);
    benchmark();
    return 0;
}