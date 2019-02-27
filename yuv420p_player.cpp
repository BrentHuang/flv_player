#include "yuv420p_player.h"
#include <QDebug>

Yuv420pPlayer::Yuv420pPlayer()
{
    video_widget_.reset(new VideoWidget());

    timer_.reset(new QTimer());
    connect(timer_.get(), SIGNAL(timeout()), this, SLOT(OnTimer()));
    timer_.get()->start(10);
}

Yuv420pPlayer::~Yuv420pPlayer()
{

}

void Yuv420pPlayer::OnTimer()
{
    qDebug() << "Yuv420pPlayer::OnTimer()";
}
