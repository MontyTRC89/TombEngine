#pragma once

namespace TEN::Utils
{
	// String utilities

	std::string ConstructAssetDirectory(std::string customDirectory);
	std::string ReplaceNewLineSymbols(const std::string& string);
	std::string ToUpper(std::string string);
	std::string ToLower(std::string string);
	std::string ToString(const std::wstring& wString);
	std::string ToString(const wchar_t* wString);
	std::wstring ToWString(const std::string& string);
	std::wstring ToWString(const char* cString);
	std::vector<std::wstring> SplitString(const std::wstring& string);
	int GetHash(const std::string& string);

	// 2D space utilities

	std::optional<Vector2> Get2DPosition(const Vector3& pos);
	Vector2				   GetAspectCorrect2DPosition(const Vector2& pos);
	Vector2				   Convert2DPositionToNDC(const Vector2& pos);
	Vector2				   ConvertNDCTo2DPosition(const Vector2& ndc);

	std::vector<unsigned short> GetProductOrFileVersion(bool productVersion);

	template<typename T>
	std::vector<T> RemoveDuplicates(const std::vector<T>& vector)
	{
		auto uniqueElements = std::unordered_set<T>{};
		auto newVector = std::vector<T>{};

		for (const auto& element : vector)
		{
			if (uniqueElements.find(element) == uniqueElements.end())
			{
				uniqueElements.insert(element);
				newVector.push_back(element);
			}
		}

		return newVector;
	}

	template <typename T>
	bool Contains(const std::vector<T>& vector, const T& element)
	{
		auto it = std::find(vector.begin(), vector.end(), element);
		return (it != vector.end());
	}

	template <typename T>
	bool Contains(const std::set<T>& set, const T& element)
	{
		auto it = std::find(set.begin(), set.end(), element);
		return (it != set.end());
	}
}
