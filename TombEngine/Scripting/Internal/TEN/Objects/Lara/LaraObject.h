#pragma once
#include "Game/Lara/lara_struct.h"
#include "Scripting/Internal/TEN/Objects/Moveable/MoveableObject.h"

static const std::unordered_map<std::string, HandStatus> HandStatusMap
{
	{ "FREE", HandStatus::Free },
	{ "BUSY", HandStatus::Busy },
	{ "WEAPONDRAW", HandStatus::WeaponDraw },
	{ "WEAPONUNDRAW", HandStatus::WeaponUndraw },
	{ "WEAPONREADY", HandStatus::WeaponReady },
	{ "SPECIAL", HandStatus::Special }
};

// TODO: Organise.
class LaraObject : public Moveable
{
public:
	void SetPoison(sol::optional<int> potency);
	int GetPoison() const;
	void SetAir(sol::optional<int> air);
	int GetAir() const;
	void SetStamina(sol::optional<int> value);
	int GetStamina() const;
	void SetWet(sol::optional<int> wetness);
	int GetWet() const;
	[[nodiscard]] bool GetAirborne() const;
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
	static void Register(sol::table& parent);
	using Moveable::Moveable;
};
