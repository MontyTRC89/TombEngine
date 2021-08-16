#pragma once

#include "GameScriptSettings.h"
#include <string>

namespace sol {
	class state;
}

struct GameScriptSettings
{
	int ScreenWidth;
	int ScreenHeight;
	bool EnableLoadSave;
	bool EnableDynamicShadows;
	bool EnableWaterCaustics;
	bool Windowed;
	int DrawingDistance;
	bool ShowRendererSteps;
	bool ShowDebugInfo;
	std::string ErrorMode;

	static void Register(sol::state* lua);
};

