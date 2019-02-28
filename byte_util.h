﻿#ifndef BYTE_UTIL_H
#define BYTE_UTIL_H

#include <stdint.h>

inline unsigned int ShowU32(const unsigned char* buf)
{
    return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]; // flv是大端序（网络序）：低地址端存放高位字节
}

inline unsigned int ShowU24(const unsigned char* buf)
{
    return (buf[0] << 16) | (buf[1] << 8) | (buf[2]);
}

inline unsigned int ShowU16(const unsigned char* buf)
{
    return (buf[0] << 8) | (buf[1]);
}

inline unsigned int ShowU8(const unsigned char* buf)
{
    return (buf[0]);
}

inline double HexStr2double(const unsigned char* hex, int length)
{
    char hexstr[length * 2];
    memset(hexstr, 0, sizeof(hexstr));

    for (int i = 0; i < length; ++i)
    {
        sprintf(hexstr + i * 2, "%02x", hex[i]);
    }

    double ret = 0;
    sscanf(hexstr, "%llx", (unsigned long long*) &ret);

    return ret;
}

inline void WriteU64(uint64_t& x, int length, int value)
{
    uint64_t mask = 0xFFFFFFFFFFFFFFFF >> (64 - length);
    x = (x << length) | ((uint64_t)value & mask);
}

//inline unsigned int WriteU32(unsigned int n)
//{
//    unsigned int nn = 0;
//    unsigned char* p = (unsigned char*) &n;
//    unsigned char* pp = (unsigned char*) &nn;
//    pp[0] = p[3];
//    pp[1] = p[2];
//    pp[2] = p[1];
//    pp[3] = p[0];
//    return nn;
//}

#endif // BYTE_UTIL_H
