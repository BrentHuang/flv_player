#ifndef YUV420P_H
#define YUV420P_H

#include <string>

struct Plane
{
    int width;
    int height;
    std::string data;

    Plane() : data()
    {
        width = 0;
        height = 0;
    }

    ~Plane() {}

    int Build(const unsigned char* data, int width, int height, int stride)
    {
        this->width = width;
        this->height = height;

        for (int j = 0; j < this->height; ++j)
        {
            this->data.append((const char*) (data + j * stride), this->width);
        }

        return 0;
    }
};

struct Yuv420p
{
    int flv_tag_idx;
    unsigned int pts;
    Plane y;
    Plane u;
    Plane v;
    bool end;

    Yuv420p() : y(), u(), v()
    {
        flv_tag_idx = 0;
        pts = 0;
        end = false;
    }

    ~Yuv420p() {}
};

#endif // YUV420P_H
