#include "framework.h"
#include "Game/pickup/pickup_key_items.h"

#include "Game/Lara/lara_struct.h"
#include "Game/pickup/pickup_misc_items.h"
#include "Objects/objectslist.h"

template <size_t N> struct KeyPickupInfo
{
	// the array in the LaraInfo struct that holds the amount of
	// each type of key item (puzzle, key, examine, combos, etc)
	int(LaraInfo::* arr)[N];
	// The range of GAME_OBJECT_IDs that correspond to each type
	// of key item
	// ID of first and last objects of this type
	GAME_OBJECT_ID min;
	GAME_OBJECT_ID max;
	constexpr KeyPickupInfo<N>(int(LaraInfo::* a_arr)[N], GAME_OBJECT_ID a_min, GAME_OBJECT_ID a_max) : arr{ a_arr }, min{ a_min }, max{ a_max } {};
};

static constexpr int kDefaultPickupAmt = 1;

// This has to be a tuple since the pointers-to-arrays all have different types
// due to differing array sizes.
static constexpr std::tuple kKeyPickupInfos = std::make_tuple( 
	KeyPickupInfo{&LaraInfo::Puzzles, ID_PUZZLE_ITEM1, ID_PUZZLE_ITEM16},
	KeyPickupInfo{&LaraInfo::PuzzlesCombo, ID_PUZZLE_ITEM1_COMBO1, ID_PUZZLE_ITEM16_COMBO2},
	KeyPickupInfo{&LaraInfo::Keys, ID_KEY_ITEM1, ID_KEY_ITEM16},
	KeyPickupInfo{&LaraInfo::KeysCombo, ID_KEY_ITEM1_COMBO1, ID_KEY_ITEM16_COMBO2},
	KeyPickupInfo{&LaraInfo::Pickups, ID_PICKUP_ITEM1, ID_PICKUP_ITEM16},
	KeyPickupInfo{&LaraInfo::PickupsCombo, ID_PICKUP_ITEM1_COMBO1, ID_PICKUP_ITEM16_COMBO2},
	KeyPickupInfo{&LaraInfo::Examines, ID_EXAMINE1, ID_EXAMINE8},
	KeyPickupInfo{&LaraInfo::ExaminesCombo, ID_EXAMINE1_COMBO1, ID_EXAMINE8_COMBO2}
 );
static constexpr auto nInfos = std::tuple_size<decltype(kKeyPickupInfos)>{};

// Test against min and max of kKeyPickupInfos[N].
// if found, return the pointer to the LaraInfo array
// as well as the position within that array.
template<size_t N> static constexpr std::pair<int *, size_t> TestAgainstRange(LaraInfo & info, GAME_OBJECT_ID obj)
{
	auto pickupInfo = std::get<N>(kKeyPickupInfos);
	int min = pickupInfo.min;
	int max = pickupInfo.max;
	if (obj >= min && obj <= max)
	{
		return std::make_pair(info.*(pickupInfo.arr), obj - min);
	}
	return std::make_pair(nullptr, 0);
}

// Test against each kKeyPickupInfos item.
// This is recursive because I couldn't find a simpler
// way to iterate across a tuple.
template<size_t N> static std::pair<int *, size_t> GetArrInternal(LaraInfo & info, GAME_OBJECT_ID obj)
{
	auto h = TestAgainstRange<N>(info, obj);
	return h.first ? h : GetArrInternal<N-1>(info, obj);
}

// Base case for recursion
template<> static std::pair<int *, size_t> GetArrInternal<0>(LaraInfo & info, GAME_OBJECT_ID obj)
{
	return TestAgainstRange<0>(info, obj);
}

static bool TryModifyKeyItem(LaraInfo & info, GAME_OBJECT_ID obj, int amt, bool add)
{
	// kick off the recursion starting at the last element
	auto result = GetArrInternal<nInfos - 1>(info, obj);
	if (result.first) {
		auto defaultModify = add ? kDefaultPickupAmt : -kDefaultPickupAmt;
		auto newVal = int{result.first[result.second]} + (amt ? amt : defaultModify);
		result.first[result.second] = std::max(0, newVal);
		return true;
	}
	return false;
}

bool TryAddKeyItem(LaraInfo & info, GAME_OBJECT_ID obj, int count)
{
	return TryModifyKeyItem(info, obj, count, true);
}

bool TryRemoveKeyItem(LaraInfo & info, GAME_OBJECT_ID obj, int count)
{
	return TryModifyKeyItem(info, obj, -count, false);
}

std::optional<int> GetKeyItemCount(LaraInfo & info, GAME_OBJECT_ID obj)
{
	// kick off the recursion starting at the last element
	auto result = GetArrInternal<nInfos - 1>(info, obj);
	if (result.first) {
		return result.first[result.second];
	}
	return std::nullopt;
}


