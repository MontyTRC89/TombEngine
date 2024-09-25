#include "framework.h"
#include "Objects/TR1/Trap/ElectricBall.h"

#include "Game/camera.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/Point.h"
#include "Game/effects/effects.h"
#include "Game/effects/tomb4fx.h"
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
#include "Objects/TR5/Entity/tr5_roman_statue.h"

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
		Target, 
		SwitchTarget
	};

	struct ElectricBallInfo
	{
		Vector3i Position = Vector3i::Zero;
		Color Colorw;
		Electricity* EnergyArcs = {};
		Electricity* MainEnergyArc = {};
		unsigned int Count = 0;
	};

	ElectricBallInfo EldectricBallData;

	constexpr auto ELECTRIC_BALL_LIGHTNING_DAMAGE = 350;
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

		//InitializeCreature(itemNumber);
		auto& item = g_Level.Items[itemNumber];
		CheckForRequiredObjects(item);

		EldectricBallData.Colorw = item.Model.Color;

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
				currentEntity.RoomNumber == item.RoomNumber &&
				currentEntity.TriggerFlags == item.TriggerFlags)
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
			return targetList[Random::GenerateInt(0, (int)targetList.size() - 1 )];
	}

	void SpawnElectricBallLightning(ItemInfo& item, const Vector3& pos, const CreatureBiteInfo& bite)
	{
		//const auto& creature = *GetCreatureInfo(&item);


	
		auto offset = Vector3::Zero;
		auto origin = GameVector(GetJointPosition(&item, bite), item.RoomNumber);
		auto targetRandom = Vector3i::Zero;
		auto target = GameVector(pos, item.RoomNumber);
		constexpr auto SPAWN_RADIUS = BLOCK(0.30f);
		auto orient = Geometry::GetOrientToPoint(origin.ToVector3(), target.ToVector3());
		auto pos1 = GetJointPosition(item, 0, Vector3i(0, 200, 0)).ToVector3();
		auto targetNew = GameVector::Zero;

		static constexpr auto raygunSmokeLife = 16.0f;
		auto distance =  origin -  target;
		if (item.ItemFlags[3] == 0)
		{

			auto random = Random::GenerateFloat(0.20f, 0.60f);
			auto halftarget = origin.ToVector3() + Vector3(random, random, random) * (target.ToVector3() - origin.ToVector3());

			SpawnElectricity(origin.ToVector3(), halftarget, 5, 32, 64, 128, 60, (int)ElectricityFlags::Spline , 8, 12);//4
			//SpawnElectricity(origin.ToVector3(), halftarget, 5, 32, 64, 128, 60, (int)ElectricityFlags::Spline , 4, 12);
			
			//ElectricBallData.EnergyArcs = &ElectricityArcs.back();
			auto arc = ElectricityArcs.back();;

			SpawnElectricityGlow(arc.pos1, 68, 32, 32, 64);


			targetRandom = Vector3i(Random::GenerateInt(515, 650), 0, Random::GenerateInt(615, 810));
			targetNew = GameVector((target.ToVector3() - Vector3(0, 120, 0)) + targetRandom.ToVector3(), item.RoomNumber);

			SpawnElectricity(arc.pos4, target.ToVector3() + targetRandom.ToVector3(), 15, 32, 64, 128, 60, (int)ElectricityFlags::Spline | (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 4, 12);//4
			TriggerRicochetSpark(targetNew, 12, 12, Color(32, 64, 128, 1.0f));
			SpawnElectricSmoke((target.ToVector3() - Vector3(0, 120, 0)) + targetRandom.ToVector3(), EulerAngles::Identity, raygunSmokeLife);
			//TriggerShockwave((Pose*)&targetNew, 1, 15, 3, 32, 64, 128, 80, EulerAngles::Identity, 8, false, true, true, (int)ShockwaveStyle::Invisible);

			SpawnElectricity((target.ToVector3() - Vector3(0, 120, 0)) + targetRandom.ToVector3(), target.ToVector3() + targetRandom.ToVector3(), 15, 32, 64, 128, 160, (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 12, 12);
			SpawnElectricity((target.ToVector3() - Vector3(0, 120, 0)) + targetRandom.ToVector3(), target.ToVector3() + targetRandom.ToVector3(), 15, 32, 64, 128, 160, (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 12, 12);

			//SpawnElectricity(arc.pos4, target.ToVector3() + targetRandom, 15, 32, 64, 128, 60, (int)ElectricityFlags::Spline | (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 8, 12);//4

			if (Random::GenerateInt(0, 100) > 20)
			{
				halftarget = arc.pos4 + Vector3(random, random, random) * (target.ToVector3() - arc.pos4);

				SpawnElectricity(arc.pos4, halftarget, 10, 32, 64, 128, 60, (int)ElectricityFlags::Spline, 8, 12);//3

				arc = ElectricityArcs.back();
			}
			//Main Arc
			targetRandom = Vector3i(Random::GenerateInt(5, 20), 0, Random::GenerateInt(5, 20));
			targetNew = GameVector((target.ToVector3() - Vector3(0, 120, 0)) + targetRandom.ToVector3(), item.RoomNumber);

			SpawnElectricity(arc.pos4, target.ToVector3() + targetRandom.ToVector3(), 15, 32, 64, 128, 60, (int)ElectricityFlags::Spline | (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 8, 12);//
			EldectricBallData.MainEnergyArc = &ElectricityArcs.back();
			
			TriggerRicochetSpark(targetNew, 12, 12, Color(32, 64, 128, 1.0f));
			SpawnElectricSmoke((target.ToVector3() - Vector3(0, 120, 0)) + targetRandom.ToVector3(), EulerAngles::Identity, raygunSmokeLife);
			//TriggerShockwave((Pose*)&targetNew, 1, 15, 3, 32, 64, 128, 80, EulerAngles::Identity, 8, false, true, true, (int)ShockwaveStyle::Invisible);
			SpawnElectricity((target.ToVector3() - Vector3(0,220,0)  ) + targetRandom.ToVector3(), target.ToVector3() + targetRandom.ToVector3(), 15, 32, 64, 128, 160,  (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 12, 12);
			SpawnElectricity((target.ToVector3() - Vector3(0, 120, 0)) + targetRandom.ToVector3(), target.ToVector3() + targetRandom.ToVector3(), 15, 32, 64, 128, 160,  (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 12, 12);

			
			

			if (Random::GenerateInt(0, 100) > 50)
			{

				halftarget = arc.pos4 + Vector3(random, random, random) * (target.ToVector3() - arc.pos4);

				SpawnElectricity(arc.pos4, halftarget, 10, 32, 64, 128, 60, (int)ElectricityFlags::Spline, 8, 12);//4

				arc = ElectricityArcs.back();
			}

			targetRandom = Vector3(Random::GenerateInt(225, 650), random, Random::GenerateInt(125, 150));
			targetNew = GameVector((target.ToVector3() - Vector3(0, 120, 0)) + targetRandom.ToVector3(), item.RoomNumber);

			SpawnElectricity(arc.pos4, target.ToVector3() + targetRandom.ToVector3(), 15, 32, 64, 128, 60, (int)ElectricityFlags::Spline | (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 8, 12);//4
			TriggerRicochetSpark(targetNew, 12, 12, Color(32, 64, 128, 1.0f));
			SpawnElectricSmoke((target.ToVector3() - Vector3(0, 120, 0)) + targetRandom.ToVector3(), EulerAngles::Identity, raygunSmokeLife);
			//TriggerShockwave((Pose*)&targetNew, 1, 15, 3, 32, 64, 128, 80, EulerAngles::Identity, 8, false, true, true, (int)ShockwaveStyle::Invisible);
			SpawnElectricity((target.ToVector3() - Vector3(0, 120, 0)) + targetRandom.ToVector3(), target.ToVector3() + targetRandom.ToVector3(), 15, 32, 64, 128, 160, (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 12, 12);
			SpawnElectricity((target.ToVector3() - Vector3(0, 120, 0)) + targetRandom.ToVector3(), target.ToVector3() + targetRandom.ToVector3(), 15, 32, 64, 128, 160, (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 12, 12);

			//SpawnElectricity(arc.pos4, target.ToVector3() + targetRandom, 15, 250, 250, 250, 60, (int)ElectricityFlags::Spline, 2, 12);

			
			item.ItemFlags[3] = Random::GenerateInt(120,230);
			item.ItemFlags[4] = 120;
			item.ItemFlags[1] = TargetType::None;
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

		SpawnElectricEffect(item, 0, Vector3i(0, 0, 0), BLOCK(0.35f), BLOCK(0.45f), BLOCK(0.25f), 6, Vector3::Zero);

		if (randomIndex < 6)
		{
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

		 if (item.ItemFlags[3] < 30)
		{
			int intensity = 0.01f;

			if (item.Model.Color.x < 4.0f)
				item.Model.Color.x += 0.05f;
			else
				item.Model.Color.x = 4.0f;

			if (item.Model.Color.y < 4.0f)
				item.Model.Color.y += 0.05f;
			else
				item.Model.Color.y = 4.0f;

			if (item.Model.Color.z < 4.0f)
				item.Model.Color.z += 0.09f;
			else
				item.Model.Color.z = 4.0f;
		}
		else if (item.ItemFlags[4] > 80)
		{
			 if (item.Model.Color.x <= EldectricBallData.Colorw.x)
				 item.Model.Color.x = EldectricBallData.Colorw.x;
			 else
				 item.Model.Color.x -= 0.1f;

			 if (item.Model.Color.y <= EldectricBallData.Colorw.y)
				 item.Model.Color.y = EldectricBallData.Colorw.y;
			 else
				 item.Model.Color.y -= 0.1f;

			 if (item.Model.Color.z <= EldectricBallData.Colorw.z)
				 item.Model.Color.z = EldectricBallData.Colorw.z;
			 else
				 item.Model.Color.z -= 0.1f;
		}
		
		if (item.ItemFlags[3] > 28)
		{
			SpawnElectricityGlow(origin.ToVector3(), 108, 22, 22, 54);
		}
		else
		{
			
			SpawnElectricityGlow(origin.ToVector3(), 128, 102, 102, 204);
			
			SpawnElectricity(origin.ToVector3() - Vector3(0, 1300, 0), origin.ToVector3() - (targetRandom.ToVector3() * 8), 15, 32, 64, 128, 30, (int)ElectricityFlags::Spline | (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 8, 12);//4
			SpawnElectricity(origin.ToVector3() - Vector3(0, 1300, 0), origin.ToVector3() - (targetRandom.ToVector3() * 8), 15, 32, 64, 128, 30, (int)ElectricityFlags::Spline | (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 8, 12);//4
			SpawnElectricity(origin.ToVector3() + Vector3(0, 300, 0), origin.ToVector3() + (targetRandom.ToVector3() * 8), 15, 32, 64, 128, 30, (int)ElectricityFlags::Spline | (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 8, 12);//4
			SpawnElectricity(origin.ToVector3() + Vector3(0, 300, 0), origin.ToVector3() + (targetRandom.ToVector3() * 8), 15, 32, 64, 128, 30, (int)ElectricityFlags::Spline | (int)(int)ElectricityFlags::ThinOut | (int)ElectricityFlags::MoveEnd, 8, 12);//4

			SpawnElectricEffect(item, 0, Vector3i(0, 0, 0), BLOCK(0.45f), BLOCK(0.45f), BLOCK(0.25f), 80, Vector3::Zero);

		}
		
		if (EldectricBallData.MainEnergyArc != nullptr)
		{
			if (EldectricBallData.MainEnergyArc->life > 0)
			{
				auto hitPos = Vector3i::Zero;
				if (ObjectOnLOS2(&origin, &target, &hitPos, nullptr, ID_LARA) == item.ItemFlags[4])
				{
					if (LaraItem->HitPoints <= ELECTRIC_BALL_LIGHTNING_DAMAGE)
					{
						ItemElectricBurn(LaraItem);
						DoDamage(LaraItem, ELECTRIC_BALL_LIGHTNING_DAMAGE);
					}
					else
					{
						DoDamage(LaraItem, ELECTRIC_BALL_LIGHTNING_DAMAGE);
					}
				}

			}
			//else
			//{
				//electricBall.MainEnergyArc = nullptr;
			//}
		}

	}

	void ControlElectricBall(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		


		auto& item = g_Level.Items[itemNumber];

		auto& object = Objects[item.ObjectNumber];
		const auto& laraItem = *LaraItem;
		//auto target = GameVector::Zero;
		static auto targetPos = Vector3i::Zero;

		AI_INFO ai;
		CreatureAIInfo(&item, &ai);

		if (ai.distance > SQUARE(BLOCK(7)) )
		{
			item.ItemFlags[3] = 70;
			item.ItemFlags[4] = 70;
			item.Model.Color = EldectricBallData.Colorw;
			item.ItemFlags[1] = TargetType::None;
			return;
		}
		
		/*ai.ahead = true;
		if (deltaAngle <= ANGLE(-90.0f) || deltaAngle >= ANGLE(90.0f))
			ai.ahead = false;*/
		auto targetPlayer = GameVector(GetJointPosition(LaraItem, LM_TORSO), LaraItem->RoomNumber);;
		auto origin = GameVector(GetJointPosition(item, 0), item.RoomNumber);
		bool los = LOS(&origin, &targetPlayer);
		FloorInfo* floor = GetFloor(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, &item.RoomNumber);

			// Drop sword.
		if (ai.distance < SQUARE(BLOCK(3)) && los)
		{
			
				//target = GameVector(GetJointPosition(LaraItem, LM_TORSO), Laraitem.RoomNumber);
			targetPos = Vector3(LaraItem->Pose.Position.x, GetFloorHeight(floor, item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z), LaraItem->Pose.Position.z);
				
			item.ItemFlags[1] = TargetType::Target;

				item.ItemFlags[5] = LaraItem->Index;
		}
		
		else if ((item.ObjectNumber == ID_ELECTRIC_BALL &&
			Objects[ID_ELECTRIC_BALL_IMPACT_POINT].loaded &&
			item.ItemFlags[1] == TargetType::None))
		{
			item.ItemFlags[1] = (short)GetTargetItemNumber(item);
			targetPos = GetTargetPosition(item);
		
			item.ItemFlags[5] = g_Level.Items[item.ItemFlags[1]].Index;
			//target = GameVector(GetJointPosition(&g_Level.Items[item.ItemFlags[1]], 0), item.RoomNumber);
			//targetPos = creature.Target;
		}

		
		//target = GameVector(GetJointPosition(&g_Level.Items[item.ItemFlags[1]]

		//los = LOS(&origin, &target);

			if (item.ItemFlags[1] != TargetType::None)
			{

				SpawnElectricBallLightning(item, targetPos.ToVector3(), ElectricBallBite);

				
			}
			else 
			{
				item.Model.Color = EldectricBallData.Colorw;
				item.ItemFlags[1] = TargetType::None;
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

