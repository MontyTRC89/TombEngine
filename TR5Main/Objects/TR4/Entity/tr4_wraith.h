#pragma once

struct ITEM_INFO;

namespace TEN::Entities::TR4
{
	void InitialiseWraith(short itemNumber);
	void WraithControl(short itemNumber);
	void WraithWallsEffect(int x, int y, int z, short yrot, short objNumber);
	void DrawWraith(int x, int y, int z, short xVel, short yVel, short zVel, int objNumber);
	void KillWraith(ITEM_INFO* item);
	void WraithExplosionEffect(ITEM_INFO* item, byte r, byte g, byte b, int speed);
}