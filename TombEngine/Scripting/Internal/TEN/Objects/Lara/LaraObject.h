#pragma once
#include "Objects/Moveable/MoveableObject.h"

class LaraObject : public Moveable
{
public:
	void SetOnFire(bool onFire);
	bool GetOnFire() const;
	static void Register(sol::table & parent);
	using Moveable::Moveable;
};

