#pragma once

#include "ScriptAssert.h"
#include <string>

static const std::unordered_map<std::string, ErrorMode> kErrorModes {
	{"SILENT", ErrorMode::Silent},
	{"WARN", ErrorMode::Warn},
	{"TERMINATE", ErrorMode::Terminate}
};

namespace sol {
	class state;
}

struct Settings
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
	ErrorMode ErrorMode;

	static void Register(sol::table & lua);
};

