#ifndef H264_DECODER_H
#define H264_DECODER_H

#include <wels/codec_api.h>
#include <memory>
#include "file/flv/video_tag.h"

class H264Decoder
{
public:
    H264Decoder();
    ~H264Decoder();

    void OnFlvH264TagReady(std::shared_ptr<flv::VideoTag> flv_h264_tag);

private:
    std::unique_ptr<unsigned char[]> ParseH264Configuration(int& media_len, std::shared_ptr<flv::VideoTag> flv_h264_tag);
    std::unique_ptr<unsigned char[]> ParseNalu(int& media_len, std::shared_ptr<flv::VideoTag> flv_h264_tag, int nalu_len_size);

private:
    ISVCDecoder* decoder_;
    int nalu_len_size_;
};

#endif // H264_DECODER_H
