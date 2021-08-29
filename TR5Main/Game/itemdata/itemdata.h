#pragma once
#include <variant>
#include <functional>
#include <cstddef>
struct ITEM_INFO;

class ITEM_DATA {
	std::variant<std::nullptr_t,long long, unsigned long long, long double, ITEM_INFO, AiInfo*> data;
};
