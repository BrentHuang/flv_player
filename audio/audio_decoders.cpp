#include "audio_decoders.h"

AudioDecoders::AudioDecoders()
{
    Initialize();
}

AudioDecoders::~AudioDecoders()
{
    Finalize();
}

int AudioDecoders::Initialize()
{
    return aac_decoder_.Initialize();
}

void AudioDecoders::Finalize()
{
    return aac_decoder_.Finalize();
}

void AudioDecoders::OnFlvAacTagReady(std::shared_ptr<flv::AudioTag> flv_aac_tag)
{
    aac_decoder_.OnFlvAacTagReady(flv_aac_tag);
}
