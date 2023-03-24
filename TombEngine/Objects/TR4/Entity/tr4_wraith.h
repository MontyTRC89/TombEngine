#pragma once

class Vector3i;
struct ItemInfo;

namespace TEN::Entities::TR4
{
	void InitialiseWraith(short itemNumber);
	void WraithControl(short itemNumber);

	void DrawWraith(Vector3i pos, Vector3i velocity, int objectNumber);
	void KillWraith(ItemInfo* item);
}
