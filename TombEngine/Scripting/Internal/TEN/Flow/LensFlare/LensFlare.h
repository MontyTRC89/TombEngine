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
		int _sunSpriteID = SPRITE_TYPES::SPR_LENS_FLARE_3;
		bool _isEnabled;

		Rotation	_rotation = {};
		ScriptColor _color	  = 0;

	public:
		static void Register(sol::table& table);

		// Constructors
		LensFlare() = default;
		LensFlare(const Rotation& rot, const ScriptColor& color);

		// Getters
		bool		GetEnabled() const;
		int			GetSunSpriteID() const;
		Rotation	GetRotation() const;
		ScriptColor GetColor() const;

		// Setters
		void SetSunSpriteID(int spriteID);
		void SetRotation(const Rotation& rot);
		void SetColor(const ScriptColor& color);
	};
}
