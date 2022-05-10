#include "framework.h"
#include "ScriptAssert.h"

static ErrorMode ScriptErrorMode = ErrorMode::Warn;

void ScriptWarn(std::string const& msg)
{
	switch (ScriptErrorMode)
	{
	case ErrorMode::Terminate:
	case ErrorMode::Warn:
		TENLog(msg, LogLevel::Warning, LogConfig::All);
		break;
	}
}


bool ScriptAssert(bool cond, std::string const& msg, std::optional<ErrorMode> forceMode)
{
	if (!cond)
	{
		ErrorMode mode = forceMode ? *forceMode : ScriptErrorMode;
		switch (mode)
		{
		case ErrorMode::Warn:
			TENLog(msg, LogLevel::Error, LogConfig::All);
			break;
		case ErrorMode::Terminate:
			TENLog(msg, LogLevel::Error, LogConfig::All);
			throw TENScriptException(msg);
			break;
		}
	}
	return cond;
}


void SetScriptErrorMode(ErrorMode mode)
{
	ScriptErrorMode = mode;
}

ErrorMode GetScriptErrorMode()
{
	return ScriptErrorMode;
}

