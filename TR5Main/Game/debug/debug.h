#pragma once
#if _DEBUG
constexpr bool DebugBuild = true;
#else
constexpr bool DebugBuild = false;
#endif
#include <stdexcept>

inline void assertion(const bool& expr,const char* msg) {
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