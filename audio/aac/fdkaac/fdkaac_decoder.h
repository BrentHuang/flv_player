#ifndef FDKAAC_DECODER_H
#define FDKAAC_DECODER_H

#include <memory>

#if defined(Q_OS_WIN)
#if defined(_MSC_VER)
#include <aacdecoder_lib.h>
#endif
#elif defined(Q_OS_MACOS)
#elif defined(Q_OS_LINUX)
#include <fdk-aac/aacdecoder_lib.h>
#endif

#include "file/flv/audio_tag.h"
#include "fdkaac_dec.h"

namespace fdkaac
{
class AACDecoder
{
public:
    AACDecoder();
    virtual ~AACDecoder();

    void OnFlvAacTagReady(std::shared_ptr<flv::AudioTag> flv_aac_tag);

private:
    std::unique_ptr<unsigned char[]> ParseAudioSpecificConfig(int& media_len, std::shared_ptr<flv::AudioTag> flv_aac_tag);
    std::unique_ptr<unsigned char[]> ParseRawAAC(int& media_len, std::shared_ptr<flv::AudioTag> flv_aac_tag, int aac_profile, int sample_rate_index, int channels);

private:
    AacDecoder* fdkaac_dec_;
    int pcm_size_;
    int aac_profile_;
    int sample_rate_index_;
    int channels_;
};
}

#endif // FDKAAC_DECODER_H
