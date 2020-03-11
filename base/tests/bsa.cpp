//
// Created by jxq on 19-6-15.
//

#include <boost/static_assert.hpp>
#include <iostream>

using namespace std;

class TimeStamp
{
public:
    TimeStamp()
        :microSecondsSinceEpoch_(0)
    {

    }

    ~TimeStamp()
    {

    }
private:
    int64_t microSecondsSinceEpoch_;
};

BOOST_STATIC_ASSERT(sizeof(TimeStamp) == sizeof(int64_t));
// BOOST_STATIC_ASSERT(sizeof(short) == sizeof(int64_t));

int main(int argc, char** argv)
{
    cout << "BOOST_STATIC_ASSERT tests!" << endl;
    return 0;
}