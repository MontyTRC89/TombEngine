#pragma once

struct Vector3Int;
struct ITEM_INFO;

namespace TEN::Entities::TR4
{
	void InitialiseWraith(short itemNumber);
	void WraithControl(short itemNumber);
	void WraithWallsEffect(Vector3Int pos, short yRot, short objectNumber);
	void DrawWraith(Vector3Int pos, Vector3Int velocity, int objectNumber);
	void KillWraith(ITEM_INFO* item);
	void WraithExplosionEffect(ITEM_INFO* item, byte r, byte g, byte b, int velocity);
}
