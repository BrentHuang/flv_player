#ifndef H264_DECODER_H
#define H264_DECODER_H

#include <QObject>

class H264Decoder : public QObject
{
    Q_OBJECT

public:
    H264Decoder();
    virtual ~H264Decoder();
};

#endif // H264_DECODER_H
