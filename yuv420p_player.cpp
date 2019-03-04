﻿#include "yuv420p_player.h"
#include <QDebug>
#include <QThread>
#include "signal_center.h"
#include "av_sync.h"
#include "global.h"

Yuv420pPlayer::Yuv420pPlayer() : yuv420p_ctx_vec_()
{
    timer_.reset(new QTimer());
    connect(timer_.get(), SIGNAL(timeout()), this, SLOT(OnTimer()));
    timer_.get()->start(20); // TODO
}

Yuv420pPlayer::~Yuv420pPlayer()
{
}

void Yuv420pPlayer::OnYuv420pReady(std::shared_ptr<Yuv420p> yuv420p)
{
    // 下列打印的数据显示，每个yuv420p大小为1179648字节(width*height*1.5)，播放时长为40毫秒
//    const int yuv420p_size = yuv420p.get()->y.data.size() + yuv420p.get()->u.data.size() + yuv420p.get()->v.data.size();
//    qDebug() << __FILE__ << ":" << __LINE__ << "yuv420p size: " << yuv420p_size << ", pts: " << yuv420p.get()->pts;

    Yuv420pCtx ctx = { yuv420p, 0, 0 };
    yuv420p_ctx_vec_.push_back(ctx);

    const int vec_size = yuv420p_ctx_vec_.size();
    if (vec_size > 1)
    {
        Yuv420pCtx& current = yuv420p_ctx_vec_[vec_size - 1];
        Yuv420pCtx& prev = yuv420p_ctx_vec_[vec_size - 2];
        current.duration = current.yuv420p.get()->pts - prev.yuv420p.get()->pts;
    }

    GLOBAL->file_parse_mutex.lock();
    ++GLOBAL->yuv420p_count_in_queue;
    GLOBAL->file_parse_mutex.unlock();
}

void Yuv420pPlayer::OnTimer()
{
    const int vec_size = yuv420p_ctx_vec_.size();
    switch (vec_size)
    {
        case 0:
        {
            return;
        }
        break;

        case 1:
        {
            Yuv420pCtx& current = yuv420p_ctx_vec_.front();
            std::shared_ptr<Yuv420p> yuv420p = current.yuv420p;

            if (yuv420p.get()->end)
            {
                yuv420p_ctx_vec_.pop_front();
                return; // 结束了
            }

            // 直接播放第一帧
            emit SIGNAL_CENTER->Yuv420pPlay(yuv420p);
            current.play_time = AVSync::TimeNowMSec();

            qDebug() << __FILE__ << ":" << __LINE__ << "flv tag idx: " << yuv420p.get()->flv_tag_idx
                     << ", video drift: " << AV_SYNC->video_drift
                     << ", current frame pts: " << yuv420p->pts;

            AV_SYNC->video_drift = yuv420p->pts - AVSync::TimeNowMSec();
        }
        break;

        default:
        {
            const qint64 now = AVSync::TimeNowMSec();
            const qint64 aclock = AV_SYNC->audio_drift + now;
            const qint64 vclock = AV_SYNC->video_drift + now;
            const qint64 delta = vclock - aclock;

            Yuv420pCtx& current = yuv420p_ctx_vec_.front(); // 当前帧

            qDebug() << __FILE__ << ":" << __LINE__ << "flv tag idx: " << current.yuv420p.get()->flv_tag_idx
                     << "audio drift: " << AV_SYNC->audio_drift
                     << ", video drift: " << AV_SYNC->video_drift
                     << ", delta: " << delta;

            if (delta > 200) // TODO
            {
                // 视频太快了，要减速
                qDebug() << __FILE__ << ":" << __LINE__ << "flv tag idx: " << current.yuv420p.get()->flv_tag_idx
                         << "video should be slower, delta: " << delta;

                current.duration += 50; // TODO
            }
            else if (delta < -200) // TODO
            {
                qDebug() << __FILE__ << ":" << __LINE__ << "flv tag idx: " << current.yuv420p.get()->flv_tag_idx
                         << "video should be faster, delta: " << delta;

                current.duration -= 50; // TODO
                if (current.duration < 0)
                {
                    current.duration = 0;
                }
            }
            else
            {
                qDebug() << __FILE__ << ":" << __LINE__ << "flv tag idx: " << current.yuv420p.get()->flv_tag_idx << ", delta: " << delta;
            }

            if (now >= current.play_time + current.duration)
            {
                yuv420p_ctx_vec_.pop_front();
                Yuv420pCtx& current = yuv420p_ctx_vec_.front();
                std::shared_ptr<Yuv420p> yuv420p = current.yuv420p;
                if (!yuv420p.get()->end)
                {
                    emit SIGNAL_CENTER->Yuv420pPlay(yuv420p);
                    current.play_time = AVSync::TimeNowMSec();
                    AV_SYNC->video_drift = yuv420p->pts - AVSync::TimeNowMSec();
                }
            }
        }
        break;
    }

    GLOBAL->file_parse_mutex.lock();

    if (--GLOBAL->yuv420p_count_in_queue <= Global::YUV420P_COUNT_TO_RESUME_PARSING)
    {
        GLOBAL->file_parse_cond.wakeAll();
    }

    GLOBAL->file_parse_mutex.unlock();
}
