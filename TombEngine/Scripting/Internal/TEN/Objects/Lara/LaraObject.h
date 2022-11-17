#pragma once
#include "Objects/Moveable/MoveableObject.h"

class LaraObject : public Moveable
{
public:
	void SetOnFire(bool onFire);
	bool GetOnFire() const;
	void SetPoison(sol::optional<int> potency);
	int GetPoison() const;
	void SetAir(sol::optional<int> air);
	int GetAir() const;
	void SetSprintEnergy(sol::optional<int> value);
	int GetSprintEnergy() const;
	void SetWet(sol::optional<int> wetness);
	int GetWet() const;
	int GetAmmoCount() const;
	static void Register(sol::table & parent);
	using Moveable::Moveable;
};

