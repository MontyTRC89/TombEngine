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
	void SetOnFire(bool onFire);
	bool GetOnFire() const;
	void SetPoison(sol::optional<int> potency);
	int GetPoison() const;
	void SetAir(sol::optional<int> air);
	int GetAir() const;
	void UndrawWeapons();
	void ThrowAwayTorch();
	HandStatus GetLaraHandStatus() const;
	LaraWeaponType GetLaraWeaponType() const;
	void SetLaraWeaponType(LaraWeaponType weaponType, bool activate);
	static void Register(sol::table & parent);
	using Moveable::Moveable;
};

