#pragma once

namespace TEN::Utils
{
	// String utilities
	std::string ToUpper(std::string string);
	std::string ToLower(std::string string);
	std::string ToString(const wchar_t* string);
	std::wstring ToWString(const std::string& string);
	std::wstring ToWString(const char* string);
	std::vector<std::string> SplitString(const std::string& string);

	std::vector<unsigned short> GetProductOrFileVersion(bool productVersion);

	template<typename T>
	std::vector<T> RemoveVectorDuplicates(const std::vector<T>& vector)
	{
		auto seenSet = std::unordered_set<T>{};

		auto newVector = std::vector<T>{};
		newVector.reserve(vector.size());

		for (const auto& item : vector)
		{
			if (seenSet.find(item) == seenSet.end())
			{
				seenSet.insert(item);
				newVector.push_back(item);
			}
		}

		return newVector;
	}
}
