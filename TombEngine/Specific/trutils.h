#pragma once

namespace TEN::Utils
{
	// String utilities
	std::string ToUpper(std::string string);
	std::string ToLower(std::string string);
	std::string FromWchar(const wchar_t* string);
	std::wstring ToWString(const std::string& string);
	std::wstring FromChar(const char* string);
	std::vector<std::string> SplitString(const std::string& string);

	std::vector<unsigned short> GetProductOrFileVersion(bool productVersion);

	Vector2 Get2DScreenPosition(const Vector3& pos);
}
