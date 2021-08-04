#include "framework.h"
#include "ScriptAssert.h"

static ERROR_MODE ScriptErrorMode = ERROR_MODE::WARN;

bool ScriptAssert(bool cond, std::string const& msg, std::optional<ERROR_MODE> forceMode)
{
	if (!cond)
	{
		ERROR_MODE mode = forceMode ? *forceMode : ScriptErrorMode;
		switch (mode)
		{
		case ERROR_MODE::WARN:
			TENLog(msg, LogLevel::Warning, LogConfig::All);
			break;
		case ERROR_MODE::TERMINATE:
			throw TENScriptException(msg);
			break;
		}
	}
	return cond;
}
