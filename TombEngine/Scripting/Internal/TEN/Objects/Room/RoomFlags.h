#pragma once
#include <string>
#include <unordered_map>

#include "Game/room.h"

/***
Constants for room flag IDs.
@enum Objects.RoomFlagID
@pragma nostrip
*/

/*** Objects.RoomFlagID constants.

The following constants are inside RoomFlagID.

	WATER
	QUICKSAND
	SKYBOX
	WIND
	COLD
	DAMAGE
	NOLENSFLARE

@section Objects.RoomFlagID
*/

/*** Table of room flag ID constants (for use with Room:SetFlag / Room:GetFlag command).
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, RoomEnvFlags> TOOM_FLAG_IDS
{
	{ "WATER", RoomEnvFlags::ENV_FLAG_WATER },
	{ "QUICKSAND", RoomEnvFlags::ENV_FLAG_SWAMP },
	{ "SKYBOX", RoomEnvFlags::ENV_FLAG_OUTSIDE },
	{ "WIND", RoomEnvFlags::ENV_FLAG_WIND },
	{ "COLD", RoomEnvFlags::ENV_FLAG_COLD },
	{ "DAMAGE", RoomEnvFlags::ENV_FLAG_DAMAGE },
	{ "NOLENSFLARE", RoomEnvFlags::ENV_FLAG_NO_LENSFLARE }
};
