#ifndef YUV420P_PLAYER_H
#define YUV420P_PLAYER_H

#include <memory>
#include <QObject>
#include <QTimer>
#include <QVector>
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

    struct Yuv420pCtx
    {
        std::shared_ptr<Yuv420p> yuv420p;
        qint64 play_time;
        int duration;
    };

    typedef QVector<Yuv420pCtx> Yuv420pCtxVec;
    Yuv420pCtxVec yuv420p_ctx_vec_;
};

#endif // YUV420P_PLAYER_H
