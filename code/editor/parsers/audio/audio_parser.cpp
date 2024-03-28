#include "wav/wav_audio_parser.h"
#include "mp3/mp3_audio_parser.h"
#include "ogg/ogg_audio_parser.h"

audio_parser* Audio_Parser_Load(allocator* Allocator, string File);

#include "wav/wav_audio_parser.cpp"
#include "mp3/mp3_audio_parser.cpp"
#include "ogg/ogg_audio_parser.cpp"