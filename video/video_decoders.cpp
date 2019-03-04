#include "video_decoders.h"

VideoDecoders::VideoDecoders()
{

}

VideoDecoders::~VideoDecoders()
{

}

void VideoDecoders::OnFlvH264TagReady(std::shared_ptr<flv::VideoTag> flv_h264_tag)
{
    openh264_decoder_.OnFlvH264TagReady(flv_h264_tag);
}
