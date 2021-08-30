#pragma once
#include <variant>
#include <functional>
#include <cstddef>
#include "upv_info.h"
#include "skidoo_info.h"
#include "door_data.h"
#include "kayak_info.h"
#include "lara_struct.h"
#include "jeep_info.h"
#include "motorbike_info.h"
#include "minecart_info.h"
#include "biggun_info.h"
#include "quad_info.h"
#include "tr5_laserhead_info.h"
#include "creature_info.h"
#include "boat_info.h"
#include "rubberboat_info.h"
#include <stdexcept>
#include "phd_global.h"
#include "tr4_wraith_info.h"
#include "tr5_pushableblock_info.h"

template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...)->overload<Ts...>; // line not needed in C++20...

struct ITEM_INFO;

//Type Wrapper to construct a ITEM_DATA
template<typename D>
struct ITEM_DATA_TYPE {
	using type = D;
};
class ITEM_DATA {
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
		ITEM_INFO*,
		CREATURE_INFO,
		LASER_HEAD_INFO,
		QUAD_INFO,
		BIGGUNINFO,
		MOTORBIKE_INFO,
		JEEP_INFO,
		LaraInfo,
		KAYAK_INFO,
		DOOR_DATA,
		SKIDOO_INFO,
		SUB_INFO,
		BOAT_INFO,
		GAME_VECTOR,
		WRAITH_INFO,
		RUBBER_BOAT_INFO,
		PUSHABLE_INFO,
		CART_INFO
	> data;
	public:
	ITEM_DATA();
	//we have to use a wrapper for a type, because the compiler needs to distinguish different overloads
	template<typename D>
	ITEM_DATA(ITEM_DATA_TYPE<D> type) : data(ITEM_DATA_TYPE<D>::type{}) {}

	template<typename D>
	ITEM_DATA(D&& type) : data(std::move(type)) {}


	// conversion operators to keep original syntax!
	// TODO: should be removed later and
	template<typename T>
	operator T* () {
		if(std::holds_alternative<T>(data)) {
			auto& ref = std::get<T>(data);
			return &ref;
		}
		throw std::runtime_error("ITEM_DATA does not hold the requested type!\n The code set the ITEM_DATA to a different type than the type that was attempted to read");
	}

	template<typename T>
	operator T& () {
		if(std::holds_alternative<T>(data)) {
			auto& ref = std::get<T>(data);
			return ref;
		}
		throw std::runtime_error("ITEM_DATA does not hold the requested type!\n The code set the ITEM_DATA to a different type than the type that was attempted to read");
	}

	template<typename T>
	ITEM_DATA& operator=(T* newData) {
		data = *newData;
		return *this;
	}

	ITEM_DATA& operator=(std::nullptr_t null) {
		data = nullptr;
		return *this;
	}

	template<typename T>
	ITEM_DATA& operator=(T& newData) {
		data = newData;
		return *this;
	}
	template<typename T>
	ITEM_DATA& operator=(T&& newData) {
		data = std::move(newData);
		return *this;
	}
	operator bool() {
		return !std::holds_alternative<std::nullptr_t>(data);
	}
	struct foo {
		void operator()(double& d) {}
	};

	auto& get() const {
		return data;
	}
};
