#ifndef FFMPEG_H264_DECODER_H
#define FFMPEG_H264_DECODER_H

#include <vector>
#include <wels/codec_api.h>
#include <memory>
#include "file/flv/video_tag.h"

namespace ffmpeg
{
class H264Decoder
{
public:
    H264Decoder();
    ~H264Decoder();

    void OnFlvH264TagReady(std::shared_ptr<flv::VideoTag> flv_h264_tag);

private:
    std::unique_ptr<unsigned char[]> ParseAVCDecorderConfigurationRecord(int& media_len, std::shared_ptr<flv::VideoTag> flv_h264_tag);
    std::unique_ptr<unsigned char[]> ParseNalus(int& media_len, std::shared_ptr<flv::VideoTag> flv_h264_tag, int nalu_len_size);
    void ParseSEI(unsigned char* nalu, int nalu_len, int dts);

private:
    ISVCDecoder* decoder_;
    int nalu_len_size_;

    struct VjjSEI
    {
        char* szUD;
        int nLen;
        int nTimeStamp; // ms
    };

    std::vector<VjjSEI> vjj_sei_vec_;
};
}

#endif // FFMPEG_H264_DECODER_H
