#ifndef YUV420P_H
#define YUV420P_H

#include <QDebug>
#include <memory>

struct Plane
{
    unsigned char* data;
    int width;
    int height;
    int stride;

    Plane()
    {
        this->data = nullptr;
        this->width = 0;
        this->height = 0;
        this->stride = 0;
    }

    ~Plane()
    {
    }

    int Build(const unsigned char* orig_data, int width, int height, int stride)
    {
//        this->width = width;
//        this->height = height;
//        this->stride = stride;

//        this->data = std::make_shared < [this->width * this->height];
//        if (NULL == this->data)
//        {
//            qDebug() << "failed to alloc memory";
//            return -1;
//        }

//        for (int j = 0; j < this->height; ++j)
//        {
//            memcpy(this->data + j * this->width, orig_data + j * this->stride, this->width);
//        }

        return 0;
    }

    void Release()
    {

    }
};

struct Yuv420pFrame
{
    int flv_tag_idx;
    int pts;
    Plane y;
    Plane u;
    Plane v;

    Yuv420pFrame() : y(), u(), v()
    {
        this->flv_tag_idx = -1;
        this->pts = 0;
    }

    ~Yuv420pFrame()
    {
        this->y.Release();
        this->u.Release();
        this->v.Release();
    }

    void SetFlvTagIdx(int flv_tag_idx)
    {
        this->flv_tag_idx = flv_tag_idx;
    }

    void SetPts(int pts)
    {
        this->pts = pts;
    }
};

#endif // YUV420P_H
