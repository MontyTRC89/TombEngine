#pragma once

class Vector3i;
struct ItemInfo;

namespace TEN::Entities::TR4
{
	void InitialiseWraith(short itemNumber);
	void WraithControl(short itemNumber);
	void WraithWallsEffect(Vector3i pos, short yRot, int objectNumber);
	void DrawWraith(Vector3i pos, Vector3i velocity, int objectNumber);
	void KillWraith(ItemInfo* item);
	void WraithExplosionEffect(ItemInfo* item, byte r, byte g, byte b, int velocity);
	void SpawnWraithTails(const ItemInfo& wraithItem, const Pose& relOffset, int objectNumber);
}
