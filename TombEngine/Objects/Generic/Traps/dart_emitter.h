#pragma once

namespace TEN::Entities::Traps
{
	void InitializeDartEmitter(short itemNumber);
	void DartControl(short itemNumber);
	void DartEmitterControl(short itemNumber);
	void TriggerDartSmoke(int x, int y, int z, int xv, int zv, bool hit);
}
