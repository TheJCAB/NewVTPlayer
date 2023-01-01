#pragma once

#include "Song.h"
#include "Stream.h"

namespace VTPlayerLib
{

typedef std::shared_ptr<ModSong> LoadSongFunc(IStream&);

LoadSongFunc LoadMod;
LoadSongFunc LoadS3M;
LoadSongFunc LoadXM;
LoadSongFunc LoadUnknown;


bool ParseModEffect(uint8_t effect, uint8_t XY, ModSong::GlobalCommand& globalCommand, ModSong::ChannelCommand& command);
bool ParseExtendedModEffect(uint8_t X, uint8_t Y, ModSong::GlobalCommand& globalCommand, ModSong::ChannelCommand& command);

}
// namespace VTPlayerLib
