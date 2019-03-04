#ifndef PCM_H
#define PCM_H

#include <string>

struct Pcm
{
    int flv_tag_idx;
    unsigned int pts;
    std::string data;

    Pcm() : data()
    {
        flv_tag_idx = 0;
        pts = 0;
    }

    int Build(const unsigned char* data, int size)
    {
        this->data.assign((const char*) data, size);
        return 0;
    }
};

#endif // PCM_H
