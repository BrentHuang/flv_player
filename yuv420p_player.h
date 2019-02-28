#ifndef YUV420P_PLAYER_H
#define YUV420P_PLAYER_H

#include <memory>
#include <list>
#include <QObject>
#include <QTimer>
#include "yuv420p.h"

class Yuv420pPlayer : public QObject
{
    Q_OBJECT

public:
    Yuv420pPlayer();
    virtual ~Yuv420pPlayer();

public slots:
    void OnYuv420pReady(std::shared_ptr<Yuv420p> yuv420p);

private slots:
    void OnTimer();

private:
    std::unique_ptr<QTimer> timer_;
    typedef std::list<std::shared_ptr<Yuv420p>> Yuv420pList;
    Yuv420pList yuv420p_list_;
};

#endif // YUV420P_PLAYER_H
