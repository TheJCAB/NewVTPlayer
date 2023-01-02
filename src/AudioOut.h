#pragma once

#include "SoundMixer.h"
#include "Generator.h"

#include <span>

namespace VTPlayerLib
{

void PlayAudioSound(Generator<Fragment>&& sound);

}
// namespace VTPlayerLib
