#ifndef AUDIO_DECODERS_H
#define AUDIO_DECODERS_H

#include <memory>
#include <QObject>
#include "file/flv/audio_tag.h"
#include "aac/fdkaac/fdkaac_decoder.h"
#include "aac/ffmpeg/ffmpeg_aac_decoder.h"

class AudioDecoders : public QObject
{
    Q_OBJECT

public:
    AudioDecoders();
    virtual ~AudioDecoders();

public slots:
    void OnFlvAacTagReady(std::shared_ptr<flv::AudioTag> flv_aac_tag);

private:
    fdkaac::AACDecoder fdkaac_decoder_;
    ffmpeg::AACDecoder ffmpeg_aac_decoder_;
};

#endif // AUDIO_DECODERS_H
