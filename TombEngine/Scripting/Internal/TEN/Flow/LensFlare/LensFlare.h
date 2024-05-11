#pragma once
#include "Objects/game_object_ids.h"
#include "Objects/objectslist.h"
#include "Scripting/Internal/TEN/Color/Color.h"
#include "Scripting/Internal/TEN/Rotation/Rotation.h"

namespace sol { class state; }

namespace TEN::Scripting
{
	class LensFlare
	{
	private:
		// Members
		int	 _sunSpriteID = SPRITE_TYPES::SPR_LENS_FLARE_3;
		bool _isEnabled	  = false;

		Rotation	_rotation = {};
		ScriptColor _color	  = 0;

	public:
		static void Register(sol::table& table);

		// Constructors
		LensFlare() = default;
		LensFlare(float pitch, float yaw, const ScriptColor& color);

		// Getters
		bool		GetEnabled() const;
		int			GetSunSpriteID() const;
		float		GetPitch() const;
		float		GetYaw() const;
		ScriptColor GetColor() const;

		// Setters
		void SetSunSpriteID(int spriteID);
		void SetPitch(float pitch);
		void SetYaw(float yaw);
		void SetColor(const ScriptColor& color);
	};
}
