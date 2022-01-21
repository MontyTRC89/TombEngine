#pragma once

#include <string>
#include <optional>
#include <spdlog/fmt/fmt.h>

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

template <typename ... Ts> bool ScriptAssertF(bool cond, std::string_view str, Ts...args)
{
	if (!cond)
	{
		auto msg = fmt::format(str, args...);
		auto mode = GetScriptErrorMode();
		switch (mode)
		{
		case ERROR_MODE::WARN:
			TENLog(msg, LogLevel::Error, LogConfig::All);
			break;
		case ERROR_MODE::TERMINATE:
			TENLog(msg, LogLevel::Error, LogConfig::All);
			throw TENScriptException(msg);
			break;
		}
	}
	return cond;
}

template <typename ... Ts> bool ScriptAssertTerminateF(bool cond, std::string_view str, Ts...args)
{
	if (!cond)
	{
		auto msg = fmt::format(str, args...);
		TENLog(msg, LogLevel::Error, LogConfig::All);
		throw TENScriptException(msg);
	}
	return cond;
}

