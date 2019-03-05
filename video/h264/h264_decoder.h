#ifndef H264_DECODER_H
#define H264_DECODER_H

#include <vector>
#include <memory>

#include <wels/codec_api.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus
}
#endif

#include "file/flv/video_tag.h"
#include "yuv420p.h"

class H264Decoder
{
public:
    H264Decoder();
    ~H264Decoder();

    int Initialize();
    void Finalize();

    void OnFlvH264TagReady(std::shared_ptr<flv::VideoTag> flv_h264_tag);

private:
    std::unique_ptr<unsigned char[]> ParseAVCDecorderConfigurationRecord(int& media_len, std::shared_ptr<flv::VideoTag> flv_h264_tag);
    std::unique_ptr<unsigned char[]> ParseNalus(int& media_len, std::shared_ptr<flv::VideoTag> flv_h264_tag, int nalu_len_size);
    void ParseSEI(unsigned char* nalu, int nalu_len, int dts);

    std::shared_ptr<Yuv420p> DecodeByOpenH264(const unsigned char* media, int media_len, int flv_tag_idx, unsigned int pts);
    std::shared_ptr<Yuv420p> DecodeByFFMpeg(const unsigned char* media, int media_len, int flv_tag_idx, unsigned int pts);

private:
    // openh264
    ISVCDecoder* decoder_;

    // ffmpeg
    AVCodecContext* codec_ctx_;

    //
    int nalu_len_size_;

    struct VjjSEI
    {
        char* szUD;
        int nLen;
        int nTimeStamp; // ms
    };

    std::vector<VjjSEI> vjj_sei_vec_;
};

#endif // H264_DECODER_H
