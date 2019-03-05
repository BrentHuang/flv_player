#include "video_decoders.h"

VideoDecoders::VideoDecoders()
{
    Initialize();
}

VideoDecoders::~VideoDecoders()
{
    Finalize();
}

int VideoDecoders::Initialize()
{
    return h264_decoder_.Initialize();
}

void VideoDecoders::Finalize()
{
    return h264_decoder_.Finalize();;
}

void VideoDecoders::OnFlvH264TagReady(std::shared_ptr<flv::VideoTag> flv_h264_tag)
{
    h264_decoder_.OnFlvH264TagReady(flv_h264_tag);
}
