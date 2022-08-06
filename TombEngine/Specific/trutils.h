#pragma once
#include <string>

namespace TEN::Utils
{
	std::string ToLower(std::string source);
	std::string FromWchar(wchar_t* source); 
	std::vector<std::string> SplitString(const std::string& source);

	std::vector<unsigned short> GetProductOrFileVersion(bool productVersion);
}