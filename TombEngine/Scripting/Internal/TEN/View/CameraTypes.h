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
	COMBAT
	FIXED
	LOOK
	FLYBY

@section View.CameraType
*/

/*** Table of camera type constants (for use with GetCameraType() function).
@table CameraType
*/

static const std::unordered_map<std::string, CameraType> CAMERA_TYPE
{
	{ "CHASE", CameraType::Chase },
	{ "COMBAT", CameraType::Combat },
	{ "FIXED", CameraType::Fixed },
	{ "LOOK", CameraType::Look },
	{ "FLYBY", CameraType::Flyby }
};
