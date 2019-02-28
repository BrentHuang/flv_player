#include "signal_center.h"

SignalCenter::SignalCenter()
{
    qRegisterMetaType<std::shared_ptr<flv::VideoTag>>("std::shared_ptr<flv::VideoTag>");
    qRegisterMetaType<std::shared_ptr<Yuv420p>> ("std::shared_ptr<Yuv420p>");
    qRegisterMetaType<std::shared_ptr<flv::AudioTag>> ("std::shared_ptr<flv::AudioTag>");
    qRegisterMetaType<std::shared_ptr<Pcm>> ("std::shared_ptr<Pcm>");
}

SignalCenter::~SignalCenter()
{

}

