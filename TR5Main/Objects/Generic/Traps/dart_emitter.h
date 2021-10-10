#pragma once

namespace TEN::Entities::Traps
{
	void DartControl(short itemNumber);
	void DartEmitterControl(short itemNumber);
	void TriggerDartSmoke(int x, int y, int z, int xv, int zv, bool hit);
}