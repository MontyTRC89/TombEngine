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
#include "biggun_info.h"
#include "quad_info.h"
#include "tr5_laserhead_info.h"
#include "creature_info.h"
#include <stdexcept>
struct ITEM_INFO;

//Type Wrapper to construct a ITEM_DATA
template<typename D>
struct ITEM_DATA_TYPE {
	using type = D;
};
class ITEM_DATA {
	std::variant<std::nullptr_t,
		long long,
		unsigned long long,
		long double,
		ITEM_INFO,
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
		SUB_INFO
	> data;

	//we have to use a wrapper for a type, because the compiler needs to distinguish different overloads
	template<typename D>
	ITEM_DATA(ITEM_DATA_TYPE<D> type) : data(ITEM_DATA_TYPE::type{}) {
	}


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
	T& operator=(T* newData){
		data = *newData;
		return *this;
	}
};
