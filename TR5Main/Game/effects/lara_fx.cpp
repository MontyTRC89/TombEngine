#include "framework.h"
#include "Game/effects/lara_fx.h"

#include "Game/collision/floordata.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/effects/smoke.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Specific/level.h"

using namespace TEN::Effects::Smoke;

namespace TEN::Effects::Lara
{
	void LaraBurn(ITEM_INFO* item)
	{
		if (!item->data.is<LaraInfo*>())
			return;

		auto lara = (LaraInfo*&)item->data;

		if (!lara->burn && !lara->burnSmoke)
		{
			short fxNum = CreateNewEffect(item->roomNumber);
			if (fxNum != NO_ITEM)
			{
				EffectList[fxNum].objectNumber = ID_FLAME;
				lara->burn = true;
			}
		}
	}

	void LavaBurn(ITEM_INFO* item)
	{
		if (!item->data.is<LaraInfo*>())
			return;

		auto lara = (LaraInfo*&)item->data;

		if (item->hitPoints >= 0 && lara->waterStatus != LW_FLYCHEAT)
		{
			short roomNumber = item->roomNumber;
			FLOOR_INFO* floor = GetFloor(item->pos.xPos, 32000, item->pos.zPos, &roomNumber);
			if (item->floor == GetFloorHeight(floor, item->pos.xPos, 32000, item->pos.zPos))
			{
				//			if (Objects[ID_KAYAK].loaded && Objects[ID_KAYAK_LARA_ANIMS].loaded)		//TEMPORARILY ADDING THIS HACK FOR TESTING-// KayakLaraRapidsDrown works fine.
				//				KayakLaraRapidsDrown();
				//			else
				//			{
				item->hitPoints = -1;
				item->hitStatus = true;
				LaraBurn(item);
				//			}
			}
		}
	}

	void LaraBreath(ITEM_INFO* item)
	{
		if (!item->data.is<LaraInfo*>())
			return;

		auto lara = (LaraInfo*&)item->data;

		if (lara->waterStatus == LARA_WATER_STATUS::LW_UNDERWATER || item->hitPoints <= 0)
			return;

		if (!(g_Level.Rooms[item->roomNumber].flags & ENV_FLAG_COLD))
			return;

		switch (item->animNumber)
		{
		case LA_STAND_IDLE:
			if (item->frameNumber < GetFrameNumber((short)ID_LARA, LA_STAND_IDLE, 30))
				return;
			break;

		case LA_CROUCH_IDLE:
			if (item->frameNumber < GetFrameNumber((short)ID_LARA, LA_CROUCH_IDLE, 30))
				return;
			break;

		case LA_CRAWL_IDLE:
			if (item->frameNumber < GetFrameNumber((short)ID_LARA, LA_CRAWL_IDLE, 30))
				return;
			break;

		default:
			if (Wibble < 0x80 || Wibble > 0xC0)
				return;
		}

		float z = std::sin(TO_RAD(item->pos.yRot)) * -64.0f;
		float x = std::cos(TO_RAD(item->pos.yRot)) * -64.0f;
		auto offset = PHD_VECTOR(0, -4, 64);

		GetLaraJointPosition(&offset, LM_HEAD);

		auto seed = PHD_VECTOR((GetRandomControl() & 7) - 4,
			(GetRandomControl() & 7) - 8,
			(GetRandomControl() & 7) - 4);

		GetLaraJointPosition(&seed, LM_HEAD);
		TriggerBreathSmoke(offset.x, offset.y, offset.z, item->pos.yRot);
	}
}