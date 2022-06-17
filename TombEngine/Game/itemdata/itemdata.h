#pragma once
#include <variant>
#include <functional>
#include <cstddef>
#include <stdexcept>
#include "Game/itemdata/creature_info.h"
#include "Game/itemdata/door_data.h"
#include "Game/Lara/lara_struct.h"
#include "Objects/TR2/Vehicles/boat_info.h"
#include "Objects/TR2/Vehicles/skidoo_info.h"
#include "Objects/TR3/Vehicles/minecart_info.h"
#include "Objects/TR3/Vehicles/big_gun_info.h"
#include "Objects/TR3/Vehicles/kayak_info.h"
#include "Objects/TR3/Vehicles/quad_info.h"
#include "Objects/TR3/Vehicles/rubber_boat_info.h"
#include "Objects/TR3/Vehicles/upv_info.h"
#include "Objects/TR4/Vehicles/jeep_info.h"
#include "Objects/TR4/Vehicles/motorbike_info.h"
#include "Objects/TR4/Entity/tr4_wraith_info.h"
#include "Objects/TR5/Entity/tr5_laserhead_info.h"
#include "Objects/TR5/Object/tr5_pushableblock_info.h"
#include "Specific/phd_global.h"

template<class... Ts> struct visitor : Ts... { using Ts::operator()...; };
template<class... Ts> visitor(Ts...)->visitor<Ts...>; // line not needed in C++20...

using namespace TEN::Entities::TR4;
using namespace TEN::Entities::Vehicles;

struct ItemInfo;

class ITEM_DATA
{
	std::variant<std::nullptr_t,
		char,
		short,
		int,
		long,
		long long,
		unsigned char,
		unsigned short,
		unsigned int,
		unsigned long,
		unsigned long long,
		float,
		double,
		long double,
		std::array<short, 4>,
		ItemInfo*,
		CreatureInfo,
		LaserHeadInfo,
		QuadInfo,
		BigGunInfo,
		MotorbikeInfo,
		JeepInfo,
		LaraInfo*,
		KayakInfo,
		DOOR_DATA,
		SkidooInfo,
		UPVInfo,
		SpeedBoatInfo,
		GameVector,
		WraithInfo,
		RubberBoatInfo,
		PushableInfo,
		MinecartInfo
	> data;
	public:
	ITEM_DATA();

	template<typename D>
	ITEM_DATA(D&& type) : data(std::move(type)) {}

	// conversion operators to keep original syntax!
	// TODO: should be removed later and
	template<typename T>
	operator T* ()
	{
		if (std::holds_alternative<T>(data))
		{
			auto& ref = std::get<T>(data);
			return &ref;
		}

		throw std::runtime_error("ITEM_DATA does not hold the requested type!\n The code set the ITEM_DATA to a different type than the type that was attempted to read");
	}

	template<typename T>
	operator T& ()
	{
		if (std::holds_alternative<T>(data))
		{
			auto& ref = std::get<T>(data);
			return ref;
		}

		throw std::runtime_error("ITEM_DATA does not hold the requested type!\n The code set the ITEM_DATA to a different type than the type that was attempted to read");
	}

	/* Uncommented, we want to store pointers to global data, too (LaraInfo for example)
	template<typename T>
	ITEM_DATA& operator=(T* newData)
	{
		data = *newData;
		return *this;
	}
	*/

	ITEM_DATA& operator=(std::nullptr_t null)
	{
		data = nullptr;
		return *this;
	}

	template<typename T>
	ITEM_DATA& operator=(T& newData)
	{
		data = newData;
		return *this;
	}

	template<typename T>
	ITEM_DATA& operator=(T&& newData)
	{
		data = std::move(newData);
		return *this;
	}

	operator bool() const
	{
		return !std::holds_alternative<std::nullptr_t>(data);
	}

	template<typename ... Funcs>
	void apply(Funcs&&... funcs)
	{
		std::visit(
		visitor
			{
				[](auto const&) {},
				std::forward<Funcs>(funcs)...
			},
		data);
	}

	template<typename T>
	bool is() const
	{
		return std::holds_alternative<T>(data);
	}
};
