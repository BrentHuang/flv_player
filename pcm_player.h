#ifndef PCM_PLAYER_H
#define PCM_PLAYER_H

#include <QObject>

class PcmPlayer : public QObject
{
    Q_OBJECT

public:
    PcmPlayer();
    virtual ~PcmPlayer();
};

#endif // PCM_PLAYER_H
