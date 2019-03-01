#ifndef GLOBAL_H
#define GLOBAL_H

#include <QWaitCondition>
#include <QMutex>
#include "singleton.h"

struct Global
{
    QWaitCondition file_parse_cond;
    QMutex file_parse_mutex;

    std::atomic_bool app_exit;

    static const int PCM_SIZE_TO_PAUSE_PARSING = 32768;
    static const int PCM_SIZE_TO_RESUME_PARSING = 16384;
    int pcm_size_in_queue;

    static const int YUV420P_COUNT_TO_PAUSE_PARSING = 10;
    static const int YUV420P_COUNT_TO_RESUME_PARSING = 5;
    int yuv420p_count_in_queue;

    Global() : file_parse_cond(), file_parse_mutex()
    {
        app_exit = false;
        pcm_size_in_queue = 0;
        yuv420p_count_in_queue = 0;
    }

    ~Global() {}
};

#define GLOBAL Singleton<Global>::Instance().get()

#endif // GLOBAL_H
