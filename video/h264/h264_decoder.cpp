#include "h264_decoder.h"
#include <QDebug>
#include <QThread>
#include <QSysInfo>
#include "byte_util.h"
#include "signal_center.h"
#include "global.h"

//16bit宽的数0x1234在两种模式CPU内存中的存放方式（假设从地址0x4000开始存放）为：
//内存地址  小端模式存放内容    大端模式存放内容
//0x4000    0x34            0x12
//0x4001    0x12            0x34

H264Decoder::H264Decoder() : sei_vec_()
{
    decoder_ = nullptr;
    codec_ctx_ = nullptr;
    nalu_len_size_ = 0;

    if (QSysInfo::LittleEndian == QSysInfo::ByteOrder)
    {
        // 本机是小端，在内存中就是00000001，也就是nalu的4字节起始码
        h264_start_code_ = 0x01000000;
    }
    else
    {
        h264_start_code_ = 0x00000001;
    }
}

H264Decoder::~H264Decoder()
{
}

int H264Decoder::Initialize()
{
    // openh264
    if (WelsCreateDecoder(&decoder_) != 0 || nullptr == decoder_)
    {
        qDebug() << __FILE__ << ":" << __LINE__ << "WelsCreateDecoder failed";
        return -1;
    }

    SDecodingParam dec_param;
    memset(&dec_param, 0, sizeof(SDecodingParam));
    dec_param.uiTargetDqLayer = UCHAR_MAX;
    dec_param.eEcActiveIdc = ERROR_CON_SLICE_COPY;
    dec_param.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;

    if (decoder_->Initialize(&dec_param) != 0)
    {
        qDebug() << __FILE__ << ":" << __LINE__ << "decode param initialize failed";
        return -1;
    }

    // ffmpeg
    AVCodec* codec  = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (nullptr == codec)
    {
        qDebug() << __FILE__ << ":" << __LINE__ << "avcodec_find_decoder failed";
        return -1;
    }

    codec_ctx_ = avcodec_alloc_context3(codec);
    if (nullptr == codec_ctx_)
    {
        qDebug() << __FILE__ << ":" << __LINE__ << "avcodec_alloc_context3 failed";
        return -1;
    }

    codec_ctx_->pix_fmt = AV_PIX_FMT_YUV420P;

    if (avcodec_open2(codec_ctx_, codec, nullptr) < 0)
    {
        qDebug() << __FILE__ << ":" << __LINE__ << "avcodec_open2 failed";
        return -1;
    }

    return 0;
}

void H264Decoder::Finalize()
{
    for (auto& sei : sei_vec_)
    {
        delete [] sei.data;
    }

    // openh264
    if (decoder_ != nullptr)
    {
        decoder_->Uninitialize();
        WelsDestroyDecoder(decoder_);
        decoder_ = nullptr;
    }

    // ffmpeg
    if (codec_ctx_ != nullptr)
    {
        avcodec_free_context(&codec_ctx_);
        codec_ctx_ = nullptr;
    }
}

void H264Decoder::OnFlvH264TagReady(std::shared_ptr<flv::VideoTag> flv_h264_tag)
{
//    qDebug() << __FILE__ << ":" << __LINE__ << "H264Decoder::OnFlvH264TagReady " << QThread::currentThreadId();

    const unsigned char* tag_data = (const unsigned char*) flv_h264_tag.get()->tag_data.data();
    const int avc_packet_type = tag_data[1];
    const int composition_time = (int) ShowU24(tag_data + 2);
    const unsigned int pts = flv_h264_tag.get()->dts + composition_time;

    std::unique_ptr<unsigned char[]> media = nullptr;
    int media_len = 0;

    if (0 == avc_packet_type)
    {
        // AVCDecoderConfigurationRecord.包含着是H.264解码相关比较重要的sps和pps信息，
        // 在给AVC解码器送数据流之前一定要把sps和pps信息送出，否则的话解码器不能正常解码。
        // 而且在解码器stop之后再次start之前，如seek、快进快退状态切换等，都需要重新送一遍sps和pps的信息。
        // AVCDecoderConfigurationRecord在FLV文件中一般情况也是出现1次，也就是第一个video tag.
        media = ParseAVCDecorderConfigurationRecord(media_len, flv_h264_tag);
    }
    else if (1 == avc_packet_type)
    {
        media = ParseNalus(media_len, flv_h264_tag, nalu_len_size_);
    }
    else
    {
        // =2时为AVC end of sequence，无data
        std::shared_ptr<Yuv420p> yuv420p(new Yuv420p());
        if (nullptr == yuv420p)
        {
            qDebug() << __FILE__ << ":" << __LINE__ << "failed to alloc memory";
            return;
        }

        yuv420p.get()->end = true;

        emit SIGNAL_CENTER->Yuv420pReady(yuv420p);
        return;
    }

    if (nullptr == media)
    {
        return;
    }

    std::shared_ptr<Yuv420p> yuv420p;

    switch (GLOBAL->config.GetVideoDecoderId())
    {
        case VIDEO_DECODER_OPENH264:
        {
            yuv420p = DecodeByOpenH264(media.get(), media_len, flv_h264_tag.get()->tag_idx, pts);
        }
        break;

        case VIDEO_DECODER_FFMPEG:
        {
            yuv420p = DecodeByFFMpeg(media.get(), media_len, flv_h264_tag.get()->tag_idx, pts);
        }
        break;

        default:
        {
        }
        break;
    }

    if (yuv420p != nullptr)
    {
        emit SIGNAL_CENTER->Yuv420pReady(yuv420p);
    }
    else
    {
        qDebug() << "frame type: " << flv_h264_tag.get()->frame_type;
    }
}

std::unique_ptr<unsigned char[]> H264Decoder::ParseAVCDecorderConfigurationRecord(int& media_len, std::shared_ptr<flv::VideoTag> flv_h264_tag)
{
    // | cfgVersion(8) | avcProfile(8) | profileCompatibility(8) |avcLevel(8) | reserved(6) | lengthSizeMinusOne(2) | reserved(3) | numOfSPS(5) |spsLength(16) | sps(n) | numOfPPS(8) | ppsLength(16) | pps(n) |
    const unsigned char* pd = (const unsigned char*) flv_h264_tag.get()->tag_data.data();

    const int cfg_version = pd[5]; // tag_data+5是h264数据的开始位置
    (void) cfg_version;
    const int avc_profile = pd[6];
    (void) avc_profile;
    const int profile_compatibility = pd[7];
    (void) profile_compatibility;
    const int avc_level = pd[8];
    (void) avc_level;

    nalu_len_size_ = (pd[9] & 0x03) + 1;

    const int num_of_sps = (pd[10] & 0x1f); // 一般也只有1个，下面就当做一个来取了
    (void) num_of_sps;
    const int sps_size = ShowU16(pd + 11);
    const int num_of_pps = pd[11 + 2 + sps_size]; // 一般也只有1个，下面就当做一个来取了
    (void) num_of_pps;
    const int pps_size = ShowU16(pd + 11 + (2 + sps_size) + 1);

    media_len = 4 + sps_size + 4 + pps_size;

    std::unique_ptr<unsigned char[]> media(new unsigned char[media_len]);
    if (nullptr == media)
    {
        qDebug() << __FILE__ << ":" << __LINE__ << "failed to alloc memory";
        return nullptr;
    }

    unsigned char* data = media.get();
    memcpy(data, &h264_start_code_, 4);
    memcpy(data + 4, pd + 11 + 2, sps_size);
    memcpy(data + 4 + sps_size, &h264_start_code_, 4);
    memcpy(data + 4 + sps_size + 4, pd + 11 + 2 + sps_size + 2 + 1, pps_size);

    return media;
}

std::unique_ptr<unsigned char[]> H264Decoder::ParseNalus(int& media_len, std::shared_ptr<flv::VideoTag> flv_h264_tag, int nalu_len_size)
{
    const unsigned char* pd = (const unsigned char*) flv_h264_tag.get()->tag_data.data();

    std::unique_ptr<unsigned char[]> media(new unsigned char[flv_h264_tag.get()->tag_head.data_size + 10]);
    if (nullptr == media)
    {
        qDebug() << __FILE__ << ":" << __LINE__ << "failed to alloc memory";
        return nullptr;
    }

    unsigned char* data = media.get();
    media_len = 0;
    int offset = 5;

    forever
    {
        if (offset >= flv_h264_tag.get()->tag_head.data_size)
        {
            break;
        }

        int nalu_Len;

        switch (nalu_len_size)
        {
            case 4:
            {
                nalu_Len = (int) ShowU32(pd + offset);
            }
            break;

            case 3:
            {
                nalu_Len = (int) ShowU24(pd + offset);
            }
            break;

            case 2:
            {
                nalu_Len = (int) ShowU16(pd + offset);
            }
            break;

            default:
            {
                nalu_Len = (int) ShowU8(pd + offset);
            }
            break;
        }

        memcpy(data + media_len, &h264_start_code_, 4);
        memcpy(data + media_len + 4, pd + offset + nalu_len_size, nalu_Len);
        ParseSEI(data + media_len, 4 + nalu_Len, flv_h264_tag.get()->dts);
        media_len += (4 + nalu_Len);
        offset += (nalu_len_size + nalu_Len);
    }

    return media;
}

void H264Decoder::ParseSEI(unsigned char* nalu, int nalu_len, int dts)
{
    // https://blog.csdn.net/yangzhongxuan/article/details/8003494
    // https://blog.csdn.net/u010925568/article/details/75040492
    // https://www.jianshu.com/p/d28e25ae339e

    // 起始码之后的一个字节是nal header，结构：
    // forbidden_bit(1bit) + nal_reference_bit(2bit) + nal_unit_type(5bit)
    // 1.forbidden_bit： 禁止位，初始为0，当网络发现NAL单元有比特错误时可设置该比特为1，以便接收方纠错或丢掉该单元。
    // 2.nal_reference_bit： nal重要性指示，标志该NAL单元的重要性，值越大，越重要，解码器在解码处理不过来的时候，可以丢掉重要性为0的NALU。
    // 3. nal unit type:
    // "0x05"，NAL unit type为IDR图像。
    // "0x06"，NAL unit type为SEI类型。
    // "0x07"，NAL unit type为SPS类型。
    // "0x08"，NAL unit type为PPS类型。

    //IDR帧就是I帧，但是I帧不一定是IDR帧，在一个完整的视频流单元中第一个图像帧是IDR帧，IDR帧是强制刷新帧，在解码过程中，当出现了IDR帧时，要更新sps、pps，原因是防止前面I帧错误，导致sps，pps参考I帧导致无法纠正。
    //GOP的全称是Group of picture图像组，也就是两个I帧之间的距离，GOP值越大，那么I帧率之间P帧和B帧数量越多，图像画质越精细，如果GOP是120，如果分辨率是720P，帧率是60，那么两I帧的时间就是120/60=2s.

    //自定义sei帧结构：|h264 start code(32)|nal header(8)|sei payload type(8)|...0xff|uuid(128)|payload(n)|
    if ((nalu[4] & 0x1f) != 0x06 || nalu[5] != 0x05) // "0x06"后一个字节为"0x05"，是SEI payload type，表示自定义消息。但是要注意的是：在H.264/AVC视频编码标准中，并没有规定SEI payload type的范围，所以表征payload type的字节数是浮动的。所以要解析其他类型的SEI，需要持续读取8bit，直到非0xff为止，然后把读取的数值累加，累加值即为SEI payload type
    {
        return;
    }

    const unsigned char* pd = nalu + 4 + 2; // h264 start code占4字节，nal header占1字节，SEI payload type占1字节

    // 接着的字节是payload size。读取SEI payload size和payload type逻辑类似，仍然是读取到0xff为止，这样可以支持任意长度的SEI payload添加。
    while (*pd++ == 0xff);

    // 接着的16个字节是uuid。这里可以比较一下是不是期望的uuid
    //FFMPEG uuid
    static unsigned char uuid[] = { 0xdc, 0x45, 0xe9, 0xbd, 0xe6, 0xd9, 0x48, 0xb7, 0x96, 0x2c, 0xd8, 0x20, 0xd9, 0x23, 0xee, 0xef };
    //self UUID
//    static unsigned char uuid[] = { 0x54, 0x80, 0x83, 0x97, 0xf0, 0x23, 0x47, 0x4b, 0xb7, 0xf7, 0x4f, 0x32, 0xb5, 0x4e, 0x06, 0xac };

    for (int i = 0; i < 16; ++i)
    {
        if (pd[i] != uuid[i])
        {
            qDebug() << "sei uuid not match";
            return;
        }
    }

    // 接着的字节是payload
    SEI sei;
    sei.timestamp = dts;
    memcpy(sei.uuid, pd, 16);
    sei.len = nalu_len - (pd - nalu) - 16 - 1; // 最后一个1是结尾的'\0'

    sei.data = new char[sei.len];
    if (nullptr == sei.data)
    {
        qDebug() << "failed to alloc memory";
        return;
    }

    memcpy(sei.data, pd + 16, sei.len);
    sei_vec_.push_back(sei);
}

std::shared_ptr<Yuv420p> H264Decoder::DecodeByOpenH264(const unsigned char* media, int media_len, int flv_tag_idx, unsigned int pts)
{
    // 解码后的数据存储在data中，格式为YUV420P
    uint8_t* data[3];
    SBufferInfo buf_info;
    memset(data, 0, sizeof(data));
    memset(&buf_info, 0, sizeof(SBufferInfo));

    DECODING_STATE ds = decoder_->DecodeFrame2(media, media_len, data, &buf_info);
    if (ds != dsErrorFree)
    {
        qDebug() << __FILE__ << ":" << __LINE__ << "h264 decode error";
        return nullptr;
    }

    if (buf_info.iBufferStatus != 1)
    {
        return nullptr;
    }

    // 是一个完整的帧
    std::shared_ptr<Yuv420p> yuv420p(new Yuv420p());
    if (nullptr == yuv420p)
    {
        qDebug() << __FILE__ << ":" << __LINE__ << "failed to alloc memory";
        return nullptr;
    }

    yuv420p->y.Build(data[0],
                     buf_info.UsrData.sSystemBuffer.iWidth,
                     buf_info.UsrData.sSystemBuffer.iHeight,
                     buf_info.UsrData.sSystemBuffer.iStride[0]);

    yuv420p->u.Build(data[1],
                     buf_info.UsrData.sSystemBuffer.iWidth >> 1,
                     buf_info.UsrData.sSystemBuffer.iHeight >> 1,
                     buf_info.UsrData.sSystemBuffer.iStride[1]);

    yuv420p->v.Build(data[2],
                     buf_info.UsrData.sSystemBuffer.iWidth >> 1,
                     buf_info.UsrData.sSystemBuffer.iHeight >> 1,
                     buf_info.UsrData.sSystemBuffer.iStride[1]);

    yuv420p.get()->flv_tag_idx = flv_tag_idx;
    yuv420p.get()->pts = pts;
    yuv420p.get()->end = false;

    return yuv420p;
}

std::shared_ptr<Yuv420p> H264Decoder::DecodeByFFMpeg(const unsigned char* media, int media_len, int flv_tag_idx, unsigned int pts)
{
    AVPacket packet;
    av_init_packet(&packet);

    packet.data = (uint8_t*) media;
    packet.size = media_len;

    int ret = avcodec_send_packet(codec_ctx_, &packet); // 解第一个视频帧会报no frame TODO
    if (ret != 0)
    {
        if (AVERROR(EAGAIN) == ret)
        {
            qDebug() << __FILE__ << ":" << __LINE__ << "EAGAIN" << ", flv tag idx: " << flv_tag_idx;

            // TODO 读出所有的frame直到eof，然后重新send这个packet
        }
        else if (AVERROR_EOF == ret)
        {
            qDebug() << __FILE__ << ":" << __LINE__ << "AVERROR_EOF" << ", flv tag idx: " << flv_tag_idx;
        }
        else if (AVERROR(EINVAL) == ret)
        {
            qDebug() << __FILE__ << ":" << __LINE__ << "EINVAL" << ", flv tag idx: " << flv_tag_idx;
        }
        else if (AVERROR(ENOMEM) == ret)
        {
            qDebug() << __FILE__ << ":" << __LINE__ << "ENOMEM" << ", flv tag idx: " << flv_tag_idx;
        }
        else
        {
            qDebug() << __FILE__ << ":" << __LINE__ << "avcodec_send_packet failed, err: " << ret << ", flv tag idx: " << flv_tag_idx;
        }

        return nullptr;
    }

    AVFrame* frame = av_frame_alloc();
    if (nullptr == frame)
    {
        qDebug() << __FILE__ << ":" << __LINE__ << "av_frame_alloc failed";
        return nullptr;
    }

    std::shared_ptr<Yuv420p> yuv420p;

    do
    {
        ret = avcodec_receive_frame(codec_ctx_, frame);
        if (ret != 0)
        {
            if (AVERROR(EAGAIN) == ret)
            {
                qDebug() << __FILE__ << ":" << __LINE__ << "EAGAIN" << ", flv tag idx: " << flv_tag_idx;

                // TODO 重新send packet
            }
            else if (AVERROR_EOF == ret)
            {
                qDebug() << __FILE__ << ":" << __LINE__ << "AVERROR_EOF" << ", flv tag idx: " << flv_tag_idx;

                // TODO 表示后面没有frame了，不需要继续receive了。在解析音频时一个packet中可能有多个frame
            }
            else if (AVERROR(EINVAL) == ret)
            {
                qDebug() << __FILE__ << ":" << __LINE__ << "EINVAL" << ", flv tag idx: " << flv_tag_idx;
            }
            else
            {
                qDebug() << __FILE__ << ":" << __LINE__ << "avcodec_receive_frame fail, err: " << ret << ", flv tag idx: " << flv_tag_idx;
            }

            break;
        }

        yuv420p.reset(new Yuv420p());
        if (nullptr == yuv420p)
        {
            qDebug() << __FILE__ << ":" << __LINE__ << "failed to alloc memory";
            break;
        }

        yuv420p->y.Build(frame->data[0],
                         frame->width,
                         frame->height,
                         frame->linesize[0]);

        yuv420p->u.Build(frame->data[1],
                         frame->width >> 1,
                         frame->height >> 1,
                         frame->linesize[1]);

        yuv420p->v.Build(frame->data[2],
                         frame->width >> 1,
                         frame->height >> 1,
                         frame->linesize[2]);

        yuv420p.get()->pts = pts;
        yuv420p.get()->flv_tag_idx = flv_tag_idx;
        yuv420p.get()->end = false;
    } while (0);

    av_frame_free(&frame);
    return yuv420p;
}
