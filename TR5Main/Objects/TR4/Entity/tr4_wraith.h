#pragma once

struct ITEM_INFO;

namespace TEN::Entities::TR4
{
	void InitialiseWraith(short itemNumber);
	void WraithControl(short itemNumber);
	void WraithWallsEffect(int x, int y, int z, short yRot, short objectNumber);
	void DrawWraith(int x, int y, int z, short xVelocity, short yVelocity, short zVelocity, int objectNumber);
	void KillWraith(ITEM_INFO* item);
	void WraithExplosionEffect(ITEM_INFO* item, byte r, byte g, byte b, int velocity);
}
