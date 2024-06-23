#pragma once
#if _DEBUG
constexpr bool DebugBuild = true;
#else
constexpr bool DebugBuild = false;
#endif

#include <stdexcept>
#include <string_view>
#include <iostream>

namespace TEN::Debug
{
	enum class LogLevel
	{
		Error,
		Warning,
		Info
	};

	enum class LogConfig
	{
		Debug,
		All
	};

	class TENScriptException : public std::runtime_error
	{
	public:
		using std::runtime_error::runtime_error;
	};

	void InitTENLog(const std::string& logDirContainingDir);
	void ShutdownTENLog();
	void TENLog(const std::string_view& string, LogLevel level = LogLevel::Info, LogConfig config = LogConfig::All, bool allowSpam = false);

	inline void TENAssert(const bool& cond, const char* msg)
	{
		if constexpr (DebugBuild)
		{
			if (!cond)
			{
				TENLog(msg, LogLevel::Error);
				throw std::runtime_error(msg);
			}
		}
	};
}
