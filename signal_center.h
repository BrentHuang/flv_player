#ifndef SIGNAL_CENTER_H
#define SIGNAL_CENTER_H

#include <QObject>
#include "singleton.h"
#include "file/flv/video_tag.h"
#include "file/flv/audio_tag.h"
#include "yuv420p.h"
#include "pcm.h"

class SignalCenter : public QObject
{
    Q_OBJECT

public:
    SignalCenter();
    virtual ~SignalCenter();

signals:
    void FlvFileOpen(const QString& file_path);
    void FlvH264TagReady(std::shared_ptr<flv::VideoTag> flv_h264_tag);
    void Yuv420pReady(std::shared_ptr<Yuv420p> yuv420p);
    void Yuv420pPlay(std::shared_ptr<Yuv420p> yuv420p);
    void FlvAacTagReady(std::shared_ptr<flv::AudioTag> flv_aac_tag);
    void PcmReady(std::shared_ptr<Pcm> pcm);
};

#define SIGNAL_CENTER Singleton<SignalCenter>::Instance().get()

#endif // SIGNAL_CENTER_H
