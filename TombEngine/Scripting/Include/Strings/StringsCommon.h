#include "Scripting/Internal/TEN/Vec2/Vec2.h"

using CallbackDrawString = std::function<
	void(
		const std::string&,
		D3DCOLOR,
		Vec2,
		float, // Scale
		int)>; // Flags
