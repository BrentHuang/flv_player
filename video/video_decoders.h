#ifndef VIDEO_DECODERS_H
#define VIDEO_DECODERS_H

#include <QObject>
#include <memory>
#include "file/flv/video_tag.h"
#include "video/h264/openh264/openh264_decoder.h"
#include "video/h264/ffmpeg/ffmpeg_h264_decoder.h"

class VideoDecoders : public QObject
{
    Q_OBJECT

public:
    VideoDecoders();
    virtual ~VideoDecoders();

public slots:
    void OnFlvH264TagReady(std::shared_ptr<flv::VideoTag> flv_h264_tag);

private:
    openh264::H264Decoder openh264_decoder_;
    ffmpeg::H264Decoder ffmpeg_h264_decoder_;
};

#endif // VIDEO_DECODERS_H
