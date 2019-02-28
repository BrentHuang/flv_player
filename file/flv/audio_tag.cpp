#include "audio_tag.h"
#include <QDebug>
#include "signal_center.h"

namespace flv
{
int AudioTag::Build(int tag_idx, const TagHead& tag_head, const unsigned char* buf, int left_len)
{
    if (Tag::Build(tag_idx, tag_head, buf, left_len) != 0)
    {
        return -1;
    }

    const unsigned char* pd = (const unsigned char*) tag_data.data();

    sound_format = (pd[0] & 0xf0) >> 4;
    sound_rate = (pd[0] & 0x0c) >> 2;
    sound_size = (pd[0] & 0x02) >> 1;
    sound_type = (pd[0] & 0x01);
//    qDebug() << "sound　type: " << sound_type;

    int ret = -1;

    switch (sound_format)
    {
        case 10:
        {
            emit SIGNAL_CENTER->FlvAacTagReady(shared_from_this());
            ret = 0;
        }
        break;

        default:
        {
            qDebug() << "audio format: " << sound_format;
        }
        break;
    }

    return ret;
}
}
