#pragma once
#include "Specific/Input/Input.h"

#include <string>
#include <unordered_map>

using namespace TEN::Input;

/***
Constants for axis IDs.
@enum Input.AxisType
@pragma nostrip
*/

/*** Table of Input.AxisType constants.
To be used with @{Input.GetAxisDisplacement}.

	MOVE - Joystick move stick.
	CAMERA - Joystick look stick.
	MOUSE - Mouse displacement.

@table Input.AxisType
*/

static const auto AXIS_TYPE = std::unordered_map<std::string, InputAxisID>
{

	{ "MOVE", InputAxisID::Move },
	{ "CAMERA", InputAxisID::Camera },
	{ "MOUSE", InputAxisID::Mouse }
};