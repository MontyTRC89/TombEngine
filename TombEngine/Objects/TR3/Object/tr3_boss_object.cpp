#include "framework.h"
#include "Objects/TR3/Object/tr3_boss_object.h"

#include "Game/collision/collide_room.h"
#include "Game/effects/effects.h"
#include "Game/effects/spark.h"
#include "Game/items.h"
#include "Game/misc.h"
#include "Objects/TR3/Entity/tr3_punaboss.h"
#include "Specific/setup.h"

using namespace TEN::Effects::Spark;
using namespace TEN::Entities::Creatures::TR3;

namespace TEN::Effects::Boss
{
	void SpawnShield(const ItemInfo& item, const Vector4& color)
	{
		if (!item.TestFlag(BOSSFlag_Object, BOSS_Shield))
			return;

		int itemNumber = CreateItem();
		if (itemNumber == NO_ITEM)
			return;

		auto& shieldItem = g_Level.Items[itemNumber];

		shieldItem.ObjectNumber = ID_BOSS_SHIELD;
		shieldItem.RoomNumber = item.RoomNumber;
		shieldItem.Pose.Position = item.Pose.Position;
		shieldItem.Pose.Position.y -= CLICK(3);
		shieldItem.Pose.Orientation = item.Pose.Orientation;
		shieldItem.Flags |= IFLAG_ACTIVATION_MASK;

		InitialiseItem(itemNumber);

		shieldItem.Model.Color = color;
		shieldItem.Collidable = true;
		shieldItem.ItemFlags[0] = 2;
		shieldItem.Model.Mutator[0].Scale = Vector3(2.0f); // Scale model by factor of 2.
		shieldItem.Status = ITEM_ACTIVE;

		AddActiveItem(itemNumber);
	}

	void SpawnShockwaveExplosion(const ItemInfo& item, const Vector4& color)
	{
		if (!item.TestFlag(BOSSFlag_Object, BOSS_ShockwaveExplosion))
			return;

		int itemNumber = CreateItem();
		if (itemNumber == NO_ITEM)
			return;

		auto* spawned = &g_Level.Items[itemNumber];

		spawned->Pose.Position = item.Pose.Position;
		spawned->Pose.Position.y -= CLICK(2);

		spawned->Pose.Orientation = EulerAngles(
			Random::GenerateAngle(ANGLE(-20.0f), ANGLE(20.0f)),
			Random::GenerateAngle(ANGLE(-20.0f), ANGLE(20.0f)),
			Random::GenerateAngle(ANGLE(-20.0f), ANGLE(20.0f)));

		spawned->ObjectNumber = ID_BOSS_EXPLOSION_SHOCKWAVE;
		spawned->RoomNumber = item.RoomNumber;
		spawned->Flags |= IFLAG_ACTIVATION_MASK;

		InitialiseItem(itemNumber);

		auto result = color;
		result.w = 1.0f;
		spawned->Model.Color = result;
		spawned->Collidable = false;					 // No collision for this entity.
		spawned->ItemFlags[0] = 70;						 // Timer before clearing; will fade out, then get destroyed.
		spawned->Model.Mutator[0].Scale = Vector3::Zero; // Start without scale.
		spawned->Status = ITEM_ACTIVE;

		AddActiveItem(itemNumber);
		SoundEffect(SFX_TR3_BLAST_CIRCLE, &spawned->Pose);
	}

	void SpawnShockwaveRing(const ItemInfo& item, const Vector3& pos, const Vector4& color)
	{
		if (!item.TestFlag(BOSSFlag_Object, BOSS_ShockwaveExplosion))
			return;

		int itemNumber = CreateItem();
		if (itemNumber == NO_ITEM)
			return;

		auto& ringItem = g_Level.Items[itemNumber];

		ringItem.Pose.Position = pos;
		ringItem.Pose.Position.y -= CLICK(2);

		ringItem.Pose.Orientation = EulerAngles(
			Random::GenerateAngle(ANGLE(-60.0f), ANGLE(60.0f)),
			Random::GenerateAngle(),
			Random::GenerateAngle(ANGLE(-60.0f), ANGLE(60.0f)));

		ringItem.ObjectNumber = ID_BOSS_EXPLOSION_RING;
		ringItem.RoomNumber = item.RoomNumber;
		ringItem.Flags |= IFLAG_ACTIVATION_MASK;

		InitialiseItem(itemNumber);

		auto result = color;
		result.w = 1.0f;
		ringItem.Model.Color = result;
		ringItem.Collidable = false;					 // No collision for this entity.
		ringItem.ItemFlags[0] = 70;						 // Timer before clearing; will fade out, then get destroyed.
		ringItem.Model.Mutator[0].Scale = Vector3::Zero; // Start without scale.
		ringItem.Status = ITEM_ACTIVE;

		AddActiveItem(itemNumber);
		SoundEffect(SFX_TR3_BLAST_CIRCLE, &ringItem.Pose);
	}

	void ShieldControl(int itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		// Entity life.
		if (item.ItemFlags[0] > 0)
			item.ItemFlags[0]--;

		if (item.ItemFlags[0] <= 0)
		{
			RemoveActiveItem(itemNumber);
			KillItem(itemNumber);
		}

		auto colorEnd = Vector4(item.Model.Color.x - 0.1f, item.Model.Color.y - 0.1f, item.Model.Color.z - 0.1f, item.Model.Color.w);
		item.Model.Color = Vector4::Lerp(item.Model.Color, colorEnd, 0.35f);
		item.Pose.Orientation.y += Random::GenerateAngle();
		UpdateItemRoom(itemNumber);
	}

	// NOTE: Ring and explosion wave rotate and scale by default.
	void ShockwaveRingControl(int itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		// Entity life.
		if (item.ItemFlags[0] > 0)
			item.ItemFlags[0]--;

		if (item.ItemFlags[0] <= 0)
		{
			auto colorEnd = Vector4(item.Model.Color.x - 0.1f, item.Model.Color.y - 0.1f, item.Model.Color.z - 0.1f, item.Model.Color.w);
			item.Model.Color = Vector4::Lerp(item.Model.Color, colorEnd, 0.35f);

			if (item.Model.Color.x <= 0.0f &&
				item.Model.Color.y <= 0.0f &&
				item.Model.Color.z <= 0.0f)
			{
				RemoveActiveItem(itemNumber);
				KillItem(itemNumber);
			}
		}

		item.Model.Mutator[0].Scale += Vector3::One;
		UpdateItemRoom(itemNumber);
	}

	void ShockwaveExplosionControl(int itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		// Entity life.
		if (item.ItemFlags[0] > 0)
			item.ItemFlags[0]--;

		if (item.ItemFlags[0] <= 0)
		{
			auto colorEnd = Vector4(item.Model.Color.x - 0.1f, item.Model.Color.y - 0.1f, item.Model.Color.z - 0.1f, item.Model.Color.w);
			item.Model.Color = Vector4::Lerp(item.Model.Color, colorEnd, 0.35f);

			if (item.Model.Color.x <= 0.0f &&
				item.Model.Color.y <= 0.0f &&
				item.Model.Color.z <= 0.0f)
			{
				RemoveActiveItem(itemNumber);
				KillItem(itemNumber);
			}
		}

		item.Pose.Orientation.y += ANGLE(5.0f);
		item.Model.Mutator[0].Scale += Vector3(0.5f);
		UpdateItemRoom(itemNumber);
	}

	void SpawnExplosionSmoke(const Vector3& pos)
	{
		auto& spark = *GetFreeParticle();

		spark.on = true;
		spark.sR = 75;
		spark.sG = 75;
		spark.sB = 175;
		spark.dR = 25;
		spark.dG = 25;
		spark.dB = 100;
		spark.colFadeSpeed = 8;
		spark.fadeToBlack = 64;
		spark.life =
		spark.sLife = Random::GenerateInt(96, 128);
		spark.blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		spark.x = pos.x + Random::GenerateInt(-16, 16);
		spark.y = pos.y + Random::GenerateInt(-16, 16);
		spark.z = pos.z + Random::GenerateInt(-16, 16);
		spark.xVel = ((GetRandomControl() & 0xFFF) - 2048) >> 2;
		spark.yVel = GetRandomControl() - 128;
		spark.zVel = ((GetRandomControl() & 0xFFF) - 2048) >> 2;
		spark.friction = 6;
		spark.flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		spark.rotAng = GetRandomControl() & 0xFFF;

		if (Random::TestProbability(1 / 2.0f))
			spark.rotAdd = -16 - (GetRandomControl() & 0xF);
		else
			spark.rotAdd = (GetRandomControl() & 0xF) + 16;

		spark.scalar = 3;
		spark.gravity = -3 - (GetRandomControl() & 3);
		spark.maxYvel = -8 - (GetRandomControl() & 3);
		spark.dSize = (GetRandomControl() & 0x1F) + 256;
		spark.sSize = spark.dSize / 2;
		spark.size = spark.dSize / 2;
	}

	void ExplodeBoss(int itemNumber, ItemInfo& item, int deathCountToDie, const Vector4& color)
	{
		auto pos = Vector3::Zero;

		// Disable shield.
		item.SetFlag(BOSSFlag_ShieldIsEnabled, 0);
		item.HitPoints = NOT_TARGETABLE;

		// Start doing the explosion (entity will do the count).
		short counter = item.ItemFlags[BOSSFlag_ExplodeCount];
		if (counter == 1)
		{
			SpawnShockwaveExplosion(item, color);
			for (int i = 0; i < 3; i++)
			{
				pos = Vector3(
					item.Pose.Position.x + Random::GenerateInt(-512, 512),
					(item.Pose.Position.y - CLICK(3)) + Random::GenerateInt(-512, 512),
					item.Pose.Position.z + Random::GenerateInt(-512, 512));
				SpawnExplosionSmoke(pos);
			}
		}

		if (counter == 10 ||
			counter == 20 ||
			counter == 30 ||
			counter == 40 ||
			counter == 50 ||
			counter == 60)
		{
			
			for (int i = 0; i < 3; i++)
			{
				pos = Vector3(
					item.Pose.Position.x + Random::GenerateInt(-512, 512),
					(item.Pose.Position.y - CLICK(3)) + Random::GenerateInt(-512, 512),
					item.Pose.Position.z + Random::GenerateInt(-512, 512));
				SpawnExplosionSmoke(pos);
			}

			pos = Vector3(
				item.Pose.Position.x + Random::GenerateInt(-64, 64),
				(item.Pose.Position.y - CLICK(2)) + Random::GenerateInt(-64, 64),
				item.Pose.Position.z + Random::GenerateInt(-64, 64));
			SpawnShockwaveRing(item, pos, color);
		}

		TriggerDynamicLight(
			item.Pose.Position.x,
			item.Pose.Position.y - CLICK(2),
			item.Pose.Position.z, counter / 2,
			color.x * 255, color.y * 255, color.z * 255);

		if (counter == deathCountToDie)
			CreatureDie(itemNumber, true);
	}

	void CheckForRequiredObjects(ItemInfo& item)
	{
		short flagResult = 0;

		if (item.ObjectNumber == ID_PUNA_BOSS && Objects[ID_LIZARD].loaded)
			flagResult |= BOSS_Lizard;

		// These are only for rendering effects. Not required but highly recommended for nice aesthetics.
		if (Objects[ID_BOSS_EXPLOSION_RING].loaded)
			flagResult |= BOSS_ShockwaveRing;

		if (Objects[ID_BOSS_EXPLOSION_SHOCKWAVE].loaded)
			flagResult |= BOSS_ShockwaveExplosion;

		if (Objects[ID_BOSS_SHIELD].loaded)
			flagResult |= BOSS_Shield;

		item.SetFlag(BOSSFlag_Object, flagResult);
	}

	void SpawnShieldAndRichochetSparks(const ItemInfo& item, const Vector3& pos, const Vector4& color)
	{
		SpawnShield(item, color);

		auto target = GameVector(pos, item.RoomNumber);
		auto yOrient = Geometry::GetOrientToPoint(item.Pose.Position.ToVector3(), target.ToVector3()).y;
		auto sparkColor = color;
		sparkColor.w = 1.0f;

		TriggerRicochetSpark(target, yOrient, 13, sparkColor);
	}
}
