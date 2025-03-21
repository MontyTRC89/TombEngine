#pragma once

#include "Game/Lara/lara_struct.h"
#include "Scripting/Internal/TEN/Objects/Moveable/MoveableObject.h"
#include "Scripting/Internal/TEN/Input/ActionIDs.h"

// TODO: Organise.
class LaraObject : public Moveable
{
public:
	static void Register(sol::table& parent);

	void SetPoison(sol::optional<int> potency);
	int GetPoison() const;
	void SetAir(sol::optional<int> air);
	int GetAir() const;
	void SetStamina(sol::optional<int> value);
	int GetStamina() const;
	void SetWet(sol::optional<int> wetness);
	int GetWet() const;
	bool GetAirborne() const;
	void SetAirborne(bool newAirborne);
	std::unique_ptr<Moveable> GetVehicle() const;
	std::unique_ptr<Moveable> GetTarget() const;
	std::unique_ptr<Moveable> GetPlayerInteractedMoveable() const;
	HandStatus GetHandStatus() const;
	LaraWeaponType GetWeaponType() const;
	void SetWeaponType(LaraWeaponType weaponType, bool activate);
	int GetAmmoType() const;
	int GetAmmoCount() const;
	void UndrawWeapon();
	void ThrowAwayTorch();
	bool TorchIsLit() const;
	void AlignToMoveable(Moveable& mov, TypeOrNil<int> animation, TypeOrNil <Vec3> bound1, TypeOrNil <Vec3> bound2, TypeOrNil <Rotation> rot1, TypeOrNil <Rotation> rot2, TypeOrNil <Vec3> offset, TypeOrNil<In> actionID) const;
	using Moveable::Moveable;
};
