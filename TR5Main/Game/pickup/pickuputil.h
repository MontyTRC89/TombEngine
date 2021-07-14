#pragma once
enum GAME_OBJECT_ID : short;

// Given an array and an id, iterate through the array until we find
// an ID that matches the ID we've passed in.
template <typename T, size_t N> int GetArrSlot(std::array<T, N> const & arr, GAME_OBJECT_ID obj)
{
	for (int i = 0; i < arr.size(); ++i)
	{
		if (obj == arr[i].id)
		{
			return i;
		}
	}
	return -1;
}
