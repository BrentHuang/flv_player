#include "openh264_decoder.h"
#include <QDebug>
#include <QThread>
#include "byte_util.h"
#include "yuv420p.h"
#include "signal_center.h"

namespace openh264
{
//16bit宽的数0x1234在两种模式CPU内存中的存放方式（假设从地址0x4000开始存放）为：
//内存地址  小端模式存放内容    大端模式存放内容
//0x4000    0x34            0x12
//0x4001    0x12            0x34

#if LITTLE_ENDIAN
static const unsigned int H264_START_CODE = 0x01000000; // 本机是小端，在内存中就是00000001，也就是nalu的4字节起始码
#else
static const unsigned int H264_START_CODE = 0x00000001;
#endif

H264Decoder::H264Decoder()
{
    decoder_ = nullptr;
    nalu_len_size_ = 4;

    if (WelsCreateDecoder(&decoder_) != 0 || nullptr == decoder_)
    {
        qDebug() << __FILE__ << ":" << __LINE__ << "WelsCreateDecoder failed";
        return;
    }

    SDecodingParam dec_param;
    memset(&dec_param, 0, sizeof(SDecodingParam));
    dec_param.uiTargetDqLayer = UCHAR_MAX;
    dec_param.eEcActiveIdc = ERROR_CON_SLICE_COPY;
    dec_param.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;

    if (decoder_->Initialize(&dec_param) != 0)
    {
        qDebug() << __FILE__ << ":" << __LINE__ << "decode param initialize failed";
        WelsDestroyDecoder(decoder_);
        decoder_ = nullptr;
        return;
    }
}

H264Decoder::~H264Decoder()
{
    for (int i = 0; i < (int) vjj_sei_vec_.size(); i++)
    {
        delete vjj_sei_vec_[i].szUD;
    }

    if (decoder_ != nullptr)
    {
        decoder_->Uninitialize();
        WelsDestroyDecoder(decoder_);
        decoder_ = nullptr;
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

    // 解码后的数据存储在data中，格式为YUV420P
    uint8_t* data[3];
    SBufferInfo buf_info;
    memset(data, 0, sizeof(data));
    memset(&buf_info, 0, sizeof(SBufferInfo));

    DECODING_STATE ds = decoder_->DecodeFrame2(media.get(), media_len, data, &buf_info);
    if (ds != dsErrorFree)
    {
        qDebug() << __FILE__ << ":" << __LINE__ << "h264 decode error";
        return;
    }

    if (1 == buf_info.iBufferStatus)
    {
        std::shared_ptr<Yuv420p> yuv420p(new Yuv420p());
        if (nullptr == yuv420p)
        {
            qDebug() << __FILE__ << ":" << __LINE__ << "failed to alloc memory";
            return;
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

        yuv420p.get()->flv_tag_idx = flv_h264_tag.get()->tag_idx;
        yuv420p.get()->pts = pts;
        yuv420p.get()->end = false;

        emit SIGNAL_CENTER->Yuv420pReady(yuv420p);
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
    if (NULL == media)
    {
        qDebug() << __FILE__ << ":" << __LINE__ << "failed to alloc memory";
        return nullptr;
    }

    unsigned char* data = media.get();
    memcpy(data, &H264_START_CODE, 4);
    memcpy(data + 4, pd + 11 + 2, sps_size);
    memcpy(data + 4 + sps_size, &H264_START_CODE, 4);
    memcpy(data + 4 + sps_size + 4, pd + 11 + 2 + sps_size + 2 + 1, pps_size);

    return media;
}

std::unique_ptr<unsigned char[]> H264Decoder::ParseNalus(int& media_len, std::shared_ptr<flv::VideoTag> flv_h264_tag, int nalu_len_size)
{
    const unsigned char* pd = (const unsigned char*) flv_h264_tag.get()->tag_data.data();

    std::unique_ptr<unsigned char[]> media(new unsigned char[flv_h264_tag.get()->tag_head.data_size + 10]);
    if (NULL == media)
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

        memcpy(data + media_len, &H264_START_CODE, 4);
        memcpy(data + media_len + 4, pd + offset + nalu_len_size, nalu_Len);
        ParseSEI(data + media_len, 4 + nalu_Len, flv_h264_tag.get()->dts);
        media_len += (4 + nalu_Len);
        offset += (nalu_len_size + nalu_Len);
    }

    return media;
}

void H264Decoder::ParseSEI(unsigned char* nalu, int nalu_len, int dts)
{
    // 起始码之后的一个字节是nal header，值和含义如下：
    // "0x06"，此时NRI为"00B"，NAL unit type为SEI类型。
    // “0x67”，此时NRI为“11B”，NAL unit type为SPS类型。
    // “0x68”，此时NRI为“11B”，NAL unit type为PPS类型。
    // “0x65”，此时NRI为“11B”，NAL unit type为IDR图像。
    if (nalu[4] != 0x06 || nalu[5] != 0x05) // "0x06"后一个字节为“0x05”，是SEI payload type，表示自定义消息
    {
        return;
    }

    // TODO https://blog.csdn.net/y601500359/article/details/80943990

    unsigned char* pd = nalu + 4 + 2;
    while (*pd++ == 0xff);

    const char* videojj_uuid = "VideojjLeonUUID";
    const char* p = (char*) pd;

    for (int i = 0; i < (int) strlen(videojj_uuid); i++)
    {
        if (p[i] != videojj_uuid[i])
        {
            return;
        }
    }

    VjjSEI sei;
    sei.nTimeStamp = dts;
    sei.nLen = nalu_len - (p - (char*) nalu) - 16 - 1;
    sei.szUD = new char[sei.nLen];
    memcpy(sei.szUD, p + 16, sei.nLen);
    vjj_sei_vec_.push_back(sei);
}
}
