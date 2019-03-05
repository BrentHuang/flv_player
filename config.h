#ifndef CONFIG_H
#define CONFIG_H

#include <QMutex>

enum
{
    AUDIO_DECODER_FDKAAC = 0,
    AUDIO_DECODER_FFMPEG = 1
};

enum
{
    VIDEO_DECODER_OPENH264 = 0,
    VIDEO_DECODER_FFMPEG = 1
};

class Config
{
public:
    Config();

    ~Config() {}

    void SetAudioDecoderId(int audio_decoder_id);
    int GetAudioDecoderId();

    void SetVideoDecoderId(int video_decoder_id);
    int GetVideoDecoderId();

private:
    QMutex mutex_;
    int audio_decoder_id_;
    int video_decoder_id_;
};

#endif // CONFIG_H
