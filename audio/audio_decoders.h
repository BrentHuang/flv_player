#ifndef AUDIO_DECODERS_H
#define AUDIO_DECODERS_H

#include <memory>
#include <QObject>
#include "file/flv/audio_tag.h"
#include "aac/aac_decoder.h"

class AudioDecoders : public QObject
{
    Q_OBJECT

public:
    AudioDecoders();
    virtual ~AudioDecoders();

private:
    int Initialize();
    void Finalize();

public slots:
    void OnFlvAacTagReady(std::shared_ptr<flv::AudioTag> flv_aac_tag);

private:
    AACDecoder aac_decoder_;
};

#endif // AUDIO_DECODERS_H
