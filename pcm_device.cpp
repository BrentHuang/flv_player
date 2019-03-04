#include "pcm_device.h"
#include <QDebug>
#include <QThread>
#include <QMutexLocker>
#include "av_sync.h"
#include "global.h"

PcmDevice::PcmDevice() : pcm_list_()
{
    open(QIODevice::ReadOnly);
}

PcmDevice::~PcmDevice()
{
    close();
}

void PcmDevice::OnPcmReady(std::shared_ptr<Pcm> pcm)
{
    pcm_list_.push_back(pcm);

    GLOBAL->file_parse_mutex.lock();
    GLOBAL->pcm_size_in_queue += pcm.get()->data.size();
    GLOBAL->file_parse_mutex.unlock();
}

qint64 PcmDevice::readData(char* data, qint64 maxSize)
{
//    qDebug() << __FILE__ << ":" << __LINE__ << "PcmDevice::readData " << QThread::currentThreadId(); // 主线程中

    if (pcm_list_.empty())
    {
        return 0;
    }

    qint64 total_size = 0;

    while (maxSize - total_size > 0)
    {
        if (pcm_list_.empty())
        {
            break;
        }

        std::shared_ptr<Pcm> current = pcm_list_.front();
        const qint64 chunk_size = qMin((qint64) current.get()->data.size(), maxSize - total_size);
        memcpy(data + total_size, current.get()->data.data(), chunk_size);

        if (chunk_size < (qint64) current.get()->data.size())
        {
            const int left_len = current.get()->data.size() - chunk_size;
            std::unique_ptr<char[]> left_data(new char[left_len]);
            memcpy(left_data.get(), current.get()->data.data() + chunk_size, left_len);
            current.get()->data.assign(left_data.get(), left_len);
        }
        else
        {
            pcm_list_.pop_front();
        }

        total_size += chunk_size;
    }

    AV_SYNC->consumed_pcm_size += total_size;
    AV_SYNC->audio_drift = (AV_SYNC->consumed_pcm_size) * 1000 / (44100 << 2) - AVSync::TimeNowMSec();
    qDebug() << __FILE__ << ":" << __LINE__ << "audio drift: " << AV_SYNC->audio_drift;

    GLOBAL->file_parse_mutex.lock();
    GLOBAL->pcm_size_in_queue -= total_size;

    if (GLOBAL->pcm_size_in_queue <= Global::PCM_SIZE_TO_RESUME_PARSING)
    {
        GLOBAL->file_parse_cond.wakeAll();
    }

    GLOBAL->file_parse_mutex.unlock();

    return total_size;
}

qint64 PcmDevice::writeData(const char* data, qint64 maxSize)
{
    Q_UNUSED(data);
    Q_UNUSED(maxSize);

    return 0;
}

qint64 PcmDevice::bytesAvailable() const
{
    QMutexLocker lock(&GLOBAL->file_parse_mutex);
    return GLOBAL->pcm_size_in_queue + QIODevice::bytesAvailable();
}
