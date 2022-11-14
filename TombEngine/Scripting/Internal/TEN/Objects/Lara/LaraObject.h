#pragma once
#include "Objects/Moveable/MoveableObject.h"

class LaraObject : public Moveable
{
public:
	void SetOnFire(bool onFire);
	bool GetOnFire() const;
	void SetPoison(int potency);
	int GetPoison();
	void RemovePoison();
	void SetAir(int air);
	int GetAir();
	static void Register(sol::table & parent);
	using Moveable::Moveable;
};

