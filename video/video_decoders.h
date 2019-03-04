#ifndef VIDEO_DECODERS_H
#define VIDEO_DECODERS_H

#include <memory>
#include <QObject>
#include "file/flv/video_tag.h"
#include "video/h264/h264_decoder.h"

class VideoDecoders : public QObject
{
    Q_OBJECT

public:
    VideoDecoders();
    virtual ~VideoDecoders();

public slots:
    void OnFlvH264TagReady(std::shared_ptr<flv::VideoTag> flv_h264_tag);

private:
    H264Decoder h264_decoder_;
};

#endif // VIDEO_DECODERS_H
