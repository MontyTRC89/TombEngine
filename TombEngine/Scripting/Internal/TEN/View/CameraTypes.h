#pragma once
#include "Game/camera.h"

/***
Constants for the type of the Camera.
@enum View.CameraType
@pragma nostrip
*/

/*** Table of View.CameraType constants. To be used with @{View.GetCameraType} function.
@table CameraType

 - `NORMAL` - standard in-game camera when weapons are holstered.
 - `COMBAT` - in-game camera when weapons are unholstered.
 - `FIXED` - classic fixed camera.
 - `LOOK` - look camera.
 - `FLYBY` - flyby or tracking camera.
 - `BINOCULARS` - binoculars is active.
 - `LASERSIGHT` - lasersight is active.
*/

static const std::unordered_map<std::string, CameraType> CAMERA_TYPE
{
	{ "CHASE",		CameraType::Chase		}, // DEPRECATED
	{ "NORMAL",		CameraType::Chase		},
	{ "COMBAT",		CameraType::Combat		},
	{ "FIXED",		CameraType::Fixed		},
	{ "HEAVY",		CameraType::Fixed		}, // DEPRECATED
	{ "LOOK",		CameraType::Look		},
	{ "FLYBY",		CameraType::Flyby		},
	{ "BINOCULARS", CameraType::Binoculars	},
	{ "LASERSIGHT", CameraType::Lasersight	}
};
