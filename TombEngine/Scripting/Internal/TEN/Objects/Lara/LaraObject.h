#pragma once
#include "Game/Lara/lara_struct.h"
#include "Objects/Moveable/MoveableObject.h"

static const std::unordered_map<std::string, LaraWeaponType> LaraWeaponTypeMap
{
	{"NONE", LaraWeaponType::None},
	{"PISTOLS", LaraWeaponType::Pistol},
	{"REVOLVER", LaraWeaponType::Revolver},
	{"UZI", LaraWeaponType::Uzi},
	{"SHOTGUN", LaraWeaponType::Shotgun},
	{"HK", LaraWeaponType::HK},
	{"CROSSBOW", LaraWeaponType::Crossbow},
	{"FLARE", LaraWeaponType::Flare},
	{"TORCH", LaraWeaponType::Torch},
	{"GRENADELAUNCHER", LaraWeaponType::GrenadeLauncher},
	{"HARPOONGUN", LaraWeaponType::HarpoonGun},
	{"ROCKETLAUNCHER", LaraWeaponType::RocketLauncher},
	{"SNOWMOBILE", LaraWeaponType::Snowmobile},
	{"NUMWEAPONS", LaraWeaponType::NumWeapons}
};

static const std::unordered_map<std::string, HandStatus> HandStatusMap
{
	{"FREE", HandStatus::Free},
	{"BUSY", HandStatus::Busy},
	{"WEAPONDRAW", HandStatus::WeaponDraw},
	{"WEAPONUNDRAW", HandStatus::WeaponUndraw},
	{"WEAPONREADY", HandStatus::WeaponReady},
	{"SPECIAL", HandStatus::Special},
};


class LaraObject : public Moveable
{
public:
	void SetPoison(sol::optional<int> potency);
	int GetPoison() const;
	void SetAir(sol::optional<int> air);
	int GetAir() const;
	void SetSprintEnergy(sol::optional<int> value);
	int GetSprintEnergy() const;
	void SetWet(sol::optional<int> wetness);
	int GetWet() const;
	[[nodiscard]] bool GetAirborne() const;
	void SetAirborne(bool newAirborne);
	std::unique_ptr<Moveable> GetVehicle() const;
	std::unique_ptr<Moveable> GetTarget() const;
	HandStatus GetHandStatus() const;
	LaraWeaponType GetWeaponType() const;
	void SetWeaponType(LaraWeaponType weaponType, bool activate);
	int GetAmmoCount() const;
	void UndrawWeapon();
	void ThrowAwayTorch();
	bool TorchIsLit() const;
	static void Register(sol::table & parent);
	using Moveable::Moveable;
};

