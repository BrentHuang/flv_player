#include "pcm_device.h"
#include <QDebug>

PcmDevice::PcmDevice() : audio_output_(), buf_()
{
    pos_ = 0;

    open(QIODevice::ReadOnly);

    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo device_info = QAudioDeviceInfo::defaultOutputDevice();

    if (!device_info.isFormatSupported(format))
    {
        qWarning() << "fefault format not supported - trying to use nearest";
        format = device_info.nearestFormat(format);
    }

    audio_output_.reset(new QAudioOutput(device_info, format));
    audio_output_.get()->start(this);
}

PcmDevice::~PcmDevice()
{
    audio_output_.get()->stop();
    close();

    pos_ = 0;
    buf_.clear();
}

void PcmDevice::OnPcmReady(std::shared_ptr<Pcm> pcm)
{
    buf_.append((const char*) pcm.get()->data.data(), pcm.get()->data.size());
}

qint64 PcmDevice::readData(char* data, qint64 maxSize)
{
    qint64 total = 0;

    if (!buf_.isEmpty())
    {
        while (maxSize - total > 0)
        {
            const qint64 chunk = qMin((buf_.size() - pos_), maxSize - total);
            memcpy(data + total, buf_.constData() + pos_, chunk);
            pos_ = (pos_ + chunk) % buf_.size();
            total += chunk;
        }
    }

    return total;
}

qint64 PcmDevice::writeData(const char* data, qint64 maxSize)
{
    Q_UNUSED(data);
    Q_UNUSED(maxSize);

    return 0;
}

qint64 PcmDevice::bytesAvailable() const
{
    return buf_.size() + QIODevice::bytesAvailable();
}
