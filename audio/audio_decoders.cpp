#include "audio_decoders.h"

AudioDecoders::AudioDecoders()
{

}

AudioDecoders::~AudioDecoders()
{

}

void AudioDecoders::OnFlvAacTagReady(std::shared_ptr<flv::AudioTag> flv_aac_tag)
{
    fdkaac_decoder_.OnFlvAacTagReady(flv_aac_tag);
}
