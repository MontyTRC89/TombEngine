#pragma once

#include <string>
#include <optional>

enum class ERROR_MODE
{
	SILENT,
	WARN,
	TERMINATE
};

void SetScriptErrorMode(ERROR_MODE mode);
ERROR_MODE GetScriptErrorMode();
void ScriptWarn(std::string const& msg);

bool ScriptAssert(bool cond, std::string const& msg, std::optional<ERROR_MODE> forceMode = std::nullopt);

