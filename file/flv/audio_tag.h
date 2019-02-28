#ifndef AUDIO_TAG_H
#define AUDIO_TAG_H

#include <memory>
#include "flv_base.h"

namespace flv
{
struct AudioTag : public Tag, public std::enable_shared_from_this<AudioTag>
{
    int sound_format; // 音频格式，10 = AAC
    int sound_rate; // 采样率，3 = 44-kHz. For AAC: always 3
    int sound_size; // 采样精度，0 = snd8Bit 1 = snd16Bit. 压缩过的音频都是16bit
    int sound_type; // 声道，0 = mono单声道，1 = stereo立体声道。AAC永远是1

    AudioTag()
    {
        sound_format = 0;
        sound_rate = 0;
        sound_size = 0;
        sound_type = 0;
    }

    virtual ~AudioTag() {}

    int Build(int tag_idx, const TagHead& tag_head, const unsigned char* buf, int left_len) override;
};
}

#endif // AUDIO_TAG_H
