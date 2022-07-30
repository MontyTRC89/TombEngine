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

	std::vector<std::string> SplitString(const std::string& source)
	{
		std::vector<std::string> strings;

		// String is single-line, early exit
		if (source.find('\n') == std::string::npos)
		{
			strings.push_back(source);
			return strings;
		}

		std::string::size_type pos = 0;
		std::string::size_type prev = 0;
		while ((pos = source.find('\n', prev)) != std::string::npos)
		{
			strings.push_back(source.substr(prev, pos - prev));
			prev = pos + 1;
		}

		strings.push_back(source.substr(prev));

		return strings;
	}
}