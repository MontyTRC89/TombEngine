#pragma once
#if _DEBUG
constexpr bool DebugBuild = true;
#else
constexpr bool DebugBuild = false;
#endif
#include <stdexcept>
#include <string_view>

inline void assertion(const bool& expr,const char* msg) noexcept {
	if constexpr (DebugBuild) {
		if (!expr) throw std::runtime_error(msg);
	}
};
template <typename ...T>
inline void logD(const T&... x) {
	if constexpr (DebugBuild) {
		(std::cout << ... << x) << std::endl;
	}
};

template <typename ...T>
inline void logE(const T&... x) {
	if constexpr (DebugBuild) {
		(std::cerr << ... << x) << std::endl;
	}
};

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

void TENLog(std::string_view str, LogLevel level = LogLevel::Info, LogConfig config = LogConfig::Debug);
void ShutdownTENLog();
void InitTENLog();

class TENScriptException : public std::runtime_error
{
public:
	using std::runtime_error::runtime_error;
};