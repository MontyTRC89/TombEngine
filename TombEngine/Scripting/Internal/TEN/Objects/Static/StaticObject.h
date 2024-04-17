#pragma once
#include "Game/room.h"
#include "Scripting/Internal/TEN/Objects/NamedBase.h"

namespace sol {	class state; }
class Vec3;
class Rotation;
class ScriptColor;

class Static : public NamedBase<Static, StaticObject&>
{
public:
	using IdentifierType = std::reference_wrapper<StaticObject>;

	static void Register(sol::table& parent);

	// Constructors and destructors
	Static(StaticObject& staticObj);
	~Static() = default;

	Static(const Static& staticObj) = delete;

	// Getters
	std::string GetName() const;
	int			GetSlotID() const;
	Vec3		GetPosition() const;
	Rotation	GetRotation() const;
	float		GetScale() const;
	ScriptColor GetColor() const;
	int			GetHitPoints() const;
	bool		GetSolid() const;
	bool		GetActive() const;

	// Setters
	void SetName(const std::string& name);
	void SetSlotID(int slotID);
	void SetPosition(const Vec3& pos);
	void SetRotation(const Rotation& rot);
	void SetScale(float scale);
	void SetColor(const ScriptColor& color);
	void SetHitPoints(int hitPoints);
	void SetSolid(bool isSolid);

	// Utilities
	void Enable();
	void Disable();
	void Shatter();

	// Operators
	Static& operator =(const Static& staticObj) = delete;

private:
	StaticObject& _static;
};
