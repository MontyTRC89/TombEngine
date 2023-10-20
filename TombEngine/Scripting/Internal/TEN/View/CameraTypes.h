#pragma once
#include "Game/camera.h"

/***
Constants for the type of the Camera.
@enum View.CameraType
@pragma nostrip
*/

/*** View.CameraType constants.

The following constants are inside CameraType.

	CHASE
	FIXED
	LOOK
	COMBAT
	HEAVY
	OBJECT

@section View.CameraType
*/

/*** Table of camera type constants (for use with GetCameraType() function).
@table CONSTANT_STRING_HERE
*/

static const std::unordered_map<std::string, CameraType> CAMERA_TYPE
{
	{ "CHASE", CameraType::Chase },
	{ "FIXED", CameraType::Fixed },
	{ "LOOK", CameraType::Look },
	{ "COMBAT", CameraType::Combat },
	{ "HEAVY", CameraType::Heavy },
	{ "OBJECT", CameraType::Object }
};
