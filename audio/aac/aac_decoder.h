#ifndef AAC_DECODER_H
#define AAC_DECODER_H

#include <memory>
#include <vector>
#include <qsystemdetection.h>

#if defined(Q_OS_WIN)
#if defined(_MSC_VER)
#include <aacdecoder_lib.h>
#endif
#elif defined(Q_OS_MACOS)
#elif defined(Q_OS_LINUX)
#include <fdk-aac/aacdecoder_lib.h>
#endif

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

#include "file/flv/audio_tag.h"
#include "fdkaac_dec.h"
#include "pcm.h"

class AACDecoder
{
public:
    AACDecoder();
    ~AACDecoder();

    int Initialize();
    void Finalize();

    void OnFlvAacTagReady(std::shared_ptr<flv::AudioTag> flv_aac_tag);

private:
    std::unique_ptr<unsigned char[]> ParseAudioSpecificConfig(int& media_len, std::shared_ptr<flv::AudioTag> flv_aac_tag);
    std::unique_ptr<unsigned char[]> ParseRawAAC(int& media_len, std::shared_ptr<flv::AudioTag> flv_aac_tag, int aac_profile, int sample_rate_index, int channels);

    std::vector<std::shared_ptr<Pcm>> DecodeByFdkaac(const unsigned char* media, int media_len, int flv_tag_idx, unsigned int pts);
    std::shared_ptr<Pcm> DecodeByFFMpeg(const unsigned char* media, int media_len, int flv_tag_idx, unsigned int pts);

private:
    // fdkaac
    AacDecoder* fdkaac_dec_;

    // ffmpeg
    AVCodecContext* codec_ctx_;
    struct SwrContext* au_convert_ctx_;

    //
    int fdkaac_pcm_size_;

    int aac_profile_;
    int sample_rate_index_;
    int channels_;
};

#endif // AAC_DECODER_H
