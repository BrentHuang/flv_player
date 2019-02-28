#include "yuv420p_player.h"
#include <QDebug>
#include <QThread>
#include "signal_center.h"

Yuv420pPlayer::Yuv420pPlayer() : yuv420p_list_()
{
    timer_.reset(new QTimer());
    connect(timer_.get(), SIGNAL(timeout()), this, SLOT(OnTimer()));
    timer_.get()->start(20);
}

Yuv420pPlayer::~Yuv420pPlayer()
{
}

void Yuv420pPlayer::OnYuv420pReady(std::shared_ptr<Yuv420p> yuv420p)
{
    yuv420p_list_.push_back(yuv420p);
}

void Yuv420pPlayer::OnTimer()
{
    if (yuv420p_list_.empty())
    {
        return;
    }

    std::shared_ptr<Yuv420p> yuv420p = yuv420p_list_.front();
    yuv420p_list_.pop_front();

    emit SIGNAL_CENTER->Yuv420pPlay(yuv420p);
}
