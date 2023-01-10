#pragma once
#include "Game/camera.h"

/***
Constants for action key IDs.
@enum Misc.ActionID
@pragma nostrip
*/

/*** Misc.ActionID constants.

The following constants are inside CameraType.

	CHASE
	FIXED
	LOOK
	COMBAT
	HEAVY
	OBJECT

@section Misc.CameraType
*/

/*** Table of camera type constants (for use with GetCameraType() function).
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, CameraType> kCameraType
{
	{"Chase", CameraType::Chase},
	{"Fixed", CameraType::Fixed},
	{"Look", CameraType::Look},
	{"Combat", CameraType::Combat},
	{"Heavy", CameraType::Heavy},
	{"Object", CameraType::Object},
};
