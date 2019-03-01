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
    pcm_device_.OnPcmReady(pcm);
}

// TODO 播放完后为什么cpu还很高？
