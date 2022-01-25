#include "frameworkandsol.h"
#include "ScriptAssert.h"

static ERROR_MODE ScriptErrorMode = ERROR_MODE::WARN;

void ScriptWarn(std::string const& msg)
{
	switch (ScriptErrorMode)
	{
	case ERROR_MODE::TERMINATE:
	case ERROR_MODE::WARN:
		TENLog(msg, LogLevel::Warning, LogConfig::All);
		break;
	}
}


bool ScriptAssert(bool cond, std::string const& msg, std::optional<ERROR_MODE> forceMode)
{
	if (!cond)
	{
		ERROR_MODE mode = forceMode ? *forceMode : ScriptErrorMode;
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


void SetScriptErrorMode(ERROR_MODE mode)
{
	ScriptErrorMode = mode;
}

ERROR_MODE GetScriptErrorMode()
{
	return ScriptErrorMode;
}

