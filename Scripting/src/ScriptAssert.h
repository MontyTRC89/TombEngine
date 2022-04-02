#pragma once

#include <string>
#include <optional>
#include <spdlog/fmt/fmt.h>

enum class ErrorMode
{
	Silent,
	Warn,
	Terminate
};

void SetScriptErrorMode(ErrorMode mode);
ErrorMode GetScriptErrorMode();

void ScriptWarn(std::string const& msg);

bool ScriptAssert(bool cond, std::string const& msg, std::optional<ErrorMode> forceMode = std::nullopt);

template <typename ... Ts> bool ScriptAssertF(bool cond, std::string_view str, Ts...args)
{
	if (!cond)
	{
		auto msg = fmt::format(str, args...);
		switch (GetScriptErrorMode())
		{
		case ErrorMode::Warn:
			TENLog(msg, LogLevel::Error, LogConfig::All);
			break;
		case ErrorMode::Terminate:
			TENLog(msg, LogLevel::Error, LogConfig::All);
			throw TENScriptException(msg);
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

