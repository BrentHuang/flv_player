#ifndef PCM_DEVICE_H
#define PCM_DEVICE_H

#include <memory>
#include <QIODevice>
#include <QAudioFormat>
#include <QAudioOutput>
#include "pcm.h"

class PcmDevice: public QIODevice
{
public:
    PcmDevice();
    virtual ~PcmDevice();

    void OnPcmReady(std::shared_ptr<Pcm> pcm);

protected:
    qint64 readData(char* data, qint64 maxSize) override;
    qint64 writeData(const char* data, qint64 maxSize) override;
    qint64 bytesAvailable() const override;

private:
    std::unique_ptr<QAudioOutput> audio_output_;
    qint64 pos_;
    QByteArray buf_;
};

#endif // PCM_DEVICE_H
