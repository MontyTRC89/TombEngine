#pragma once

namespace TEN::Utils
{
	// String utilities
	std::string ToUpper(std::string string);
	std::string ToLower(std::string string);
	std::string ToString(const wchar_t* wString);
	std::wstring ToWString(const char* cString);
	std::wstring ToWString(const std::string& string);
	std::vector<std::string> SplitString(const std::string& string);

	// Screen space utilities
	Vector2 GetAspectCorrectScreenSpacePos(const Vector2& pos);
	Vector2 ConvertScreenSpacePosToNDC(const Vector2& pos);
	Vector2 ConvertNDCToScreenSpacePos(const Vector2& pos);

	std::vector<unsigned short> GetProductOrFileVersion(bool productVersion);
}
