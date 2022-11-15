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
	static void Register(sol::table & parent);
	using Moveable::Moveable;
};

