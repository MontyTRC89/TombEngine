#include "framework.h"

#include <codecvt>
#include <filesystem>

#include "Renderer/Renderer11.h"
#include "Renderer/Renderer11Enums.h"
#include "Specific/trutils.h"

using TEN::Renderer::g_Renderer;

namespace TEN::Utils
{
	std::string ConstructAssetDirectory(std::string customDirectory)
	{
		static const int searchDepth = 2;
		static const std::string upDir = "../";
		static const std::string testPath = "Scripts/Gameflow.lua";

		if (!customDirectory.empty())
		{
			// Replace all backslashes with forward slashes.
			std::replace(customDirectory.begin(), customDirectory.end(), '\\', '/');

			// Add trailing slash if missing.
			if (customDirectory.back() != '/')
				customDirectory += '/';
		}

		// Wrap directory depth searching into try-catch block to avoid crashes if we get too
		// shallow directory level (e.g. if user have placed executable in a disk root folder).

		try
		{
			// First, search custom directory, if exists, only then try own (empty) subdirectory.

			for (int useCustomSubdirectory = 1; useCustomSubdirectory >= 0; useCustomSubdirectory--)
			{
				// Quickly exit if no custom directory specified.

				if (useCustomSubdirectory && customDirectory.empty())
					continue;

				for (int depth = 0; depth < searchDepth + 1; depth++)
				{
					auto result = useCustomSubdirectory ? customDirectory : std::string{};
					bool isAbsolute = useCustomSubdirectory && std::filesystem::path(result).is_absolute();

					if (isAbsolute)
					{
						// Custom directory may be specified as absolute. In such case, it makes no sense
						// to search for assets on extra depth levels, since user never would expect that.

						if (depth > 0)
							break;
					}
					else
					{
						// Add upward directory levels, according to current depth.

						for (int level = 0; level < depth; level++)
							result = upDir + result;
					}

					// Look if provided test path / file exists in current folder. If it is,
					// it means this is a valid asset folder.

					auto testDir = result + (useCustomSubdirectory ? "/" : "") + testPath;
					if (std::filesystem::is_regular_file(testDir))
						return result;
				}
			}
		}
		catch (std::exception ex)
		{
			return std::string{}; // Use exe path if any error is encountered.
		}

		return std::string{}; // Use exe path if no any assets were found.
	}

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

	std::string ToString(const std::wstring& wString)
	{
		return ToString(wString.c_str());
	}

	std::string ToString(const wchar_t* wString)
	{
        auto converter = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>();
		return converter.to_bytes(std::wstring(wString));
	}

    std::wstring ToWString(const std::string& string)
    {
        auto cString = string.c_str();
        int size = MultiByteToWideChar(CP_UTF8, 0, cString, (int)string.size(), nullptr, 0);
        auto wString = std::wstring(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, cString, (int)strlen(cString), &wString[0], size);
        return wString;
    }

	std::wstring ToWString(const char* cString)
	{
		wchar_t buffer[UCHAR_MAX];
		std::mbstowcs(buffer, cString, UCHAR_MAX);
		return std::wstring(buffer);
	}

	std::string ReplaceNewLineSymbols(const std::string& string)
	{
		auto result = string;
		std::string::size_type index = 0;

		while ((index = result.find("\\n", index)) != std::string::npos) 
		{
			result.replace(index, 2, "\n");
			++index;
		}

		return result;
	}

	std::vector<std::string> SplitString(const std::string& string)
	{
		auto strings = std::vector<std::string>{};

		// Exit early if string is single line.
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

    Vector2 GetAspectCorrect2DPosition(Vector2 pos2D)
    {
       constexpr auto SCREEN_SPACE_ASPECT_RATIO = SCREEN_SPACE_RES.x / SCREEN_SPACE_RES.y;

        auto screenRes = g_Renderer.GetScreenResolution().ToVector2();
        float screenResAspectRatio = screenRes.x / screenRes.y;
        float aspectRatioDelta = screenResAspectRatio - SCREEN_SPACE_ASPECT_RATIO;

        if (aspectRatioDelta > EPSILON)
        {
            pos2D.x *= 1.0f - (aspectRatioDelta / 2);
        }
        else if (aspectRatioDelta < -EPSILON)
        {
            pos2D.y *= 1.0f - (aspectRatioDelta / 2);
        }

        return pos2D;
    }

    Vector2 Convert2DPositionToNDC(const Vector2& pos2D)
    {
        return Vector2(
            ((pos2D.x * 2) / SCREEN_SPACE_RES.x) - 1.0f,
            1.0f - ((pos2D.y * 2) / SCREEN_SPACE_RES.y));
    }

    Vector2 ConvertNDCTo2DPosition(const Vector2& ndc)
    {
        return Vector2(
            ((ndc.x + 1.0f) * SCREEN_SPACE_RES.x) / 2,
            ((1.0f - ndc.y) * SCREEN_SPACE_RES.y) / 2);
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

		if (size == 0)
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
