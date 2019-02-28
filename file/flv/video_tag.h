#ifndef VIDEO_TAG_H
#define VIDEO_TAG_H

#include <memory>
#include "flv_base.h"

namespace flv
{
struct VideoTag : public Tag, public std::enable_shared_from_this<VideoTag>
{
    int frame_type; // 1: keyframe (for AVC, a seekable frame)——h264的IDR，关键帧，可重入帧。 2: inter frame (for AVC, a non- seekable frame)——h264的普通帧
    int codec_id; // 7: AVC

    VideoTag()
    {
        frame_type = 0;
        codec_id = 0;
    }

    virtual ~VideoTag() {}

    int Build(int tag_idx, const TagHead& tag_head, const unsigned char* buf, int left_len) override;
};
}

#endif // VIDEO_TAG_H
