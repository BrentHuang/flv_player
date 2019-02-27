#ifndef AAC_DECODER_H
#define AAC_DECODER_H

#include <QObject>

class AacDecoder : public QObject
{
    Q_OBJECT

public:
    AacDecoder();
    virtual ~AacDecoder();
};

#endif // AAC_DECODER_H
