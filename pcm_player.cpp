#include "pcm_player.h"
#include <QDebug>

PcmPlayer::PcmPlayer() : audio_output_(), pcm_device_()
{
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo device_info = QAudioDeviceInfo::defaultOutputDevice();

    if (!device_info.isFormatSupported(format))
    {
        qWarning() << "default format not supported - trying to use nearest";
        format = device_info.nearestFormat(format);
    }

    audio_output_.reset(new QAudioOutput(device_info, format));
    audio_output_.get()->start(&pcm_device_);
}

PcmPlayer::~PcmPlayer()
{
    if (audio_output_ != nullptr)
    {
        audio_output_.get()->stop();
    }
}

void PcmPlayer::OnPcmReady(std::shared_ptr<Pcm> pcm)
{
    // 下列打印的数据显示，每个pcm大小为4096字节，播放时长为23毫秒
//    qDebug() << "pcm size: " << pcm.get()->data.size() << ", pts: " << pcm.get()->pts;
    pcm_device_.OnPcmReady(pcm);
}

// TODO 播放完后为什么CPU还很高？后续调研一下ffplay和qtav中文件解析与音视频播放进度是怎么同步的
