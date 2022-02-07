#pragma once

#include "ScriptAssert.h"
#include <string>

static const std::unordered_map<std::string, ERROR_MODE> kErrorModes {
	{"SILENT", ERROR_MODE::SILENT},
	{"WARN", ERROR_MODE::WARN},
	{"TERMINATE", ERROR_MODE::TERMINATE}
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
	ERROR_MODE ErrorMode;

	static void Register(sol::table & lua);
};

