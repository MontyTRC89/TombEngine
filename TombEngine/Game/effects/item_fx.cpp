#include "framework.h"
#include "Game/effects/item_fx.h"

#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/effects/smoke.h"
#include "Game/items.h"
#include "Color/Color.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Specific/level.h"

using namespace TEN::Effects::Smoke;

namespace TEN::Effects::Items
{
	void ItemBurn(ItemInfo* item, int timeout)
	{
		item->Effect.Type = EffectType::Fire;
		item->Effect.Count = timeout;
		item->Effect.LightColor = Vector3(250, 176, 0);
		item->Effect.EffectColor1 = Vector3(255, 48, 48);
		item->Effect.EffectColor2 = Vector3(255, 216, 32);
	}

	void ItemColorBurn(ItemInfo* item, ScriptColor const& color1, ScriptColor const& color2, int timeout)
	{
		item->Effect.Type = EffectType::ColoredFire;
		Vector3 col1 = Vector3(color1.GetR(), color1.GetG(), color1.GetB());
		Vector3 col2 = Vector3(color2.GetR(), color2.GetG(), color2.GetB());
		item->Effect.Count = timeout;
		item->Effect.LightColor = col1;
		item->Effect.EffectColor1 = col1;
		item->Effect.EffectColor2 = col2;
	}

	void ItemMagicBurn(ItemInfo* item, ScriptColor const& color1, ScriptColor const& color2, int timeout)
	{
		item->Effect.Type = EffectType::MagicFire;
		Vector3 col1 = Vector3(color1.GetR(), color1.GetG(), color1.GetB());
		Vector3 col2 = Vector3(color2.GetR(), color2.GetG(), color2.GetB());
		item->Effect.Count = timeout;
		item->Effect.LightColor = col1;
		item->Effect.EffectColor1 = col1;
		item->Effect.EffectColor2 = col2;
	}

	void ItemElectricBurn(ItemInfo* item, int timeout)
	{
		item->Effect.Type = EffectType::Sparks;
		item->Effect.Count = timeout;
		item->Effect.LightColor = Vector3(0, 147, 255 );
	}

	void ItemBlueElectricBurn(ItemInfo* item, int timeout)
	{
		item->Effect.Type = EffectType::ElectricDeath;
		item->Effect.Count = 2;
		item->Effect.LightColor = Vector3(0, 147, 255);
	}

	void ItemRedLaserBurn(ItemInfo* item, int timeout)
	{
		item->Effect.Type = EffectType::LaserDeath;
		item->Effect.Count = 2;
		item->Effect.LightColor = Vector3(255, 100, 0);
	}

	void ItemSmoke(ItemInfo* item, int timeout)
	{
		item->Effect.Type = EffectType::Smoke;
		item->Effect.Count = timeout;
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
			ItemBurn(item);
		}
	}

	void LaraBreath(ItemInfo* item)
	{
		if (item->IsLara())
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
