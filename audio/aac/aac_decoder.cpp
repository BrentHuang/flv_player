#include "aac_decoder.h"
#include <QDebug>
#include <QThread>

#ifdef __cplusplus
extern "C" {
#endif
#include <libswresample/swresample.h>
#ifdef __cplusplus
}
#endif

#include "byte_util.h"
#include "signal_center.h"
#include "global.h"

struct adts_header_t
{
    unsigned char syncword_0_to_8                       :   8;

    unsigned char protection_absent                     :   1;
    unsigned char layer                                 :   2;
    unsigned char ID                                    :   1;
    unsigned char syncword_9_to_12                      :   4;

    unsigned char channel_configuration_0_bit           :   1;
    unsigned char private_bit                           :   1;
    unsigned char sampling_frequency_index              :   4;
    unsigned char profile                               :   2;

    unsigned char frame_length_0_to_1                   :   2;
    unsigned char copyrignt_identification_start        :   1;
    unsigned char copyright_identification_bit          :   1;
    unsigned char home                                  :   1;
    unsigned char original_or_copy                      :   1;
    unsigned char channel_configuration_1_to_2          :   2;

    unsigned char frame_length_2_to_9                   :   8;

    unsigned char adts_buffer_fullness_0_to_4           :   5;
    unsigned char frame_length_10_to_12                 :   3;

    unsigned char number_of_raw_data_blocks_in_frame    :   2;
    unsigned char adts_buffer_fullness_5_to_10          :   6;
};

#define MAX_AUDIO_FRAME_SIZE 192000

AACDecoder::AACDecoder()
{
    fdkaac_dec_ = nullptr;
    codec_ctx_ = nullptr;
    fdkaac_pcm_size_ = 0;
    aac_profile_ = 0;
    sample_rate_index_ = 0;
    channels_ = 0;
}

AACDecoder::~AACDecoder()
{
}

int AACDecoder::Initialize()
{
    // fdkaac
    fdkaac_dec_ = new AacDecoder();
    if (nullptr == fdkaac_dec_)
    {
        qDebug() << __FILE__ << ":" << __LINE__ << "failed to alloc memory";
        return -1;
    }

    if (fdkaac_dec_->aacdec_init_adts() != 0)
    {
        qDebug() << __FILE__ << ":" << __LINE__ << "aacdec_init_adts failed";
        return -1;
    }

    fdkaac_pcm_size_ = fdkaac_dec_->aacdec_pcm_size();
    if (0 == fdkaac_pcm_size_)
    {
        fdkaac_pcm_size_ = 50 * 1024;
    }

    // ffmpeg
    AVCodec* codec  = avcodec_find_decoder(AV_CODEC_ID_AAC);
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

    codec_ctx_->codec_type = AVMEDIA_TYPE_AUDIO;
    codec_ctx_->sample_rate = 44100;
    codec_ctx_->channels = 2;
    codec_ctx_->bit_rate = 16;

    if (avcodec_open2(codec_ctx_, codec, NULL) < 0)
    {
        qDebug() << __FILE__ << ":" << __LINE__ << "avcodec_open2 failed";
        return -1;
    }

    return 0;
}

void AACDecoder::Finalize()
{
    // fdkaac
    if (fdkaac_dec_ != nullptr)
    {
        delete fdkaac_dec_;
        fdkaac_dec_ = nullptr;
    }

    // ffmpeg
    if (codec_ctx_ != nullptr)
    {
        avcodec_free_context(&codec_ctx_);
        codec_ctx_ = nullptr;
    }
}

void AACDecoder::OnFlvAacTagReady(std::shared_ptr<flv::AudioTag> flv_aac_tag)
{
//    qDebug() << __FILE__ << ":" << __LINE__ << "AACDecoder::OnFlvAacTagReady " << QThread::currentThreadId();

    const int aac_packet_type = flv_aac_tag.get()->tag_data[1];
    const unsigned int pts = flv_aac_tag.get()->dts;

    std::unique_ptr<unsigned char[]> media = nullptr;
    int media_len = 0;

    if (0 == aac_packet_type)
    {
        ParseAudioSpecificConfig(media_len, flv_aac_tag);
        return;
    }
    else if (1 == aac_packet_type)
    {
        media = ParseRawAAC(media_len, flv_aac_tag, aac_profile_, sample_rate_index_, channels_);
    }

    if (nullptr == media)
    {
        return;
    }

    std::vector<std::shared_ptr<Pcm>> pcm_vec;

    switch (GLOBAL->config.GetAudioDecoderId())
    {
        case AUDIO_DECODER_FDKAAC:
        {
            pcm_vec = DecodeByFdkAac(media.get(), media_len, flv_aac_tag.get()->tag_idx, pts);
        }
        break;

        case AUDIO_DECODER_FFMPEG:
        {
            pcm_vec = DecodeByFFMpeg(media.get(), media_len, flv_aac_tag.get()->tag_idx, pts);
        }
        break;

        default:
        {
        }
        break;
    }

    for (int i = 0; i < (int) pcm_vec.size(); ++i)
    {
        emit SIGNAL_CENTER->PcmReady(pcm_vec[i]);
    }
}

std::unique_ptr<unsigned char[]> AACDecoder::ParseAudioSpecificConfig(int& media_len, std::shared_ptr<flv::AudioTag> flv_aac_tag)
{
    (void) media_len;

    const unsigned char* pd = (const unsigned char*) flv_aac_tag.get()->tag_data.data();

    aac_profile_ = ((pd[2] & 0xf8) >> 3) - 1; // Object Type(5位，没有减1，这里减去1是什么意思？ TODO)
    sample_rate_index_ = ((pd[2] & 0x07) << 1) | (pd[3] >> 7);
    channels_ = (pd[3] >> 3) & 0x0f; // 1：单声道 2：双声道 以此类推

    return nullptr;
}

std::unique_ptr<unsigned char[]> AACDecoder::ParseRawAAC(int& media_len, std::shared_ptr<flv::AudioTag> flv_aac_tag, int aac_profile, int sample_rate_index, int channels)
{
    const int data_size = flv_aac_tag.get()->tag_head.data_size - 2;
    uint64_t bits = 0;

    WriteU64(bits, 12, 0xFFF); // TODO AAC格式，待分析
    WriteU64(bits, 1, 0);
    WriteU64(bits, 2, 0);
    WriteU64(bits, 1, 1);
    WriteU64(bits, 2, aac_profile);
    WriteU64(bits, 4, sample_rate_index);
    WriteU64(bits, 1, 0);
    WriteU64(bits, 3, channels);
    WriteU64(bits, 1, 0);
    WriteU64(bits, 1, 0);
    WriteU64(bits, 1, 0);
    WriteU64(bits, 1, 0);
    WriteU64(bits, 13, 7 + data_size);
    WriteU64(bits, 11, 0x7FF);
    WriteU64(bits, 2, 0);

    media_len = 7 + data_size;

    std::unique_ptr<unsigned char[]> media(new unsigned char[media_len]);
    if (nullptr == media)
    {
        qDebug() << __FILE__ << ":" << __LINE__ << "failed to alloc memory";
        return nullptr;
    }

    unsigned char* data = media.get();

    unsigned char p64[8];
    p64[0] = (unsigned char)(bits >> 56);
    p64[1] = (unsigned char)(bits >> 48);
    p64[2] = (unsigned char)(bits >> 40);
    p64[3] = (unsigned char)(bits >> 32);
    p64[4] = (unsigned char)(bits >> 24);
    p64[5] = (unsigned char)(bits >> 16);
    p64[6] = (unsigned char)(bits >> 8);
    p64[7] = (unsigned char)(bits);

    memcpy(data, p64 + 1, 7);
    memcpy(data + 7, flv_aac_tag.get()->tag_data.data() + 2, data_size);

    return media;
}

std::vector<std::shared_ptr<Pcm>> AACDecoder::DecodeByFdkAac(const unsigned char* media, int media_len, int flv_tag_idx, unsigned int pts)
{
    std::vector<std::shared_ptr<Pcm>> pcm_vec;

    char* aac_buf = (char*) media;
    const int aac_size = media_len;
    int pos = 0;

    std::unique_ptr<char[]> pcm_buf(new char[fdkaac_pcm_size_]);
    if (nullptr == pcm_buf)
    {
        qDebug() << __FILE__ << ":" << __LINE__ << "failed to alloc memory";
        return pcm_vec;
    }

    forever
    {
        if (aac_size - pos < 7)
        {
            break;
        }

        adts_header_t* adts = (adts_header_t*)(&aac_buf[0] + pos);

        if (adts->syncword_0_to_8 != 0xff || adts->syncword_9_to_12 != 0xf)
        {
            break;
        }

        const int aac_frame_size = adts->frame_length_0_to_1 << 11 | adts->frame_length_2_to_9 << 3 | adts->frame_length_10_to_12;
        if (pos + aac_frame_size > aac_size)
        {
            break;
        }

        int left_size = aac_frame_size;
        int ret = fdkaac_dec_->aacdec_fill(&aac_buf[0] + pos, aac_frame_size, &left_size);
        pos += aac_frame_size;

        if (ret != 0)
        {
            continue;
        }

        if (left_size > 0)
        {
            continue;
        }

        int valid_size = 0;
        ret = fdkaac_dec_->aacdec_decode_frame(pcm_buf.get(), fdkaac_pcm_size_, &valid_size);

        if (ret == AAC_DEC_NOT_ENOUGH_BITS)
        {
            continue;
        }

        if (ret != 0)
        {
            continue;
        }

        std::shared_ptr<Pcm> pcm(new Pcm());
        if (nullptr == pcm)
        {
            qDebug() << __FILE__ << ":" << __LINE__ << "failed to alloc memory";
            return pcm_vec;
        }

        pcm->Build((const unsigned char*) pcm_buf.get(), valid_size);

        pcm.get()->flv_tag_idx = flv_tag_idx;
        pcm.get()->pts = pts;

        pcm_vec.push_back(pcm);
    }

    return pcm_vec;
}

std::vector<std::shared_ptr<Pcm>> AACDecoder::DecodeByFFMpeg(const unsigned char* media, int media_len, int flv_tag_idx, unsigned int pts)
{
    std::vector<std::shared_ptr<Pcm>> pcm_vec;

    AVPacket packet;
    av_init_packet(&packet);

    packet.data = (uint8_t*) media;
    packet.size = media_len;

    int ret = avcodec_send_packet(codec_ctx_, &packet);
    if (ret != 0)
    {
        if (AVERROR(EAGAIN) == ret)
        {
            qDebug() << __FILE__ << ":" << __LINE__ << "EAGAIN" << ", flv tag idx: " << flv_tag_idx;
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
            qDebug() << "avcodec_send_packet failed, err: " << ret << ", flv tag idx: " << flv_tag_idx;
        }

        return pcm_vec;
    }

    AVFrame* frame = av_frame_alloc();
    if (nullptr == frame)
    {
        qDebug() << "av_frame_alloc failed";
        return pcm_vec;
    }

    struct SwrContext* swr_ctx = nullptr;
    uint8_t* data = nullptr;

    do
    {
        ret = avcodec_receive_frame(codec_ctx_, frame);
        if (ret != 0)
        {
            if (AVERROR(EAGAIN) == ret)
            {
                qDebug() << __FILE__ << ":" << __LINE__ << "EAGAIN" << ", flv tag idx: " << flv_tag_idx;
            }
            else if (AVERROR_EOF == ret)
            {
                qDebug() << __FILE__ << ":" << __LINE__ << "AVERROR_EOF" << ", flv tag idx: " << flv_tag_idx;
            }
            else if (AVERROR(EINVAL) == ret)
            {
                qDebug() << __FILE__ << ":" << __LINE__ << "EINVAL" << ", flv tag idx: " << flv_tag_idx;
            }
            else
            {
                qDebug() << "avcodec_receive_frame failed, err: " << ret << ", flv tag idx: " << flv_tag_idx;
            }

            break;
        }

        swr_ctx = swr_alloc_set_opts(nullptr,
                                     AV_CH_LAYOUT_STEREO,
                                     AV_SAMPLE_FMT_S16,
                                     44100,
                                     av_get_default_channel_layout(frame->channels),
                                     codec_ctx_->sample_fmt,
                                     codec_ctx_->sample_rate,
                                     0, nullptr);
        if (nullptr == swr_ctx)
        {
            qDebug() << "swr_alloc_set_opts failed";
            break;
        }

        ret = swr_init(swr_ctx);
        if (ret < 0)
        {
            qDebug() << "swr_init failed, err: " << AVERROR(ret);
            break;
        }

        data = (uint8_t*) malloc(MAX_AUDIO_FRAME_SIZE * 2); // *2表示双声道
        if (nullptr == data)
        {
            qDebug() << "failed to alloc memory";
            break;
        }

        if (swr_convert(swr_ctx,
                        &data, swr_get_out_samples(swr_ctx, frame->nb_samples),
                        (const uint8_t**) (frame->data), frame->nb_samples) < 0)
        {
            qDebug() << "swr_convert failed";
            break;
        }

        const int size = av_samples_get_buffer_size(nullptr, 2, frame->nb_samples, AV_SAMPLE_FMT_S16, 1);
        if (size < 0)
        {
            qDebug() << "swr_convert failed";
            break;
        }

        std::shared_ptr<Pcm> pcm(new Pcm());
        if (nullptr == pcm)
        {
            qDebug() << __FILE__ << ":" << __LINE__ << "failed to alloc memory";
            break;
        }

        pcm->Build((const unsigned char*) data, size);

        pcm.get()->flv_tag_idx = flv_tag_idx;
        pcm.get()->pts = pts;

        pcm_vec.push_back(pcm);
    } while (0);

    if (data != nullptr)
    {
        free(data);
    }

    if (swr_ctx != nullptr)
    {
        swr_free(&swr_ctx);
    }

    av_frame_free(&frame);
    return pcm_vec;
}
