#pragma once

#include "Scripting/Internal/ScriptAssert.h"

namespace sol { class state; }

static const std::unordered_map<std::string, ErrorMode> ERROR_MODES
{
	{ "SILENT", ErrorMode::Silent },
	{ "WARN", ErrorMode::Warn },
	{ "TERMINATE", ErrorMode::Terminate }
};

struct Settings
{
	ErrorMode ErrorMode	 = ErrorMode::Warn;
	bool	  FastReload = true;

	static void Register(sol::table& parent);
};
