#ifndef AV_SYNC_H
#define AV_SYNC_H

#include <chrono>
#include <qglobal.h>
#include "singleton.h"

struct AVSync
{
    qint64 consumed_pcm_size;
    qint64 audio_drift;
    qint64 video_drift;

    AVSync()
    {
        consumed_pcm_size = 0;
        audio_drift = 0;
        video_drift = 0;
    }

    ~AVSync() {}

    static qint64 TimeNowMSec()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch()).count();
    }
};

#define AV_SYNC Singleton<AVSync>::Instance().get()

#endif // AV_SYNC_H
