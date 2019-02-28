#ifndef PCM_PLAYER_H
#define PCM_PLAYER_H

#include <memory>
#include <QObject>
#include "pcm_device.h"

class PcmPlayer : public QObject
{
    Q_OBJECT

public:
    PcmPlayer();
    virtual ~PcmPlayer();

public slots:
    void OnPcmReady(std::shared_ptr<Pcm> pcm);

private:
    PcmDevice pcm_device_;
};

#endif // PCM_PLAYER_H
