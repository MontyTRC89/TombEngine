#pragma once
#include "Sound/sound.h"

/***
Constants for the type of the audio tracks.
@enum Misc.SoundTrackType
@pragma nostrip
*/

/*** Misc.SoundTrackType constants.

The following constants are inside SoundTrackType.

	ONESHOT
	LOOPED
	VOICE

@section Misc.SoundTrackType
*/

/*** Table of sound track type constants (for use with sound track functions).
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, SoundTrackType> SOUNDTRACK_TYPE
{
	{ "ONESHOT", SoundTrackType::OneShot },
	{ "LOOPED", SoundTrackType::BGM },
	{ "VOICE", SoundTrackType::Voice }
};
