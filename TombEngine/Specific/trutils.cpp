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

    std::vector<unsigned short> GetProductOrFileVersion(bool productVersion)
    {
        char fileName[UCHAR_MAX] = {};

        if (!GetModuleFileNameA(nullptr, fileName, UCHAR_MAX))
        {
            TENLog("Can't get current assembly filename", LogLevel::Error);
            return {};
        }

        int size = GetFileVersionInfoSizeA(fileName, NULL);

        if (!size)
        {
            TENLog("GetFileVersionInfoSizeA failed", LogLevel::Error);
            return {};
        }
        std::unique_ptr<unsigned char> buffer(new unsigned char[size]);

        // Load the version info
        if (!GetFileVersionInfoA(fileName, 0, size, buffer.get()))
        {
            TENLog("GetFileVersionInfoA failed", LogLevel::Error);
            return {};
        }

        VS_FIXEDFILEINFO* info;
        unsigned int info_size;

        if (!VerQueryValueA(buffer.get(), "\\", (void**)&info, &info_size))
        {
            TENLog("VerQueryValueA failed", LogLevel::Error);
            return {};
        }

        if (info_size != sizeof(VS_FIXEDFILEINFO))
        {
            TENLog("VerQueryValueA returned wrong size for VS_FIXEDFILEINFO", LogLevel::Error);
            return {};
        }

        if (productVersion)
            return
            {
                HIWORD(info->dwProductVersionMS),
                LOWORD(info->dwProductVersionMS),
                HIWORD(info->dwProductVersionLS),
                LOWORD(info->dwProductVersionLS)
            };
        else
            return
            {
                HIWORD(info->dwFileVersionMS),
                LOWORD(info->dwFileVersionMS),
                HIWORD(info->dwFileVersionLS),
                LOWORD(info->dwFileVersionLS)
            };
    }
}