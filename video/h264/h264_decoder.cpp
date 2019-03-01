#include "h264_decoder.h"
#include <QDebug>
#include <QThread>
#include "byte_util.h"
#include "yuv420p.h"
#include "signal_center.h"

static const unsigned int H264_START_CODE = 0x01000000;

H264Decoder::H264Decoder()
{
    decoder_ = nullptr;
    nalu_len_size_ = 4;

    if (WelsCreateDecoder(&decoder_) != 0 || nullptr == decoder_)
    {
        qDebug() << "WelsCreateDecoder failed";
        return;
    }

    SDecodingParam dec_param;
    memset(&dec_param, 0, sizeof(SDecodingParam));
    dec_param.uiTargetDqLayer = UCHAR_MAX;
    dec_param.eEcActiveIdc = ERROR_CON_SLICE_COPY;
    dec_param.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;

    if (decoder_->Initialize(&dec_param) != 0)
    {
        qDebug() << "decode param initialize failed";
        WelsDestroyDecoder(decoder_);
        decoder_ = nullptr;
        return;
    }
}

H264Decoder::~H264Decoder()
{
    if (decoder_ != nullptr)
    {
        decoder_->Uninitialize();
        WelsDestroyDecoder(decoder_);
        decoder_ = nullptr;
    }
}

void H264Decoder::OnFlvH264TagReady(std::shared_ptr<flv::VideoTag> flv_h264_tag)
{
//    qDebug() << "H264Decoder::OnFlvH264TagReady " << QThread::currentThreadId();

    const unsigned char* tag_data = (const unsigned char*) flv_h264_tag.get()->tag_data.data();
    const int avc_packet_type = tag_data[1];
    const int composition_time = (int) ShowU24(tag_data + 2);
    const unsigned int pts = flv_h264_tag.get()->dts + composition_time;

    std::unique_ptr<unsigned char[]> media = nullptr;
    int media_len = 0;

    if (0 == avc_packet_type)
    {
        media = ParseH264Configuration(media_len, flv_h264_tag);
    }
    else if (1 == avc_packet_type)
    {
        media = ParseNalu(media_len, flv_h264_tag, nalu_len_size_);
    }
    else
    {
        return; // TODO
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
        qDebug() << "h264 decode error";
        return;
    }

    if (1 == buf_info.iBufferStatus)
    {
        std::shared_ptr<Yuv420p> yuv420p(new Yuv420p());
        if (nullptr == yuv420p)
        {
            qDebug() << "failed to alloc memory";
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

        yuv420p.get()->pts = pts;

        emit SIGNAL_CENTER->Yuv420pReady(yuv420p);
    }
}

std::unique_ptr<unsigned char[]> H264Decoder::ParseH264Configuration(int& media_len, std::shared_ptr<flv::VideoTag> flv_h264_tag)
{
    // tag_data+5是h264数据的开始位置
    // 举例：0x01+sps[1]+sps[2]+sps[3]+0xFF+0xE1+sps size+sps+01+pps size+pps。sps和pps可以有多个，一般只有1个

    const unsigned char* pd = (const unsigned char*) flv_h264_tag.get()->tag_data.data();

    nalu_len_size_ = (pd[9] & 0x03) + 1;

    const int sps_size = ShowU16(pd + 11);
    const int pps_size = ShowU16(pd + 11 + (2 + sps_size) + 1);

    media_len = 4 + sps_size + 4 + pps_size;

    std::unique_ptr<unsigned char[]> media(new unsigned char[media_len]);
    if (NULL == media)
    {
        qDebug() << "failed to alloc memory";
        return nullptr;
    }

    unsigned char* data = media.get();
    memcpy(data, &H264_START_CODE, 4);
    memcpy(data + 4, pd + 11 + 2, sps_size);
    memcpy(data + 4 + sps_size, &H264_START_CODE, 4);
    memcpy(data + 4 + sps_size + 4, pd + 11 + 2 + sps_size + 2 + 1, pps_size);

    return media;
}

std::unique_ptr<unsigned char[]> H264Decoder::ParseNalu(int& media_len, std::shared_ptr<flv::VideoTag> flv_h264_tag, int nalu_len_size)
{
    const unsigned char* pd = (const unsigned char*) flv_h264_tag.get()->tag_data.data();

    std::unique_ptr<unsigned char[]> media(new unsigned char[flv_h264_tag.get()->tag_head.data_size + 10]);
    if (NULL == media)
    {
        qDebug() << "failed to alloc memory";
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
        media_len += (4 + nalu_Len);
        offset += (nalu_len_size + nalu_Len);
    }

    return media;
}
