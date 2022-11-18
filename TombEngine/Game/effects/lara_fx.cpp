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
		item->Effect.Type = EffectType::Burn;
		item->Effect.Count = -1;
		item->Effect.Color = Vector3(0.8f, 0.5f, 0.0f);
	}

	void LaraElectricBurn(ItemInfo* item)
	{
		item->Effect.Type = EffectType::Electric;
		item->Effect.Count = 48;
		item->Effect.Color = Vector3(0.0f, 0.2f, 0.8f);
	}

	void LavaBurn(ItemInfo* item)
	{
		if (item->IsLara() && GetLaraInfo(item)->Control.WaterStatus == WaterStatus::FlyCheat)
			return;

		if (item->HitPoints < 0)
			return;

		auto height = GetCollision(item->Pose.Position.x, 32000, item->Pose.Position.z, item->RoomNumber).Position.Floor;
		if (item->Floor == height)
		{
			item->HitPoints = -1;
			item->HitStatus = true;
			LaraBurn(item);
		}
	}

	void LaraBreath(ItemInfo* item)
	{
		if (!item->IsLara())
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
		auto offset = GetJointPosition(item, LM_HEAD, Vector3i(0, -4, 64));

		auto seed = GetJointPosition(item, 
			LM_HEAD,
			Vector3i((GetRandomControl() & 7) - 4,
				(GetRandomControl() & 7) - 8,
				(GetRandomControl() & 7) - 4));

		TriggerBreathSmoke(offset.x, offset.y, offset.z, item->Pose.Orientation.y);
	}
}
