#pragma once
#include <string>
#include <unordered_map>

#include "Sound/sound.h"

/***
Constants for room reverb types.
@enum Objects.RoomReverb
@pragma nostrip
*/

/*** Objects.RoomReverb constants.

The following constants are inside RoomReverb.

	OUTSIDE
	SMALL
	MEDIUM
	LARGE
	PIPE

@section Objects.RoomReverb
*/

/*** Table of room reverb constants (for use with Room:SetReverb / Room:GetReverb command).
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, ReverbType> kRoomReverbTypes
{
	{"OUTSIDE", ReverbType::Outside},
	{"SMALL", ReverbType::Small},
	{"MEDIUM", ReverbType::Medium},
	{"LARGE", ReverbType::Large},
	{"PIPE", ReverbType::Pipe}
};
