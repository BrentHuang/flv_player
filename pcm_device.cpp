#include "pcm_device.h"
#include <QDebug>

PcmDevice::PcmDevice() : buf_()
{
    pos_ = 0;

    open(QIODevice::ReadOnly);
}

PcmDevice::~PcmDevice()
{
    close();

    buf_.clear();
    pos_ = 0;
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
