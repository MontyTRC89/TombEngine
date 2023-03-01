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

	// 2D space utilities
	Vector2 GetAspectCorrect2DPosition(Vector2 pos2D);
	Vector2 Convert2DPositionToNDC(const Vector2& pos2D);
	Vector2 ConvertNDCTo2DPosition(const Vector2& pos2D);

	std::vector<unsigned short> GetProductOrFileVersion(bool productVersion);
}
