#include "flv_base.h"
#include <QDebug>
#include "byte_util.h"

namespace flv
{
int FlvHead::Build(const unsigned char* buf, int left_len)
{
    (void) left_len;

    if (buf[0] != 'F' || buf[1] != 'L' || buf[2] != 'V')
    {
        qDebug() << __FILE__ << ":" << __LINE__ << "not flv file";
        return -1;
    }

    version = buf[3];
    has_audio = (buf[4] >> 2) & 0x01;
    has_video = (buf[4] >> 0) & 0x01;
    head_size = ShowU32(buf + 5);

    return 0;
}

int TagHead::Build(const unsigned char* buf, int left_len)
{
    (void) left_len;

    type = ShowU8(buf);
    data_size = ShowU24(buf + 1);
    timestamp = ShowU24(buf + 4);
    timestamp_ex = ShowU8(buf + 7);
    stream_id = ShowU24(buf + 8);

    return 0;
}

int Tag::Build(int tag_idx, const TagHead& tag_head, const unsigned char* buf, int left_len)
{
    (void) left_len;

    this->tag_idx = tag_idx;
    dts = (unsigned int)((tag_head.timestamp_ex << 24)) + tag_head.timestamp;
    this->tag_head = tag_head;
    tag_data.assign((const char*) (buf + TAG_HEAD_LEN), tag_head.data_size);

    return 0;
}
}
