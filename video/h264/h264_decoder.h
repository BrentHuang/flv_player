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

//参考：https://www.jianshu.com/p/9522c4a7818d
//H.264原始码流(又称为裸流)，是由一个接一个的 NALU 组成的，
//而它的功能分为两层：视频编码层(VCL, Video Coding Layer)和网络提取层(NAL, Network Abstraction Layer)。
//VCL 数据即编码处理的输出，它表示被压缩编码后的视频数据序列。
//在 VCL 数据传输或存储之前，这些编码的 VCL 数据，先被映射或封装进 NAL 单元(以下简称 NALU，Nal Unit) 中。
//每个 NALU 包括一个原始字节序列负荷(RBSP, Raw Byte Sequence Payload)、一个对应于视频编码的 NALU 头部信息。
//RBSP 的基本结构是:在原始编码数据的后面填加了结尾比特：一个 bit“1”若干比特“0”，以便字节对齐。
//一帧图片经过 H.264 编码器之后，就被编码为一个或多个片（slice），而装载着这些片（slice）的载体，就是 NALU 了。
//片（slice）都是由 NALU 装载并进行网络传输的，但是这并不代表 NALU 内就一定是切片，NALU 还有可能装载其他用作描述视频的信息，例如 SEI、SPS、PPS 等。
//一帧（frame）对应一张图片，一张图片至少有一个或多个片（slice）。
//每个分片也包含着头和数据两部分：
//1、分片头中包含着分片类型、分片中的宏块(Macroblock)类型、分片帧的数量、分片属于那个图像以及对应的帧的设置和参数等信息。
//2、分片数据中则是宏块，这里就是我们要找的存储像素数据的地方。每片（slice）至少一个 宏块(Macroblock)，最多时每片包 整个图像的宏块。
//宏块是视频信息的主要承载者，因为它包含着每一个像素的亮度和色度信息。视频解码最主要的工作就是提供高效的方式从码流中获得宏块中的像素阵列。
//一个宏块由一个16×16亮度像素和附加的一个8×8 Cb和一个 8×8 Cr 彩色像素块组成。每个图象中，若干宏块被排列成片的形式。

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
    unsigned int h264_start_code_;
    int nalu_len_size_;

    struct SEI
    {
        int timestamp; // ms
        char uuid[16];
        char* data; // user data
        int len; // user data len
    };

    std::vector<SEI> sei_vec_;
};

#endif // H264_DECODER_H
