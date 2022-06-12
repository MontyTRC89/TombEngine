#include "framework.h"
#include "Game/effects/lara_fx.h"

#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/effects/smoke.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Specific/level.h"

using namespace TEN::Effects::Smoke;

namespace TEN::Effects::Lara
{
	void LaraBurn(ItemInfo* item)
	{
		if (!item->Data.is<LaraInfo*>())
			return;

		auto* lara = GetLaraInfo(item);

		if (!lara->Burn && !lara->BurnSmoke)
		{
			short fxNum = CreateNewEffect(item->RoomNumber);
			if (fxNum != NO_ITEM)
			{
				EffectList[fxNum].objectNumber = ID_FLAME;
				lara->Burn = true;
			}
		}
	}

	void LavaBurn(ItemInfo* item)
	{
		if (!item->Data.is<LaraInfo*>())
			return;

		auto* lara = GetLaraInfo(item);

		if (item->HitPoints >= 0 && lara->Control.WaterStatus != WaterStatus::FlyCheat)
		{
			short roomNumber = item->RoomNumber;
			FloorInfo* floor = GetFloor(item->Pose.Position.x, 32000, item->Pose.Position.z, &roomNumber);
			if (item->Floor == GetFloorHeight(floor, item->Pose.Position.x, 32000, item->Pose.Position.z))
			{
				//			if (Objects[ID_KAYAK].loaded && Objects[ID_KAYAK_LARA_ANIMS].loaded)		//TEMPORARILY ADDING THIS HACK FOR TESTING-// KayakLaraRapidsDrown works fine.
				//				KayakLaraRapidsDrown();
				//			else
				//			{
				item->HitPoints = -1;
				item->HitStatus = true;
				LaraBurn(item);
				//			}
			}
		}
	}

	void LaraBreath(ItemInfo* item)
	{
		if (!item->Data.is<LaraInfo*>())
			return;

		auto* lara = GetLaraInfo(item);

		if (lara->Control.WaterStatus == WaterStatus::Underwater || item->HitPoints <= 0)
			return;

		if (!TestEnvironment(ENV_FLAG_COLD, item))
			return;

		switch (item->Animation.AnimNumber)
		{
		case LA_STAND_IDLE:
			if (item->Animation.FrameNumber < GetFrameNumber((short)ID_LARA, LA_STAND_IDLE, 30))
				return;
			break;

		case LA_CROUCH_IDLE:
			if (item->Animation.FrameNumber < GetFrameNumber((short)ID_LARA, LA_CROUCH_IDLE, 30))
				return;
			break;

		case LA_CRAWL_IDLE:
			if (item->Animation.FrameNumber < GetFrameNumber((short)ID_LARA, LA_CRAWL_IDLE, 30))
				return;
			break;

		default:
			if (Wibble < 0x80 || Wibble > 0xC0)
				return;
		}

		float z = std::sin(TO_RAD(item->Pose.Orientation.y)) * -64.0f;
		float x = std::cos(TO_RAD(item->Pose.Orientation.y)) * -64.0f;
		auto offset = Vector3Int(0, -4, 64);

		GetLaraJointPosition(&offset, LM_HEAD);

		auto seed = Vector3Int((GetRandomControl() & 7) - 4,
			(GetRandomControl() & 7) - 8,
			(GetRandomControl() & 7) - 4);

		GetLaraJointPosition(&seed, LM_HEAD);
		TriggerBreathSmoke(offset.x, offset.y, offset.z, item->Pose.Orientation.y);
	}
}
