#pragma once

#include "Scripting/Internal/ScriptAssert.h"
#include <string>

static const std::unordered_map<std::string, ErrorMode> ERROR_MODES {
	{"SILENT", ErrorMode::Silent},
	{"WARN", ErrorMode::Warn},
	{"TERMINATE", ErrorMode::Terminate}
};

namespace sol {
	class state;
}

struct Settings
{
	ErrorMode ErrorMode = ErrorMode::Warn;
	bool FastReload = true;

	static void Register(sol::table & parent);
};

