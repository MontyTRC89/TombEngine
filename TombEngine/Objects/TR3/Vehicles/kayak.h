#pragma once
#include "Objects/TR3/Vehicles/kayak_info.h"

struct CollisionInfo;
struct ItemInfo;

namespace TEN::Entities::Vehicles
{
	enum class VehicleMountType;

	KayakInfo& GetKayakInfo(ItemInfo& kayakItem);
	void InitializeKayak(short itemNumber);

	void KayakPlayerCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll);
	bool KayakControl(ItemInfo& laraItem);
	void KayakUserInput(ItemInfo& kayakItem, ItemInfo& laraItem);

	void KayakToBackground(ItemInfo& kayakItem, ItemInfo& laraItem);
	void KayakToEntityCollision(ItemInfo& kayakItem, ItemInfo& laraItem);

	void DoKayakMount(ItemInfo& kayakItem, ItemInfo& laraItem, VehicleMountType mountType);
	bool TestKayakDismount(ItemInfo& kayakItem, int direction);

	void KayakLaraRapidsDrown(ItemInfo& laraItem);
	void KayakDoCurrent(ItemInfo& kayakItem, ItemInfo& laraItem);
	void KayakDraw(ItemInfo& kayakItem);
	void KayakDoWake(ItemInfo& kayakItem, int xOffset, int zOffset, short rotate);
	void KayakDoRipple(ItemInfo& kayakItem, int xOffset, int zOffset);
	void UpdateKayakWakeEffect();
}
