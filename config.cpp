#include "config.h"
#include <QMutexLocker>

Config::Config()
{
    audio_decoder_id_ = AUDIO_DECODER_FDKAAC;
    video_decoder_id_ = VIDEO_DECODER_OPENH264;
}

void Config::SetAudioDecoderId(int audio_decoder_id)
{
    QMutexLocker lock(&mutex_);
    audio_decoder_id_ = audio_decoder_id;
}

int Config::GetAudioDecoderId()
{
    QMutexLocker lock(&mutex_);
    return audio_decoder_id_;
}

void Config::SetVideoDecoderId(int video_decoder_id)
{
    QMutexLocker lock(&mutex_);
    video_decoder_id_ = video_decoder_id;
}

int Config::GetVideoDecoderId()
{
    QMutexLocker lock(&mutex_);
    return video_decoder_id_;
}
