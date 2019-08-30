//
// Created by jxq on 19-8-28.
//

#ifndef MYMUDUO_SOCKETSOPS_H
#define MYMUDUO_SOCKETSOPS_H


#include <cstdint>
#include <endian.h>
#include <arpa/inet.h>


namespace muduo
{
    namespace sockets
    {
        //  (无符号)64字节 主机转网络
        inline uint64_t hostToNetwork64(uint64_t host64)
        {
            return htobe64(host64);
        }

        // 主机顺序转换成网络顺序
        inline uint32_t hostToNetwork32(uint32_t host32)
        {
            return htonl(host32);
        }

        // 主机顺序转换成网络顺序
        inline uint16_t hostToNetwork16(uint16_t host16)
        {
            return htons(host16);
        }

        //  (无符号)64字节 网络转主机
        inline uint64_t networkToHost64(uint64_t net64)
        {
            return be64toh(net64);
        }

        // 网络顺序转换成主机顺序
        inline uint32_t networkToHost32(uint32_t net32)
        {
            return ntohl(net32);
        }

        // 网络顺序转换成主机顺序
        inline uint16_t networkToHost16(uint16_t net16)
        {
            return ntohs(net16);
        }

        ///
        /// Creates a non-blocking socket file descriptor,
        /// abort if any error.
        int createNonblockingOrDie();

        void bindOrDie(int sockfd, const struct sockaddr_in& addr);
        void listenOrDie(int sockfd);
        int accept(int sockfd, struct sockaddr_in* addr);
        void close(int sockfd);

        // 从struct sockaddr_in转换成1.2.3.4::5形式
        void toHostPort(char* buf, size_t size,
                const struct sockaddr_in& addr);
        // 从1.2.3.4::5形式转换成struct sockaddr_in
        void fromHostPort(const char* ip, uint16_t port,
                          struct sockaddr_in* addr);
        struct sockaddr_in getLocalAddr(int sockfd);
        int getSockError(int sockfd);
    }


}


#endif //MYMUDUO_SOCKETSOPS_H
