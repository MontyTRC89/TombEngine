#include "framework.h"
#include "Specific/trutils.h"

#include <codecvt>

#include "Renderer/Renderer11.h"

using TEN::Renderer::g_Renderer;

namespace TEN::Utils
{
	std::string ToUpper(std::string string)
	{
		std::transform(string.begin(), string.end(), string.begin(), [](unsigned char c) { return std::toupper(c); });
		return string;
	}
    
	std::string ToLower(std::string string)
	{
		std::transform(string.begin(), string.end(), string.begin(), [](unsigned char c) { return std::tolower(c); });
		return string;
	}

	std::string ToString(const wchar_t* source)
	{
        auto converter = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>();
		return converter.to_bytes(std::wstring(source));
	}

    std::wstring ToWString(const std::string& string)
    {
        auto cString = string.c_str();
        int size = MultiByteToWideChar(CP_UTF8, 0, cString, string.size(), nullptr, 0);
        auto wString = std::wstring(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, cString, strlen(cString), &wString[0], size);
        return wString;
    }

    std::wstring ToWString(const char* source)
    {
        wchar_t buffer[UCHAR_MAX];
        std::mbstowcs(buffer, source, UCHAR_MAX);
        return std::wstring(buffer);
    }

	std::vector<std::string> SplitString(const std::string& string)
	{
        auto strings = std::vector<std::string>{};

		// String is single line; exit early.
		if (string.find('\n') == std::string::npos)
		{
			strings.push_back(string);
			return strings;
		}

		std::string::size_type pos = 0;
		std::string::size_type prev = 0;
		while ((pos = string.find('\n', prev)) != std::string::npos)
		{
			strings.push_back(string.substr(prev, pos - prev));
			prev = pos + 1;
		}

		strings.push_back(string.substr(prev));
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

        // Load version info.
        if (!GetFileVersionInfoA(fileName, 0, size, buffer.get()))
        {
            TENLog("GetFileVersionInfoA failed", LogLevel::Error);
            return {};
        }

        VS_FIXEDFILEINFO* info;
        unsigned int infoSize;

        if (!VerQueryValueA(buffer.get(), "\\", (void**)&info, &infoSize))
        {
            TENLog("VerQueryValueA failed", LogLevel::Error);
            return {};
        }

        if (infoSize != sizeof(VS_FIXEDFILEINFO))
        {
            TENLog("VerQueryValueA returned wrong size for VS_FIXEDFILEINFO", LogLevel::Error);
            return {};
        }

        if (productVersion)
        {
            return
            {
                HIWORD(info->dwProductVersionMS),
                LOWORD(info->dwProductVersionMS),
                HIWORD(info->dwProductVersionLS),
                LOWORD(info->dwProductVersionLS)
            };
        }
        else
        {
            return
            {
                HIWORD(info->dwFileVersionMS),
                LOWORD(info->dwFileVersionMS),
                HIWORD(info->dwFileVersionLS),
                LOWORD(info->dwFileVersionLS)
            };
        }
    }
}
