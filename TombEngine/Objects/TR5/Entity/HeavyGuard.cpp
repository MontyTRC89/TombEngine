#include "framework.h"
#include "Objects/TR5/Entity/HeavyGuard.h"

#include "Game/animation.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/control/control.h"
#include "Game/control/los.h"
#include "Game/effects/debris.h"
#include "Game/effects/effects.h"
#include "Game/effects/item_fx.h"
#include "Game/effects/lightning.h"
#include "Game/effects/spark.h"
#include "Game/effects/tomb4fx.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Math/Math.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Effects::Items;
using namespace TEN::Effects::Lightning;
using namespace TEN::Effects::Spark;

namespace TEN::Entities::Creatures::TR5
{
	constexpr auto HEAVY_GUARD_RAYGUN_DAMAGE = 250;

	constexpr auto HEAVY_GUARD_HEAD_JOINT_RANGE = BLOCK(0.25f);
	constexpr auto HEAVY_GUARD_ALERT_RANGE		= SQUARE(BLOCK(2));
	constexpr auto HEAVY_GUARD_IDLE_AIM_RANGE	= SQUARE(BLOCK(3));
	constexpr auto HEAVY_GUARD_CLOSE_RANGE		= SQUARE(BLOCK(1));

	constexpr auto HEAVY_GUARD_IDLE_TURN_RATE_MAX = ANGLE(2.0f);
	constexpr auto HEAVY_GUARD_WALK_TURN_RATE_MAX = ANGLE(5.0f);

	constexpr auto HEAVY_GUARD_PLAYER_ALERT_VELOCITY	 = 20.0f;
	constexpr auto HEAVY_GUARD_RAYGUN_PLAYER_BURN_HEALTH = 500;

	const auto HeavyGuardHandJoints = std::vector<int>{ 8, 5 };
	const auto HeavyGuardRaygunLaserOffsets = std::vector<Vector3>
	{
		Vector3(0.0f, 230.0f, 40.0f),
		Vector3(8.0f, 230.0f, 40.0f),
	};

	const auto HeavyGuardHeadBite = BiteInfo(Vector3(0.0f, -200.0f, 0.0f), 2);

	enum HeavyGuardState
	{
		// No state 0.
		HEAVY_GUARD_STATE_IDLE = 1,
		HEAVY_GUARD_STATE_WALK_FORWARD = 2,
		HEAVY_GUARD_STATE_WALK_RAYGUN_ATTACK_RIGHT = 3,
		HEAVY_GUARD_STATE_WALK_RAYGUN_ATTACK_LEFT = 4,
		HEAVY_GUARD_STATE_IDLE_TO_AIM = 5,
		HEAVY_GUARD_STATE_DUAL_RAYGUN_ATTACK = 6,
		HEAVY_GUARD_STATE_DEATH = 7,
		HEAVY_GUARD_STATE_TURN_180 = 8,
		HEAVY_GUARD_STATE_MISFIRE = 9,
		HEAVY_GUARD_STATE_RATTLE_RAYGUN = 10,
		HEAVY_GUARD_STATE_FALL_START = 11,
		HEAVY_GUARD_STATE_FALL_CONTINUE = 12,
		HEAVY_GUARD_STATE_FALL_DEATH = 13
	};

	enum HeavyGuardAnim
	{
		HEAVY_GUARD_ANIM_WALK_FORWARD = 0,
		HEAVY_GUARD_ANIM_WALK_RAYGUN_ATTACK_RIGHT = 1,
		HEAVY_GUARD_ANIM_WALK_RAYGUN_ATTACK_LEFT = 2,
		HEAVY_GUARD_ANIM_DEATH = 3,
		HEAVY_GUARD_ANIM_IDLE_TO_WALK_FORWARD = 4,
		HEAVY_GUARD_ANIM_WALK_FORWARD_TO_IDLE = 5,
		HEAVY_GUARD_ANIM_IDLE = 6,
		HEAVY_GUARD_ANIM_IDLE_TO_AIM = 7,
		HEAVY_GUARD_ANIM_AIM = 8,
		HEAVY_GUARD_ANIM_DUAL_RAYGUN_ATTACK = 9,
		HEAVY_GUARD_ANIM_AIM_TO_IDLE = 10,
		HEAVY_GUARD_ANIM_TURN_180 = 11,
		HEAVY_GUARD_ANIM_MISFIRE = 12,
		HEAVY_GUARD_ANIM_RATTLE_RAYGUN = 13,
		HEAVY_GUARD_ANIM_FALL_DEATH_START = 14,
		HEAVY_GUARD_ANIM_FALL_DEATH_CONTINUE = 15,
		HEAVY_GUARD_ANIM_FALL_DEATH_END = 16
	};

	// TODO: Demagic these.
	enum class HeavyGuardFlags
	{

	};

	void InitialiseHeavyGuard(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];

		item.ItemFlags[7] = HEAVY_GUARD_HEAD_JOINT_RANGE;
		SetAnimation(&item, HEAVY_GUARD_ANIM_IDLE);
	}

	void HeavyGuardControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto& item = g_Level.Items[itemNumber];
		auto& creature = *GetCreatureInfo(&item);

		short headingAngle = 0;
		auto headOrient = EulerAngles::Zero;
		auto torsoOrient = EulerAngles::Zero;

		AI_INFO ai;
		CreatureAIInfo(&item, &ai);

		if (item.ItemFlags[0] || item.ItemFlags[1])
		{
			for (int i = 0; i < 2; i++)
			{
				if (item.ItemFlags[i])
				{
					FireHeavyGuardRaygun(item, i, 1);

					if (item.ItemFlags[i] > 0)
					{
						auto pos1 = GetJointPosition(&item, HeavyGuardHandJoints[i], HeavyGuardRaygunLaserOffsets[i]);
						short blue = item.ItemFlags[i] << 4;
						short green = blue >> 2;
						short red = 0;

						TriggerDynamicLight(pos1.x, pos1.y, pos1.z, item.ItemFlags[i] + 8, red, green, blue);
						item.ItemFlags[i]--;
					}
					else
					{
						item.ItemFlags[i]++;
					}
				}
			}
		}

		if (item.HitPoints <= 0)
		{
			if (item.Animation.ActiveState != HEAVY_GUARD_STATE_DEATH && item.TriggerFlags != 1)
			{
				SetAnimation(&item, HEAVY_GUARD_ANIM_DEATH);
			}
			else if (item.TriggerFlags == 1)
			{
				switch (item.Animation.ActiveState)
				{
				case HEAVY_GUARD_STATE_FALL_START:
				{
					int frame = item.Animation.FrameNumber;
					int frameStart = g_Level.Anims[item.Animation.AnimNumber].frameBase;

					if (frame == GetFrameNumber(&item, 48) || frame == GetFrameNumber(&item, 15))
					{
						short roomNumber = item.RoomNumber;
						GetFloor(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, &roomNumber),
						TestTriggers(&item, true, 0);
					}

					break;
				}

				case HEAVY_GUARD_STATE_FALL_CONTINUE:
					item.Animation.IsAirborne = true;

					if (item.Pose.Position.y >= item.Floor)
					{
						item.Pose.Position.y = item.Floor;
						item.Animation.TargetState = HEAVY_GUARD_STATE_FALL_DEATH;
						item.Animation.IsAirborne = false;
						item.Animation.Velocity.y = 0.0f;
						CreatureEffect2(&item, HeavyGuardHeadBite, 10, item.Pose.Orientation.y, DoBloodSplat);
					}

					break;

				case HEAVY_GUARD_STATE_FALL_DEATH:
					item.Pose.Position.y = item.Floor;
					break;

				default:
					SetAnimation(&item, HEAVY_GUARD_ANIM_FALL_DEATH_START);
					creature.LOT.IsJumping = true;
					break;
				}
			}
		}
		else
		{
			if (item.AIBits)
				GetAITarget(&creature);
			else
				creature.Enemy = LaraItem;

			CreatureAIInfo(&item, &ai);

			AI_INFO laraAI;
			if (creature.Enemy->IsLara())
			{
				laraAI.angle = ai.angle;
				laraAI.distance = ai.distance;
			}
			else
			{
				int dx = LaraItem->Pose.Position.x - item.Pose.Position.x;
				int dz = LaraItem->Pose.Position.z - item.Pose.Position.z;
				laraAI.angle = phd_atan(dz, dx) - item.Pose.Orientation.y;
				laraAI.distance = SQUARE(dx) + SQUARE(dz);
			}

			GetCreatureMood(&item, &ai, creature.Enemy != LaraItem);
			CreatureMood(&item, &ai, creature.Enemy != LaraItem);

			headingAngle = CreatureTurn(&item, creature.MaxTurn);

			if (((laraAI.distance < HEAVY_GUARD_ALERT_RANGE &&
				(laraAI.angle < ANGLE(90.0f) && laraAI.angle > ANGLE(-90.0f) ||
					LaraItem->Animation.Velocity.z > HEAVY_GUARD_PLAYER_ALERT_VELOCITY)) ||
				item.HitStatus ||
				TargetVisible(&item, &laraAI)) &&
				abs(item.Pose.Position.y - LaraItem->Pose.Position.y) < BLOCK(1.5f))
			{
				creature.Enemy = LaraItem;
				AlertAllGuards(itemNumber);
			}

			switch (item.Animation.ActiveState)
			{
			case HEAVY_GUARD_STATE_IDLE:
				creature.MaxTurn = 0;
				creature.Flags = 0;

				if (!(item.AIBits & GUARD))
				{
					if (abs(ai.angle) < HEAVY_GUARD_IDLE_TURN_RATE_MAX)
					{
						item.Pose.Orientation.y += ai.angle;
					}
					else
					{
						if (ai.angle >= 0)
							item.Pose.Orientation.y += HEAVY_GUARD_IDLE_TURN_RATE_MAX;
						else
							item.Pose.Orientation.y -= HEAVY_GUARD_IDLE_TURN_RATE_MAX;
					}

					headOrient.y = laraAI.angle / 2;
					torsoOrient.x = ai.xAngle / 2;
					torsoOrient.y = laraAI.angle / 2;
				}

				if (item.AIBits & GUARD)
				{
					headOrient.y = AIGuard(&creature);
				}
				else
				{
					if (laraAI.angle > ANGLE(112.0f) || laraAI.angle < ANGLE(-112.0f))
					{
						item.Animation.TargetState = HEAVY_GUARD_STATE_TURN_180;
					}
					else if (!Targetable(&item, &ai))
					{
						if (item.TriggerFlags != 1 && ai.distance > HEAVY_GUARD_CLOSE_RANGE)
							item.Animation.TargetState = HEAVY_GUARD_STATE_WALK_FORWARD;
					}
					else if (ai.distance < HEAVY_GUARD_IDLE_AIM_RANGE || ai.zoneNumber != ai.enemyZone)
					{
						item.Animation.TargetState = HEAVY_GUARD_STATE_IDLE_TO_AIM;
					}
					else if (item.AIBits != MODIFY)
					{
						if (item.TriggerFlags != 1)
							item.Animation.TargetState = HEAVY_GUARD_STATE_WALK_FORWARD;
					}
				}

				break;

			case HEAVY_GUARD_STATE_WALK_FORWARD:
				creature.MaxTurn = HEAVY_GUARD_WALK_TURN_RATE_MAX;

				if (ai.distance < HEAVY_GUARD_CLOSE_RANGE)
				{
					item.Animation.TargetState = HEAVY_GUARD_STATE_IDLE;
					break;
				}

				if (Targetable(&item, &ai) &&
					laraAI.angle < ANGLE(33.0f) && laraAI.angle > ANGLE(-33.0f))
				{
					if (item.Animation.FrameNumber >= GetFrameNumber(&item, 29))
						item.Animation.TargetState = HEAVY_GUARD_STATE_WALK_RAYGUN_ATTACK_RIGHT;
					else
						item.Animation.TargetState = HEAVY_GUARD_STATE_WALK_RAYGUN_ATTACK_LEFT;
				}
				else
				{
					if (laraAI.angle > ANGLE(112.0f) || laraAI.angle < ANGLE(-112.0f))
						item.Animation.TargetState = HEAVY_GUARD_STATE_IDLE;
					else
						item.Animation.TargetState = HEAVY_GUARD_STATE_WALK_FORWARD;
				}

				break;

			case HEAVY_GUARD_STATE_WALK_RAYGUN_ATTACK_RIGHT:
			case HEAVY_GUARD_STATE_WALK_RAYGUN_ATTACK_LEFT:
				headOrient.y = laraAI.angle;

				if (item.Animation.FrameNumber == g_Level.Anims[item.Animation.AnimNumber].frameBase)
				{
					if (item.Animation.ActiveState == HEAVY_GUARD_STATE_WALK_RAYGUN_ATTACK_LEFT)
						FireHeavyGuardRaygun(item, 0, 0);
					else
						FireHeavyGuardRaygun(item, 1, 0);
				}

				break;

			case HEAVY_GUARD_STATE_IDLE_TO_AIM:
				creature.Flags = 0;
				creature.MaxTurn = 0;
				headOrient.y = laraAI.angle / 2;
				torsoOrient.x = ai.xAngle / 2;
				torsoOrient.y = laraAI.angle / 2;

				if (abs(ai.angle) < HEAVY_GUARD_IDLE_TURN_RATE_MAX)
				{
					item.Pose.Orientation.y += ai.angle;
				}
				else if (ai.angle >= 0)
				{
					item.Pose.Orientation.y += HEAVY_GUARD_IDLE_TURN_RATE_MAX;
				}
				else
				{
					item.Pose.Orientation.y -= HEAVY_GUARD_IDLE_TURN_RATE_MAX;
				}

				if (item.TriggerFlags == 2)
				{
					item.Animation.TargetState = HEAVY_GUARD_STATE_MISFIRE;
				}
				else if (item.TriggerFlags == 3)
				{
					item.Animation.TargetState = HEAVY_GUARD_STATE_RATTLE_RAYGUN;
				}
				else if (Targetable(&item, &ai) &&
					(ai.distance <= HEAVY_GUARD_IDLE_AIM_RANGE || ai.distance < HEAVY_GUARD_CLOSE_RANGE))
				{
					item.Animation.TargetState = HEAVY_GUARD_STATE_DUAL_RAYGUN_ATTACK;
				}
				else
				{
					item.Animation.TargetState = HEAVY_GUARD_STATE_IDLE;
				}

				break;

			case HEAVY_GUARD_STATE_DUAL_RAYGUN_ATTACK:
				headOrient.y = laraAI.angle / 2;
				torsoOrient.x = ai.xAngle;
				torsoOrient.y = laraAI.angle / 2;

				if (abs(ai.angle) < HEAVY_GUARD_IDLE_TURN_RATE_MAX)
				{
					item.Pose.Orientation.y += ai.angle;
				}
				else
				{
					if (ai.angle >= 0)
						item.Pose.Orientation.y += HEAVY_GUARD_IDLE_TURN_RATE_MAX;
					else
						item.Pose.Orientation.y -= HEAVY_GUARD_IDLE_TURN_RATE_MAX;
				}

				if (item.Animation.FrameNumber == g_Level.Anims[item.Animation.AnimNumber].frameBase + 17)
				{
					FireHeavyGuardRaygun(item, 0, 0);
					FireHeavyGuardRaygun(item, 1, 0);
				}

				break;

			case HEAVY_GUARD_STATE_TURN_180:
				creature.Flags = 0;
				creature.MaxTurn = 0;

				if (ai.angle < 0)
					item.Pose.Orientation.y += HEAVY_GUARD_IDLE_TURN_RATE_MAX;
				else
					item.Pose.Orientation.y -= HEAVY_GUARD_IDLE_TURN_RATE_MAX;

				if (TestLastFrame(&item))
					item.Pose.Orientation.y += ANGLE(180.0f);

				break;

			case HEAVY_GUARD_STATE_MISFIRE:
				headOrient.y = laraAI.angle / 2;
				torsoOrient.x = ai.xAngle / 2;
				torsoOrient.y = laraAI.angle / 2;

				if (item.Animation.FrameNumber == (g_Level.Anims[item.Animation.AnimNumber].frameBase + 18))
				{
					FireHeavyGuardRaygun(item, 0, 0);
					item.ItemFlags[1] = -16;
					item.TriggerFlags = 3;
				}

				break;

			case HEAVY_GUARD_STATE_RATTLE_RAYGUN:
				item.TriggerFlags = 0;
				break;
			}
		}

		CreatureJoint(&item, 0, torsoOrient.y);
		CreatureJoint(&item, 1, torsoOrient.x);
		CreatureJoint(&item, 2, headOrient.y);
		CreatureAnimation(itemNumber, headingAngle, 0);
	}

	void HeavyGuardHit(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, bool isExplosive, int jointIndex)
	{
		if (pos.has_value())
		{
			if (jointIndex == 2 )
			{
				DoBloodSplat(pos->x, pos->y, pos->z, 10, source.Pose.Orientation.y, pos->RoomNumber);
				DoDamage(&target, INT_MAX);
			}
			else
			{
				SoundEffect(SFX_TR4_BADDY_SWORD_RICOCHET, &target.Pose);
				TriggerRicochetSpark(*pos, source.Pose.Orientation.y, 3, 0);
			}
		}
	}

	void FireHeavyGuardRaygun(ItemInfo& item, bool isRight, bool spawnLaser)
	{
		const auto& creature = *GetCreatureInfo(&item);

		auto pos1 = GetJointPosition(&item, HeavyGuardHandJoints[isRight], HeavyGuardRaygunLaserOffsets[isRight]).ToVector3();
		auto pos2 = GetJointPosition(
			&item,
			HeavyGuardHandJoints[isRight],
			Vector3i(
				HeavyGuardRaygunLaserOffsets[isRight].x,
				HeavyGuardRaygunLaserOffsets[isRight].y + BLOCK(4),
				HeavyGuardRaygunLaserOffsets[isRight].z)).ToVector3();

		auto orient = Geometry::GetOrientToPoint(pos1, pos2);

		if (spawnLaser)
		{
			SpawnRaygunLaser(Pose(pos1, orient), abs(item.ItemFlags[isRight]));
			return;
		}

		SpawnHelicalLaser(pos1, pos2);

		item.ItemFlags[isRight] = 16;
		SpawnRaygunLaser(Pose(pos1, orient), 16);
		SpawnRaygunLaser(Pose(pos1, orient), 16);
		SpawnRaygunLaser(Pose(pos1, orient), 16);

		auto origin = GameVector(pos1, item.RoomNumber);
		auto target = GameVector(pos2);
		auto hitPos = Vector3i::Zero;
		MESH_INFO* hitJoint = nullptr;

		if (ObjectOnLOS2(&origin, &target, &hitPos, &hitJoint, ID_LARA) == GetLaraInfo(creature.Enemy)->ItemNumber)
		{
			if (LaraItem->HitPoints <= HEAVY_GUARD_RAYGUN_PLAYER_BURN_HEALTH)
			{
				ItemCustomBurn(LaraItem, Vector3(0.2f, 0.4f, 1.0f), Vector3(0.2f, 0.3f, 0.8f), 1 * FPS);
				DoDamage(LaraItem, INT_MAX);
			}
			else
			{
				DoDamage(LaraItem, 250);
			}
		}
	}

	void SpawnRaygunLaser(const Pose& pos, float life)
	{
		auto* sptr = GetFreeParticle();

		sptr->on = true;

		sptr->sB = (((GetRandomControl() & 127) + 128) * life) / 16;
		sptr->sR = sptr->sB - (sptr->sB >> 2);
		sptr->sG = sptr->sR;
		sptr->dR = 0;
		sptr->dB = (((GetRandomControl() & 127) + 32) * life) / 16;
		sptr->dG = sptr->dB >> 2;
		sptr->colFadeSpeed = 8 + (GetRandomControl() & 3);
		sptr->fadeToBlack = 8;
		sptr->sLife = sptr->life = (GetRandomControl() & 3) + 24;

		sptr->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		sptr->x = pos.Position.x;
		sptr->y = pos.Position.y;
		sptr->z = pos.Position.z;

		int size = ((life * 64) * phd_cos(TO_RAD(pos.Orientation.x))) / 5;

		sptr->xVel = Random::GenerateInt(-128, 128) / 5;
		sptr->yVel = life * 16 / 5;
		sptr->zVel = Random::GenerateInt(-128, 128) / 5;
		sptr->friction = 0;
		sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		sptr->rotAng = GetRandomControl() & SECTOR(4);
		sptr->rotAdd = (GetRandomControl() & 127) - 64;
		sptr->gravity = (GetRandomControl() & 31) + 32;
		sptr->maxYvel = 0;
		sptr->scalar = 1;
		sptr->size = sptr->sSize = size;
		sptr->dSize = 1;
	}
}
