#pragma once
#include <variant>
#include <functional>
#include <cstddef>
struct ITEM_INFO;
class ITEM_DATA {
	std::variant<std::nullptr_t,int, short, ItemInfo*, AiInfo*> data;
	public:
	template<typename ... F>
	void apply(F&&... funcs) {
		std::visit(std::forward<F>(...funcs), data);
	}
};
