#ifndef PCM_H
#define PCM_H

#include <QDebug>

struct PcmFrame
{
    int tag_idx;
    const unsigned char* orig_data;
    unsigned char* data;
    int size;

    PcmFrame()
    {
        this->tag_idx = 0;
        this->orig_data = NULL;
        this->data = NULL;
        this->size = 0;
    }

    int Build(int tag_idx, const unsigned char* orig_data, int size)
    {
        this->tag_idx = tag_idx;

        this->data = new unsigned char[size];
        if (NULL == this->data)
        {
            qDebug() << "failed to alloc memory";
            return -1;
        }

        memcpy(this->data, orig_data, size);
        this->size = size;

        return 0;
    }

    void Release()
    {
        if (this->data != NULL)
        {
            delete [] this->data;
            this->data = NULL;
        }
    }
};

typedef std::list<PcmFrame*> PcmFrameList;
typedef std::vector<PcmFrame*> PcmFrameVec;

#endif // PCM_H
