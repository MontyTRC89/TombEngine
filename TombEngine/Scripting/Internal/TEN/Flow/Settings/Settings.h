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
	ErrorMode ErrorMode;

	static void Register(sol::table & parent);
};

