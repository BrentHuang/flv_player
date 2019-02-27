#ifndef YUV420P_PLAYER_H
#define YUV420P_PLAYER_H

#include <memory>
#include <QObject>
#include <QTimer>
#include "video_widget.h"

class Yuv420pPlayer : public QObject
{
    Q_OBJECT

public:
    Yuv420pPlayer();
    virtual ~Yuv420pPlayer();

    std::shared_ptr<VideoWidget> GetVideoWidget()
    {
        return video_widget_;
    }

private slots:
    void OnTimer();

private:
    std::shared_ptr<VideoWidget> video_widget_;
    std::unique_ptr<QTimer> timer_;
};

#endif // YUV420P_PLAYER_H
