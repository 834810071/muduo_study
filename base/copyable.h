//
// Created by jxq on 19-6-15.
//

#ifndef MYMUDUO_COPYABLE_H
#define MYMUDUO_COPYABLE_H

namespace muduo
{
    class copyable
    {
    protected:
        copyable() = default;   // "=default"函数特性仅适用于类的特殊成员函数，且该特殊成员函数没有默认参数。
        ~copyable() = default;  // 该函数比用户自己定义的默认构造函数获得更高的代码效率
    };
}

#endif //MYMUDUO_COPYABLE_H
