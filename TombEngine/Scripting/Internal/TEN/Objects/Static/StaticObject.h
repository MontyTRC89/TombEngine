#pragma once

#include "Game/room.h"
#include "Scripting/Internal/TEN/Objects/NamedBase.h"

namespace sol { class state; }

enum GAME_OBJECT_ID : short;
class Vec3;
namespace TEN::Scripting::Types { class ScriptColor; }

using namespace TEN::Scripting::Types;

namespace TEN::Scripting
{
	class Rotation;

	class Static : public NamedBase<Static, MESH_INFO&>
	{
	public:
		static void Register(sol::table& parent);

	private:
		// Fields

		MESH_INFO& _static;

	public:
		// Aliases

		using IdentifierType = std::reference_wrapper<MESH_INFO>;

		// Constructors, destructors

		Static(MESH_INFO& staticObj);
		Static(const Static& staticObj) = delete;
		~Static() = default;

		// Getters

		std::string	   GetName() const;
		GAME_OBJECT_ID GetObjectId() const;
		Vec3		   GetPosition() const;
		Rotation	   GetRotation() const;
		Vec3		   GetScale() const;
		ScriptColor	   GetColor() const;
		int			   GetHitPoints() const;
		bool		   GetActiveStatus() const;
		bool		   GetSolidStatus() const;

		// Setters

		void SetName(const std::string& name);
		void SetObjectId(GAME_OBJECT_ID objectId);
		void SetPosition(const Vec3& pos);
		void SetRotation(const Rotation& rot);
		void SetScale(const Vec3& scale);
		void SetColor(const ScriptColor& color);
		void SetHitPoints(int hitPoints);
		void SetSolidStatus(bool status);

		// Utilities

		void Enable();
		void Disable();
		void Shatter();

		// Operators

		Static& operator =(const Static& staticObj) = delete;

		// COMPATIBILITY

		void SetScale(float scale);
	};
}
