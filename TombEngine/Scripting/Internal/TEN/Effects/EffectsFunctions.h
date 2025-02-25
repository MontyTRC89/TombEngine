#pragma once

namespace sol { class state; };

namespace TEN::Scripting::Effects
{
	void Register(sol::state* lua, sol::table& parent);

	// Horizon Rotation and Meshswap variables
	extern Vector3 _horizonRotation;
	extern Vector3 _horizonRotationOld;
	extern GAME_OBJECT_ID horizon;
};

