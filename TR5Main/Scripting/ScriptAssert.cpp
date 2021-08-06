#include "framework.h"
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
			TENLog(msg, LogLevel::Warning, LogConfig::All);
			break;
		case ERROR_MODE::TERMINATE:
			throw TENScriptException(msg);
			break;
		}
	}
	return cond;
}

void SetErrorMode(std::string const& mode)
{
	std::string noCase{ mode };
	std::transform(std::cbegin(noCase), std::cend(noCase), std::begin(noCase), [](unsigned char c) {return std::tolower(c); });
	if (noCase == "silent")
	{
		ScriptErrorMode = ERROR_MODE::SILENT;
	}
	else if (noCase == "warn")
	{
		ScriptErrorMode = ERROR_MODE::WARN;
	}
	else if (noCase == "terminate")
	{
		ScriptErrorMode = ERROR_MODE::TERMINATE;
	}
	else
	{
		TENLog("Wrong error mode set - valid settings are \"silent\", \"warn\" and \"terminate\"; defaulting to \"warn\".", LogLevel::Warning);
	}
}
