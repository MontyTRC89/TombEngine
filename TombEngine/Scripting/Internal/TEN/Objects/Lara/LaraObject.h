#pragma once
#include "Game/Lara/lara_struct.h"
#include "Scripting/Internal/TEN/Objects/Moveable/MoveableObject.h"

static const std::unordered_map<std::string, LaraWeaponType> LaraWeaponTypeMap
{
	{ "NONE", LaraWeaponType::None },
	{ "PISTOLS", LaraWeaponType::Pistol },
	{ "REVOLVER", LaraWeaponType::Revolver },
	{ "UZI", LaraWeaponType::Uzi },
	{ "SHOTGUN", LaraWeaponType::Shotgun },
	{ "HK", LaraWeaponType::HK },
	{ "CROSSBOW", LaraWeaponType::Crossbow },
	{ "FLARE", LaraWeaponType::Flare },
	{ "TORCH", LaraWeaponType::Torch },
	{ "GRENADELAUNCHER", LaraWeaponType::GrenadeLauncher },
	{ "HARPOONGUN", LaraWeaponType::HarpoonGun },
	{ "ROCKETLAUNCHER", LaraWeaponType::RocketLauncher },
	{ "SNOWMOBILE", LaraWeaponType::Snowmobile },
	{ "NUMWEAPONS", LaraWeaponType::NumWeapons}
};

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
	static void Register(sol::table& parent);

	void SetPoison(sol::optional<int> potency);
	void SetAir(sol::optional<int> air);
	void SetStamina(sol::optional<int> value);
	void SetWet(sol::optional<int> wetness);
	void SetAirborne(bool newAirborne);
	void SetControlLock(bool value);
	void SetWeaponType(LaraWeaponType weaponType, bool activate);

	int GetPoison() const;
	int GetAir() const;
	int GetStamina() const;
	int GetWet() const;
	bool GetAirborne() const;
	bool GetControlLock() const;

	std::unique_ptr<Moveable> GetVehicle() const;
	std::unique_ptr<Moveable> GetTarget() const;
	std::unique_ptr<Moveable> GetInteractedMoveable() const;
	HandStatus GetHandStatus() const;
	LaraWeaponType GetWeaponType() const;
	int GetAmmoType() const;
	int GetAmmoCount() const;

	void UndrawWeapon();
	void ThrowAwayTorch();
	bool TorchIsLit() const;

	using Moveable::Moveable;
};
