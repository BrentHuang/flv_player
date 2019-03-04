#include "video_tag.h"
#include <QDebug>
#include "signal_center.h"

namespace flv
{
int VideoTag::Build(int tag_idx, const TagHead& tag_head, const unsigned char* buf, int left_len)
{
    if (Tag::Build(tag_idx, tag_head, buf, left_len) != 0)
    {
        return -1;
    }

    const unsigned char* pd = (const unsigned char*) tag_data.data();

    frame_type = (pd[0] & 0xf0) >> 4;
    codec_id = pd[0] & 0x0f;

    int ret = -1;

    switch (codec_id)
    {
        case 7:
        {
            if (1 == frame_type || 2 == frame_type)
            {
                emit SIGNAL_CENTER->FlvH264TagReady(shared_from_this());
                ret = 0;
            }
            else
            {
                ret = 0; // 不处理
            }
        }
        break;

        default:
        {
            qDebug() << __FILE__ << ":" << __LINE__ << "vedio codec: " << codec_id;
        }
        break;
    }

    return ret;
}
}
