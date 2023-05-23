#include "framework.h"
#include "Specific/trutils.h"

#include <codecvt>
#include <filesystem>

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
					if (std::filesystem::exists(testDir))
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

	std::string ToString(const std::wstring& string)
	{
		return ToString(string.c_str());
	}

	std::string ToString(const wchar_t* string)
	{
		auto converter = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>();
		return converter.to_bytes(std::wstring(string));
	}

	std::wstring ToWString(const std::string& string)
	{
		auto cString = string.c_str();
		int size = MultiByteToWideChar(CP_UTF8, 0, cString, (int)string.size(), nullptr, 0);
		auto wString = std::wstring(size, 0);
		MultiByteToWideChar(CP_UTF8, 0, cString, (int)strlen(cString), &wString[0], size);
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
