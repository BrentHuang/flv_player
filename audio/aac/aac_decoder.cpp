#include "aac_decoder.h"
#include <QDebug>
#include <QThread>
#include "byte_util.h"
#include "pcm.h"
#include "signal_center.h"

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

AACDecoder::AACDecoder()
{
    fdkaac_dec_ = nullptr;
    aac_profile_ = 0;
    sample_rate_index_ = 0;
    channels_ = 0;

    fdkaac_dec_ = new AacDecoder();
    if (nullptr == fdkaac_dec_)
    {
        qDebug() << "failed to alloc memory";
        return;
    }

    if (fdkaac_dec_->aacdec_init_adts() != 0)
    {
        qDebug() << "aacdec_init_adts failed";
        return;
    }

    pcm_size_ = fdkaac_dec_->aacdec_pcm_size();
    if (0 == pcm_size_)
    {
        pcm_size_ = 50 * 1024;
    }
}

AACDecoder::~AACDecoder()
{
    if (fdkaac_dec_ != nullptr)
    {
        delete fdkaac_dec_;
        fdkaac_dec_ = nullptr;
    }
}

void AACDecoder::OnFlvAacTagReady(std::shared_ptr<flv::AudioTag> flv_aac_tag)
{
//    qDebug() << "AACDecoder::OnFlvAacTagReady " << QThread::currentThreadId();

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

    char* aac_buf = (char*) media.get();
    const int aac_size = media_len;
    int pos = 0;

    std::unique_ptr<char[]> pcm_buf(new char[pcm_size_]);
    if (nullptr == pcm_buf)
    {
        qDebug() << "failed to alloc memory";
        return;
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
        ret = fdkaac_dec_->aacdec_decode_frame(pcm_buf.get(), pcm_size_, &valid_size);

        if (ret == AAC_DEC_NOT_ENOUGH_BITS)
        {
            continue;
        }

        if (ret != 0)
        {
            continue;
        }

        std::shared_ptr<Pcm> pcm(new Pcm());
        if (NULL == pcm)
        {
            qDebug() << "failed to alloc memory";
            return;
        }

        pcm->Build((const unsigned char*) pcm_buf.get(), valid_size);
        pcm.get()->pts = pts;

        emit SIGNAL_CENTER->PcmReady(pcm);

        //        if (!wav)
        //        {
        //            if (fdkaac_dec->aacdec_sample_rate() <= 0)
        //            {
        //                break;
        //            }

        //            wav = wav_write_open("world.wav", fdkaac_dec->aacdec_sample_rate(), 16, fdkaac_dec->aacdec_num_channels());
        //            if (!wav)
        //            {
        //                break;
        //            }
        //        }

        //        wav_write_data(wav, (unsigned char*)&pcm_buf[0], valid_size);
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

    WriteU64(bits, 12, 0xFFF); // TODO 待分析
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
    if (NULL == media)
    {
        qDebug() << "failed to alloc memory";
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
