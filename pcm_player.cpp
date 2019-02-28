#include "pcm_player.h"

PcmPlayer::PcmPlayer()
{

}

PcmPlayer::~PcmPlayer()
{

}

void PcmPlayer::OnPcmReady(std::shared_ptr<Pcm> pcm)
{
    pcm_device_.OnPcmReady(pcm);
}
