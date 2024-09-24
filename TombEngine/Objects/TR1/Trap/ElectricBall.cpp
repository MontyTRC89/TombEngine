#include "framework.h"
#include "Objects/TR1/Trap/ElectricBall.h"

#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/Point.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "Game/control/los.h"
#include "Game/effects/item_fx.h"
#include "Game/misc.h"
#include "Objects/Effects/Boss.h"
#include "Game/effects/spark.h"
#include "Objects/TR5/Entity/tr5_roman_statue.h"
#include "Objects/TR5/Light/tr5_light.h"
#include "Objects/TR5/Light/tr5_light_info.h"

using namespace TEN::Collision::Point;
using namespace TEN::Math;
using namespace TEN::Effects::Electricity;
using namespace TEN::Effects::Items;
using namespace TEN::Effects::Spark;
using namespace TEN::Effects::Boss;
using namespace TEN::Entities::Creatures::TR5;


namespace TEN::Entities::Traps
{
	// NOTES:
	// ItemFlags[0] = random turn rate when active.
	// ItemFlags[1] = calculated forward velocity.

	enum TargetType
	{
		None,
		Target
	};

	struct ElectricBallInfo
	{
		Vector3i Position = Vector3i::Zero;
		Electricity* EnergyArcs = {};
		Color Color;
		unsigned int Count = 0;
	};

	ElectricBallInfo ElectricBallData;

	constexpr auto ELECTRIC_LIGHTNING_DAMAGE = 350;
	constexpr auto DAMOCLES_SWORD_DAMAGE = 100;

	constexpr auto DAMOCLES_SWORD_VELOCITY_MIN = BLOCK(1 / 20.0f);
	constexpr auto DAMOCLES_SWORD_VELOCITY_MAX = BLOCK(1 / 8.0f);

	constexpr auto DAMOCLES_SWORD_IMPALE_DEPTH			  = -BLOCK(1 / 8.0f);
	constexpr auto DAMOCLES_SWORD_ACTIVATE_RANGE_2D		  = BLOCK(1.5f);
	constexpr auto DAMOCLES_SWORD_ACTIVATE_RANGE_VERTICAL = BLOCK(3);

	const auto ElectricBallBite = CreatureBiteInfo(Vector3::Zero, 0);
	const auto ElectricBallSparkBite = CreatureBiteInfo(Vector3::Zero, 0);

	constexpr auto DAMOCLES_SWORD_TURN_RATE_MAX = ANGLE(5.0f);
	constexpr auto DAMOCLES_SWORD_TURN_RATE_MIN = ANGLE(1.0f);

	static void TriggerElectricBallShockwaveAttackSparks(int x, int y, int z, byte r, byte g, byte b, byte size)
	{
		auto* spark = GetFreeParticle();

		spark->dG = g;
		spark->sG = g;
		spark->colFadeSpeed = 2;
		spark->dR = r;
		spark->sR = r;
		spark->blendMode = BlendMode::Additive;
		spark->life = 4;
		spark->sLife = 4;
		spark->x = x;
		spark->on = 1;
		spark->dB = b;
		spark->sB = b;
		spark->fadeToBlack = 4;
		spark->y = y;
		spark->z = z;
		spark->zVel = 0;
		spark->yVel = 0;
		spark->xVel = 0;
		spark->flags = SP_SCALE | SP_DEF;
		spark->scalar = 3;
		spark->maxYvel = 0;
		spark->spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_LENSFLARE_LIGHT;
		spark->gravity = 0;
		spark->dSize = spark->sSize = spark->size = size + (GetRandomControl() & 3);
	}

	const void SpawnElectricSmoke(const Vector3& pos, const EulerAngles& orient, float life)
	{
		auto& smoke = *GetFreeParticle();

		float scale = (life * 18) * phd_cos(orient.x);

		smoke.on = true;
		smoke.blendMode = BlendMode::Additive;
		smoke.spriteIndex = Objects[ID_DEFAULT_SPRITES].meshIndex + SPR_LENSFLARE_LIGHT;// SPR_LENSFLARE_LIGHT;

		smoke.x = pos.x;
		smoke.y = pos.y;
		smoke.z = pos.z;
		smoke.xVel = Random::GenerateInt(-25, 25);
		smoke.yVel = life * 3;
		smoke.zVel = Random::GenerateInt(-25, 25);
		smoke.sB = (Random::GenerateInt(28, 196) * life) / 16;
		smoke.sR =
			smoke.sG = smoke.sB - (smoke.sB / 4);
		
		smoke.dB = (Random::GenerateInt(62, 80) * life) / 16;
		smoke.dR = smoke.dB / 3;
		smoke.dG = 
			smoke.dB /2 ;

		smoke.colFadeSpeed = Random::GenerateInt(8, 12);
		smoke.fadeToBlack = 8;
		smoke.sLife =
			smoke.life = Random::GenerateFloat(24.0f, 28.0f);

		smoke.friction = 0;
		smoke.rotAng = Random::GenerateAngle(0, ANGLE(22.5f));
		smoke.rotAdd = Random::GenerateAngle(ANGLE(-0.35f), ANGLE(0.35f));
		smoke.gravity = Random::GenerateAngle(ANGLE(0.175f), ANGLE(0.35f));
		smoke.maxYvel = 0;
		smoke.scalar = 1;
		smoke.size =
			smoke.sSize = scale;
		smoke.dSize = 1;
		smoke.flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
	}

	void InitializeElectricBall(short itemNumber)
	{

		InitializeCreature(itemNumber);
		auto& item = g_Level.Items[itemNumber];
		CheckForRequiredObjects(item);

	
		ElectricBallData.Color = item.Model.Color;

		int sign = Random::TestProbability(0.5f) ? 1 : -1;
		//item.ItemFlags[0] = Random::GenerateAngle(DAMOCLES_SWORD_TURN_RATE_MIN, DAMOCLES_SWORD_TURN_RATE_MAX) * sign;
		item.ItemFlags[1] = TargetType::None;

	}

	static std::vector<int> GetTargetEntityList(const ItemInfo& item)
	{
		auto entityList = std::vector<int>{};

		for (auto& currentEntity : g_Level.Items)
		{
			if (currentEntity.ObjectNumber == ID_ELECTRIC_BALL_IMPACT_POINT &&
				currentEntity.RoomNumber == item.RoomNumber)
			{
				entityList.push_back(currentEntity.Index);
			}
		}

		return entityList;
	}

	static Vector3 GetTargetPosition(ItemInfo& item)
	{
		if (item.ItemFlags[1] != TargetType::None)
		{
			const auto& targetEntity = g_Level.Items[item.ItemFlags[1]];
			return targetEntity.Pose.Position.ToVector3();
		}

		// Failsafe.
		const auto& creature = *GetCreatureInfo(&item);
		return creature.Target.ToVector3();
	}

	static int GetTargetItemNumber(const ItemInfo& item)
	{
		if (item.ObjectNumber == ID_ELECTRIC_BALL && !Objects[ID_ELECTRIC_BALL_IMPACT_POINT].loaded)
			return NO_VALUE;

		auto targetList = GetTargetEntityList(item);
		if (targetList.empty())
			return NO_VALUE;

		if (targetList.size() == 1)
			return targetList[0];
		else
			return targetList[Random::GenerateInt(0, (int)targetList.size() )];
	}

	static void SpawnElectricBallLightning(ItemInfo& item, const Vector3& pos, const CreatureBiteInfo& bite)
	{
		const auto& creature = *GetCreatureInfo(&item);

		auto offset = Vector3::Zero;
		auto origin = GameVector(GetJointPosition(&item, bite), item.RoomNumber);
		auto targetRandom = Vector3::Zero;
		auto target = GameVector(pos, item.RoomNumber);
		constexpr auto SPAWN_RADIUS = BLOCK(0.30f);
		auto orient = Geometry::GetOrientToPoint(origin.ToVector3(), target.ToVector3());
		auto pos1 = GetJointPosition(item, 0, Vector3i(0, 200, 0)).ToVector3();


		static constexpr auto raygunSmokeLife = 16.0f;
		auto distance =  origin -  target;
		if (item.ItemFlags[3] == 0)
		{

			auto random = Random::GenerateFloat(0.20f, 0.60f);
			auto halftarget = origin.ToVector3() + Vector3(random, random, random) * (target.ToVector3() - origin.ToVector3());

			SpawnElectricity(origin.ToVector3(), halftarget, 5, 32, 64, 128, 60, (int)ElectricityFlags::Spline , 4, 12);

			
			ElectricBallData.EnergyArcs = &ElectricityArcs.back();
			auto arc = ElectricityArcs.back();;

			SpawnElectricityGlow(arc.pos1, 68, 32, 32, 64);

			targetRandom = Vector3(Random::GenerateInt(515, 650), 0, Random::GenerateInt(615, 810));
			SpawnElectricity(arc.pos4, target.ToVector3() + targetRandom, 15, 32, 64, 128, 60, (int)ElectricityFlags::Spline | (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 4, 12);
			//SpawnElectricity(arc.pos4, target.ToVector3() + targetRandom, 15, 250, 250, 250, 60, (int)ElectricityFlags::Spline, 2, 12);

			if (Random::GenerateInt(0, 100) > 20)
			{
				halftarget = arc.pos4 + Vector3(random, random, random) * (target.ToVector3() - arc.pos4);

				SpawnElectricity(arc.pos4, halftarget, 10, 32, 64, 128, 60, (int)ElectricityFlags::Spline, 3, 12);

				arc = ElectricityArcs.back();;
			}

			targetRandom = Vector3(Random::GenerateInt(15, 50), 0, Random::GenerateInt(15, 20));
			SpawnElectricity(arc.pos4, target.ToVector3() + targetRandom, 15, 32, 64, 128, 60, (int)ElectricityFlags::Spline | (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 4, 12);
			//SpawnElectricity(arc.pos4, target.ToVector3() + targetRandom, 15, 250, 250, 250, 60, (int)ElectricityFlags::Spline, 2, 12);

			if (Random::GenerateInt(0, 100) > 50)
			{

				halftarget = arc.pos4 + Vector3(random, random, random) * (target.ToVector3() - arc.pos4);

				SpawnElectricity(arc.pos4, halftarget, 10, 32, 64, 128, 60, (int)ElectricityFlags::Spline, 4, 12);

				arc = ElectricityArcs.back();
			}

			targetRandom = Vector3(Random::GenerateInt(225, 650), random, Random::GenerateInt(125, 150));
			SpawnElectricity(arc.pos4, target.ToVector3() + targetRandom, 15, 32, 64, 128, 60, (int)ElectricityFlags::Spline | (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 4, 12);
			//SpawnElectricity(arc.pos4, target.ToVector3() + targetRandom, 15, 250, 250, 250, 60, (int)ElectricityFlags::Spline, 2, 12);

			
			item.ItemFlags[3] = Random::GenerateInt(120,230);
			item.ItemFlags[4] = 120;

			auto sparkOrigin =  GetJointPosition(item, 0, Vector3i(0, 300, 0)).ToVector3();

			//SpawnElectricityGlow(sparkOrigin, 48, 32, 32, 64);
			//SpawnCyborgSpark(sparkOrigin);
			TriggerElectricBallShockwaveAttackSparks(origin.x, origin.y, origin.z, 128, 128, 200, 128);

		}

		if (item.ItemFlags[4] > 90)
		{
			if (item.ItemFlags[4] >  0)
			{
				
				short blue =( item.ItemFlags[4] << 2) + Random::GenerateInt(2,18);
				short green = blue >> 2;
				short red = 0;

				TriggerDynamicLight(pos1.x, pos1.y, pos1.z, (item.ItemFlags[4] +8) / 5, red, green, blue);
				item.ItemFlags[3]--;
				
			}
			else
			{
				//item.ItemFlags[4]++;
			}
			
		}
		if (item.ItemFlags[4] > 0)
			item.ItemFlags[4]--;

		if (item.ItemFlags[4] > 110)
		{
			//Trigger Smoke on flashesend

			auto sphere = BoundingSphere(pos1, SPAWN_RADIUS);
			auto pos2 = Random::GeneratePointOnSphere(sphere);


			SpawnElectricSmoke(pos2, EulerAngles::Identity, raygunSmokeLife);
			SpawnElectricSmoke(pos2, EulerAngles::Identity, raygunSmokeLife);
			SpawnElectricSmoke(pos2, EulerAngles::Identity, raygunSmokeLife);


			pos2 = Random::GeneratePointInSphere(sphere);

			SpawnElectricSmoke(pos2, EulerAngles::Identity, raygunSmokeLife);
			SpawnElectricSmoke(pos2, EulerAngles::Identity, raygunSmokeLife);
			SpawnElectricSmoke(pos2, EulerAngles::Identity, raygunSmokeLife);
		}

		int randomIndex = Random::GenerateInt(0, 100);

		if (randomIndex < 6)
		{
			pos1 = GetJointPosition(item, 0, Vector3i(0, 0, 0)).ToVector3();
			auto sphere2 = BoundingSphere(pos1, BLOCK(0.35f));
			auto pos3 = Random::GeneratePointOnSphere(sphere2);

			SpawnElectricityGlow(pos3, 28, 32, 32, 64);

			SpawnCyborgSpark(pos3);
			TriggerDynamicLight(pos3.x, pos3.y, pos3.z, Random::GenerateInt(4, 8), 31, 63, 127);

			pos1 = GetJointPosition(item, 0, Vector3i(0, 0, 0)).ToVector3();
			sphere2 = BoundingSphere(pos1, BLOCK(0.45f));
			auto sphere3 = BoundingSphere(pos1, BLOCK(0.25f));
			pos3 = Random::GeneratePointOnSphere(sphere2);
			auto pos4 = Random::GeneratePointOnSphere(sphere3);

			SpawnElectricity(pos3, pos4, Random::GenerateInt(8, 16), 32, 64, 128, 24, (int)ElectricityFlags::Spline | (int)ElectricityFlags::ThinOut | (int)ElectricityFlags::ThinIn, 2, 8);

			if (item.ItemFlags[4] > 60)

			{
				pos1 = GetJointPosition(item, 0, Vector3i(0, 100, 0)).ToVector3();
				auto sphere = BoundingSphere(pos1, SPAWN_RADIUS);
				auto pos2 = Random::GeneratePointOnSphere(sphere);

				SpawnElectricSmoke(pos2, EulerAngles::Identity, raygunSmokeLife);
				SpawnElectricSmoke(pos2, EulerAngles::Identity, raygunSmokeLife);
				SpawnElectricSmoke(pos2, EulerAngles::Identity, raygunSmokeLife);
			}

		}

		if (item.ItemFlags[4] > 75 && randomIndex < 37)

		{
			pos1 = GetJointPosition(item, 0, Vector3i(0, 160, 0)).ToVector3();
			auto sphere = BoundingSphere(pos1, BLOCK(0.25f));
			auto pos2 = Random::GeneratePointOnSphere(sphere);

			SpawnElectricSmoke(pos2, EulerAngles::Identity, raygunSmokeLife);
			SpawnElectricSmoke(pos2, EulerAngles::Identity, raygunSmokeLife);
			SpawnElectricSmoke(pos2, EulerAngles::Identity, raygunSmokeLife);
		}
		//auto origin1 = GameVector(Geometry::TranslatePoint(origin.ToVector3(), pos - origin.ToVector3(), distance.ToVector3() / (Random::GenerateInt(0, 4))), item.RoomNumber);
		//SpawnElectricity(offset, target.ToVector3(), Random::GenerateInt(25, 50), 150, 50, 200, 30, (int)(int)(int)ElectricityFlags::ThinIn | (int)(int)ElectricityFlags::ThinOut, 4, 12);
		//SpawnElectricity(offset, target.ToVector3(), Random::GenerateInt(25, 50), 50, 50, 0, 5, (int)(int)(int)ElectricityFlags::ThinIn | (int)(int)ElectricityFlags::ThinOut, 4, 12);
		//TriggerDynamicLight(origin.x, origin.y, origin.z, 20, 0, 255, 0);

		if (item.ItemFlags[3] == 1)
		{

			item.Model.Color = ElectricBallData.Color;
		}
		else if (item.ItemFlags[3] < 30)
		{
			int intensity = 0.01f;
			// Set light mesh color. Model.Color max value is 2.0f.
			item.Model.Color += Color(0.05f, 0.05f, 0.09f, 0.0f); /*Vector4(
				std::clamp<unsigned char>(intensity , 0.0f, 2.0f),
				std::clamp<unsigned char>(intensity  , 0.0f, 2.0f),
				std::clamp<unsigned char>(intensity  , 0.0f, 2.0f),
				0.0f);*/
		}

		if (item.ItemFlags[3] > 28)
		{
			SpawnElectricityGlow(origin.ToVector3(), 108, 22, 22, 54);
		}
		else
		{
			//auto pos5 = origin.ToVector3 - Vector3(0, 300, 0)

			SpawnElectricityGlow(origin.ToVector3(), 128, 102, 102, 204);
			//SpawnElectricity( origin.ToVector3() + targetRandom, origin.ToVector3() - Vector3(0, 500, 0), 15, 32, 64, 128, 60, (int)ElectricityFlags::Spline | (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 4, 12);
			//SpawnElectricity(origin.ToVector3() + targetRandom, origin.ToVector3() - Vector3(0, 500, 0), 15, 32, 64, 128, 60, (int)ElectricityFlags::Spline | (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 4, 12);
			
			SpawnElectricity(origin.ToVector3() - Vector3(0, 1300, 0), origin.ToVector3() - (targetRandom * 8), 15, 32, 64, 128, 30, (int)ElectricityFlags::Spline | (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 4, 12);
			SpawnElectricity(origin.ToVector3() - Vector3(0, 1300, 0), origin.ToVector3() - (targetRandom * 8), 15, 32, 64, 128, 30, (int)ElectricityFlags::Spline | (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 4, 12);

			SpawnElectricity(origin.ToVector3() + Vector3(0, 300, 0), origin.ToVector3() + (targetRandom * 8), 15, 32, 64, 128, 30, (int)ElectricityFlags::Spline | (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 4, 12);
			SpawnElectricity(origin.ToVector3() + Vector3(0, 300, 0), origin.ToVector3() + (targetRandom * 8), 15, 32, 64, 128, 30, (int)ElectricityFlags::Spline | (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 4, 12);

			if (randomIndex < 80)
			{
				pos1 = GetJointPosition(item, 0, Vector3i(0, 0, 0)).ToVector3();
				auto sphere2 = BoundingSphere(pos1, BLOCK(0.45f));
				auto pos3 = Random::GeneratePointOnSphere(sphere2);

				SpawnElectricityGlow(pos3, 28, 32, 32, 64);

				SpawnCyborgSpark(pos3);
				TriggerDynamicLight(pos3.x, pos3.y, pos3.z, Random::GenerateInt(4, 8), 31, 63, 127);

				pos1 = GetJointPosition(item, 0, Vector3i(0, 0, 0)).ToVector3();
				sphere2 = BoundingSphere(pos1, BLOCK(0.45f));
				auto sphere3 = BoundingSphere(pos1, BLOCK(0.25f));
				pos3 = Random::GeneratePointOnSphere(sphere2);
				auto pos4 = Random::GeneratePointOnSphere(sphere3);

				SpawnElectricity(pos3, pos4, Random::GenerateInt(8, 16), 32, 64, 128, 24, (int)ElectricityFlags::Spline | (int)ElectricityFlags::ThinOut | (int)ElectricityFlags::ThinIn, 6, 8);
			}

		}
		

	}

	void ControlElectricBall(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto& item = g_Level.Items[itemNumber];
		auto& object = Objects[item.ObjectNumber];
		auto& creature = *GetCreatureInfo(&item);
		const auto& laraItem = *LaraItem;
		static auto targetPos = Vector3i::Zero;
		
		// Scan for player.
		//if (item.Pose.Position.y < GetPointCollision(item).GetFloorHeight())
		//{
			/*item.Pose.Orientation.y += item.ItemFlags[0];

			// Check vertical position to player.
			if (item.Pose.Position.y >= laraItem.Pose.Position.y)
				return;

			// Check vertical distance.
			float distanceV = laraItem.Pose.Position.y - item.Pose.Position.y;
			if (distanceV > DAMOCLES_SWORD_ACTIVATE_RANGE_VERTICAL)
				return;

			// Check 2D distance.
			float distance2D = Vector2i::Distance(
				Vector2i(item.Pose.Position.x, item.Pose.Position.z),
				Vector2i(laraItem.Pose.Position.x, laraItem.Pose.Position.z));
			if (distance2D > DAMOCLES_SWORD_ACTIVATE_RANGE_2D)
				return;*/

			// Drop sword.
			if (item.ObjectNumber == ID_ELECTRIC_BALL && Objects[ID_ELECTRIC_BALL_IMPACT_POINT].loaded &&
				item.ItemFlags[1] == TargetType::None)
			{
				item.ItemFlags[1] = (short)GetTargetItemNumber(item);
				creature.Target = GetTargetPosition(item);

				targetPos = creature.Target;
			}


			if (item.ItemFlags[1] != TargetType::None)
			{
				SpawnElectricBallLightning(item, targetPos.ToVector3(), ElectricBallBite);
				//item.ItemFlags[2] = creature.Target.;
			}

			if (item.ItemFlags[3] > 0)
				item.ItemFlags[3]--;

			//return;
		//}
	}


	void InitializeElectricBallImpactPoint(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

	}

	


	
}

