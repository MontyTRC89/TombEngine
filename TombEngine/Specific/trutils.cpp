#include "framework.h"
#include "Specific/trutils.h"

#include <codecvt>

namespace TEN::Utils
{
	std::string ToLower(std::string source)
	{
		std::transform(source.begin(), source.end(), source.begin(), [](unsigned char c) { return std::tolower(c); });
		return source;
	}

	std::string FromWchar(wchar_t* source)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
		return converter.to_bytes(std::wstring(source));
	}
}