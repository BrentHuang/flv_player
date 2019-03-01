#ifndef PCM_DEVICE_H
#define PCM_DEVICE_H

#include <memory>
#include <list>
#include <QIODevice>
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
    typedef std::list<std::shared_ptr<Pcm>> PcmList;
    PcmList pcm_list_;
};

#endif // PCM_DEVICE_H
