#include "framework.h"
#include "Objects/TR2/Entity/Dragon.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Specific/Input/Input.h"

using namespace TEN::Input;
using namespace TEN::Math;

// NOTES:
// item.ItemFlags[0]: Back segment item number.
// item.ItemFlags[1]: Timer for temporary defeat and death in frame time.

namespace TEN::Entities::Creatures::TR2
{
	constexpr auto DRAGON_SWIPE_ATTACK_DAMAGE = 250;
	constexpr auto DRAGON_CONTACT_DAMAGE	  = 10;

	constexpr auto DRAGON_NEAR_RANGE = SQUARE(BLOCK(3));
	constexpr auto DRAGON_IDLE_RANGE = SQUARE(BLOCK(6));

	constexpr auto DRAGON_WALK_TURN_RATE_MAX   = ANGLE(2.0f);
	constexpr auto DRAGON_TURN_THRESHOLD_ANGLE = ANGLE(1.0f);

	constexpr auto DRAGON_LIVE_TIME	  = 30 * 11;
	constexpr auto DRAGON_ALMOST_LIVE = 100;

	constexpr auto DRAGON_DISTANCE_NEAR = 900;
	constexpr auto DRAGON_DISTANCE_FAR	= 2300;
	constexpr auto DRAGON_MID	= ((DRAGON_DISTANCE_NEAR + DRAGON_DISTANCE_FAR) / 2);
	constexpr auto DRAGON_LCOL	= -CLICK(2);
	constexpr auto DRAGON_RCOL	= CLICK(2);

	const auto DragonMouthBite = CreatureBiteInfo(Vector3(35.0f, 171.0f, 1168.0f), 12);
	const auto DragonBackSpineJoints		= std::vector<unsigned int>{ 21, 22, 23 };
	const auto DragonSwipeAttackJointsLeft	= std::vector<unsigned int>{ 24, 25, 26, 27, 28, 29, 30 };
	const auto DragonSwipeAttackJointsRight = std::vector<unsigned int>{ 1, 2, 3, 4, 5, 6, 7 };

	enum class DragonLightEffectType
	{
		Yellow,
		Red
	};

	enum DragonState
	{
		// No state 0.
		DRAGON_STATE_WALK = 1,
		DRAGON_STATE_MOVE_LEFT = 2,
		DRAGON_STATE_MOVE_RIGHT = 3,
		DRAGON_STATE_AIM_1 = 4,
		DRAGON_STATE_FIRE_1 = 5,
		DRAGON_STATE_IDLE = 6,
		DRAGON_STATE_TURN_LEFT = 7,
		DRAGON_STATE_TURN_RIGHT = 8,
		DRAGON_STATE_SWIPE_ATTACK_LEFT = 9,
		DRAGON_STATE_SWIPE_ATTACK_RIGHT = 10,
		DRAGON_STATE_DEFEAT = 11
	};

	enum DragonAnim
	{
		DRAGON_ANIM_WALK = 0,
		DRAGON_ANIM_WALK_TO_MOVE_LEFT = 1,
		DRAGON_ANIM_MOVE_LEFT = 2,
		DRAGON_ANIM_MOVE_LEFT_TO_WALK = 3,
		DRAGON_ANIM_WALK_TO_MOVE_RIGHT = 4,
		DRAGON_ANIM_MOVE_RIGHT = 5,
		DRAGON_ANIM_MOVE_RIGHT_TO_WALK = 6,
		DRAGON_ANIM_WALK_TO_IDLE = 7,
		DRAGON_ANIM_IDLE = 8,
		DRAGON_ANIM_IDLE_TO_WALK = 9,
		DRAGON_ANIM_IDLE_TO_FIRE = 10,
		DRAGON_ANIM_FIRE = 11,
		DRAGON_ANIM_FIRE_TO_IDLE = 12,
		DRAGON_ANIM_TURNING_LEFT = 13,
		DRAGON_ANIM_TURNING_RIGHT = 14,
		DRAGON_ANIM_ATTACK_LEFT_1 = 15,
		DRAGON_ANIM_ATTACK_LEFT_2 = 16,
		DRAGON_ANIM_ATTACK_LEFT_3 = 17,
		DRAGON_ANIM_ATTACK_RIGHT_1 = 18,
		DRAGON_ANIM_ATTACK_RIGHT_2 = 19,
		DRAGON_ANIM_ATTACK_RIGHT_3 = 20,
		DRAGON_ANIM_DEATH = 21,
		DRAGON_ANIM_DEFEATED = 22,
		DRAGON_ANIM_RECOVER = 23
	};

	// TODO: Check.
	static void InitializeDragonBones(short frontItemNumber)
	{
		const auto& frontItem = g_Level.Items[frontItemNumber];

		int backItemNumber = frontItem.ItemFlags[0];
		const auto& backItem = g_Level.Items[backItemNumber];

		int frontBoneItemNumber = CreateItem();
		int backBoneItemNumber = CreateItem();
		if (backBoneItemNumber == NO_ITEM || frontBoneItemNumber == NO_ITEM)
			return;

		auto& frontBoneItem = g_Level.Items[frontBoneItemNumber];
		auto& backBoneItem = g_Level.Items[backBoneItemNumber];

		frontBoneItem.ObjectNumber = ID_DRAGON_BONE_FRONT;
		frontBoneItem.Pose = frontItem.Pose;
		frontBoneItem.Pose.Orientation.x = 0;
		frontBoneItem.Pose.Orientation.z = 0;
		frontBoneItem.RoomNumber = frontItem.RoomNumber;
		frontBoneItem.Model.Color = frontItem.Model.Color;
		InitializeItem(frontBoneItemNumber);

		backBoneItem.ObjectNumber = ID_DRAGON_BONE_BACK;
		backBoneItem.Pose = backItem.Pose;
		backBoneItem.Pose.Orientation.x = 0;
		backBoneItem.Pose.Orientation.z = 0;
		backBoneItem.RoomNumber = backItem.RoomNumber;
		backBoneItem.Model.Color = backItem.Model.Color;
		InitializeItem(backBoneItemNumber);

		frontBoneItem.MeshBits = 0xFF3FFFFF;
		backBoneItem.MeshBits = 0xFF3FFFFF;
	}

	static void InitializeDragonBack(ItemInfo& frontItem)
	{
		int backItemNumber = CreateItem();
		if (backItemNumber == NO_ITEM)
		{
			TENLog("Failed to create dragon back.", LogLevel::Warning);
			return;
		}
		auto& backItem = g_Level.Items[backItemNumber];

		SetAnimation(backItem, DRAGON_ANIM_IDLE);

		backItem.ObjectNumber = ID_DRAGON_BACK;
		backItem.Pose = frontItem.Pose;
		backItem.RoomNumber = frontItem.RoomNumber;
		backItem.Model.Color = frontItem.Model.Color;
		backItem.MeshBits.Clear(DragonBackSpineJoints); // TODO: Check what this is. Check if necessary.

		InitializeItem(backItem.Index);

		// Link front item to dragon back half item number.
		frontItem.ItemFlags[0] = backItemNumber;
		backItem.ItemFlags[0] = NO_ITEM;
	}

	void InitializeDragon(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		
		// Initialize front body segment.
		InitializeCreature(item.Index);
		SetAnimation(item, DRAGON_ANIM_IDLE);

		// Initialize back body segment.
		InitializeDragonBack(item);
	}

	static void SyncDragonBackSegment(ItemInfo& frontItem)
	{
		short& backItemNumber = frontItem.ItemFlags[0];
		auto& backItem = g_Level.Items[backItemNumber];

		// Sync destruction.
		backItem.Status = frontItem.Status;
		if (backItem.Status == ITEM_DEACTIVATED)
		{
			KillItem(backItem.Index);
			backItemNumber = NO_ITEM;
			return;
		}

		// Sync animation.
		SetAnimation(backItem, GetAnimNumber(frontItem), GetFrameNumber(frontItem));

		// Sync position.
		backItem.Pose = frontItem.Pose;
		if (backItem.RoomNumber != frontItem.RoomNumber)
			ItemNewRoom(backItem.Index, frontItem.RoomNumber);
	}

	static void SpawnDragonLightEffect(const ItemInfo& item, DragonLightEffectType type)
	{
		auto pos = item.Pose.Position.ToVector3() + Vector3(0.0f, -CLICK(1), 0.0f);

		switch (type)
		{
		default:
		case DragonLightEffectType::Yellow:
		{
			auto color = Color(
				Random::GenerateFloat(0.8f, 0.9f),
				Random::GenerateFloat(0.4f, 0.5f),
				Random::GenerateFloat(0.2f, 0.3f));
			float falloff = Random::GenerateFloat(0.1f, 0.4f);

			TriggerDynamicLight(pos, color, falloff);
		}
			break;

		case DragonLightEffectType::Red:
		{
			auto color = Color(
				Random::GenerateFloat(0.8f, 0.9f),
				Random::GenerateFloat(0.2f, 0.3f),
				Random::GenerateFloat(0.0f, 0.1f));
			float falloff = Random::GenerateFloat(0.1f, 0.2f);

			TriggerDynamicLight(pos, color, falloff);
		}
			break;
		}
	}

	// TODO: Demagic.
	// TODO: Smoke and sparks.
	// TODO: Animate flame sprite sequence.
	static void SpawnDragonFireBreath(const ItemInfo& item, const CreatureBiteInfo& bite, const ItemInfo& targetItem, float vel)
	{
		constexpr auto FIRE_COUNT = 3;
		constexpr auto SPHERE_RADIUS = BLOCK(0.2f);

		for (int i = 0; i < FIRE_COUNT; i++)
		{
			auto& fire = *GetFreeParticle();

			auto origin = GetJointPosition(item, bite.BoneID, bite.Position).ToVector3();
			auto target = GetJointPosition(LaraItem, LM_HIPS).ToVector3();

			auto sphere = BoundingSphere(origin, SPHERE_RADIUS);
			auto pos = Random::GeneratePointInSphere(sphere);

			auto dir = target - origin;
			dir.Normalize();
			dir *= vel;

			fire.spriteIndex = Objects[ID_FIRE_SPRITES].meshIndex;

			fire.on = true;
			fire.sR = Random::GenerateFloat(0.85f, 1.0f) * UCHAR_MAX;
			fire.sG = 64;
			fire.sB = 38;
			fire.dR = Random::GenerateFloat(0.5f, 0.75f) * UCHAR_MAX;
			fire.dG = Random::GenerateFloat(0.31f, 0.56f) * UCHAR_MAX;
			fire.dB = 32;
			fire.colFadeSpeed = 12;
			fire.fadeToBlack = 8;
			fire.blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
			
			fire.x = pos.x;
			fire.y = pos.y;
			fire.z = pos.z;

			int v = Random::GenerateInt(192, 255);
			fire.life =
			fire.sLife = v / 6;

			fire.xVel = v * (dir.x) / 10;
			fire.yVel = v * (dir.y) / 10;
			fire.zVel = v * (dir.z) / 10;

			fire.friction = 85;
			fire.gravity = -Random::GenerateInt(-16, 16);
			fire.maxYvel = 0;
			fire.flags = SP_FIRE | SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;

			fire.scalar = 6;
			fire.dSize = (v * Random::GenerateFloat(60.0f, 67.0f)) / 256;
			fire.sSize = fire.dSize / 4;
			fire.size = fire.dSize;
		}
	}

	void ControlDragon(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;
		
		auto& item = g_Level.Items[itemNumber];
		auto& creature = *GetCreatureInfo(&item);
		auto& timer = item.ItemFlags[1];

		short headingAngle = 0;
		short headYRot = 0;

		bool isTargetAhead = false;

		if (item.HitPoints <= 0)
		{
			if (item.Animation.ActiveState != DRAGON_STATE_DEFEAT)
			{
				SetAnimation(item, DRAGON_ANIM_DEATH);
				timer = 0;
			}
			else
			{
				// Temporary defeat.
				if (timer >= 0)
				{
					SpawnDragonLightEffect(item, DragonLightEffectType::Yellow);
					timer++;

					if (timer == DRAGON_LIVE_TIME)
						item.Animation.TargetState = DRAGON_STATE_IDLE;

					if (timer == DRAGON_LIVE_TIME + DRAGON_ALMOST_LIVE)
						item.HitPoints = Objects[ID_DRAGON_FRONT].HitPoints / 2;
				}
				// Death.
				else
				{
					if (timer > -20)
						SpawnDragonLightEffect(item, DragonLightEffectType::Red);

					if (timer == -100)
					{
						InitializeDragonBones (itemNumber);
					}
					else if (timer == -200)
					{
						DisableEntityAI(itemNumber);
						KillItem(itemNumber);
						item.Status = ITEM_DEACTIVATED;
					}
					else if (timer < -100)
					{
						item.Pose.Position.y += 10;
					}

					timer--;
				}
			}
		}
		else
		{
			AI_INFO ai;
			CreatureAIInfo(&item, &ai);
			
			GetCreatureMood(&item, &ai, true);
			CreatureMood(&item, &ai, true);
			headingAngle = CreatureTurn(&item, DRAGON_WALK_TURN_RATE_MAX);

			isTargetAhead = (ai.ahead && ai.distance > DRAGON_NEAR_RANGE && ai.distance < DRAGON_IDLE_RANGE);

			if (item.TouchBits.TestAny())
				DoDamage(creature.Enemy, DRAGON_CONTACT_DAMAGE);

			switch (item.Animation.ActiveState)
			{
			case DRAGON_STATE_IDLE:
				item.Pose.Orientation.y -= headingAngle;

				if (!isTargetAhead)
				{
					if (ai.distance > DRAGON_IDLE_RANGE || !ai.ahead)
					{
						item.Animation.TargetState = DRAGON_STATE_WALK;
					}
					else if (ai.ahead && ai.distance < DRAGON_NEAR_RANGE && !creature.Flags)
					{
						creature.Flags = 1;

						if (ai.angle < 0)
						{
							item.Animation.TargetState = DRAGON_STATE_SWIPE_ATTACK_LEFT;
						}
						else
						{
							item.Animation.TargetState = DRAGON_STATE_SWIPE_ATTACK_RIGHT;
						}
					}
					else if (ai.angle < 0)
					{
						item.Animation.TargetState = DRAGON_STATE_TURN_LEFT;
					}
					else
					{
						item.Animation.TargetState = DRAGON_STATE_TURN_RIGHT;
					}
				}
				else
				{
					item.Animation.TargetState = DRAGON_STATE_AIM_1;
				}

				break;

			case DRAGON_STATE_SWIPE_ATTACK_LEFT:
				if (item.TouchBits.Test(DragonSwipeAttackJointsLeft))
				{
					DoDamage(creature.Enemy, DRAGON_SWIPE_ATTACK_DAMAGE);
					creature.Flags = 0;
				}

				break;

			case DRAGON_STATE_SWIPE_ATTACK_RIGHT:
				if (item.TouchBits.Test(DragonSwipeAttackJointsRight))
				{
					DoDamage(creature.Enemy, DRAGON_SWIPE_ATTACK_DAMAGE);
					creature.Flags = 0;
				}

				break;

			case DRAGON_STATE_WALK:
				creature.Flags = 0;

				if (isTargetAhead)
				{
					item.Animation.TargetState = DRAGON_STATE_IDLE;
				}
				else if (headingAngle < -DRAGON_TURN_THRESHOLD_ANGLE)
				{
					if (ai.distance < DRAGON_IDLE_RANGE && ai.ahead)
					{
						item.Animation.TargetState = DRAGON_STATE_IDLE;
					}
					else
					{
						item.Animation.TargetState = DRAGON_STATE_MOVE_LEFT;
					}
				}
				else if (headingAngle > DRAGON_TURN_THRESHOLD_ANGLE)
				{
					if (ai.distance < DRAGON_IDLE_RANGE && ai.ahead)
					{
						item.Animation.TargetState = DRAGON_STATE_IDLE;
					}
					else
					{
						item.Animation.TargetState = DRAGON_STATE_MOVE_RIGHT;
					}
				}

				break;

			case DRAGON_STATE_MOVE_LEFT:
				if (headingAngle > -DRAGON_TURN_THRESHOLD_ANGLE || isTargetAhead)
					item.Animation.TargetState = DRAGON_STATE_WALK;

				break;

			case DRAGON_STATE_MOVE_RIGHT:
				if (headingAngle < DRAGON_TURN_THRESHOLD_ANGLE || isTargetAhead)
					item.Animation.TargetState = DRAGON_STATE_WALK;

				break;

			case DRAGON_STATE_TURN_LEFT:
				item.Pose.Orientation.y += -(ANGLE(1.0f) - headingAngle);
				creature.Flags = 0;
				break;

			case DRAGON_STATE_TURN_RIGHT:
				item.Pose.Orientation.y += ANGLE(1.0f) - headingAngle;
				creature.Flags = 0;
				break;

			case DRAGON_STATE_AIM_1:
				item.Pose.Orientation.y -= headingAngle;

				if (ai.ahead)
					headYRot = -ai.angle;

				if (isTargetAhead)
				{
					item.Animation.TargetState = DRAGON_STATE_FIRE_1;
					creature.Flags = 30;
				}
				else
				{
					item.Animation.TargetState = DRAGON_STATE_AIM_1;
					creature.Flags = 0;
				}

				break;

			case DRAGON_STATE_FIRE_1:
				item.Pose.Orientation.y -= headingAngle;
				SoundEffect(SFX_TR2_DRAGON_FIRE, &item.Pose);

				if (ai.ahead)
					headYRot = -ai.angle;

				if (creature.Flags)
				{
					if (ai.ahead)
						SpawnDragonFireBreath(item, DragonMouthBite, *creature.Enemy, 300.0f);

					creature.Flags--;
				}
				else
				{
					item.Animation.TargetState = DRAGON_STATE_IDLE;
				}

				break;
			}
		}

		if (timer >= 0)
		{
			CreatureJoint(&item, 0, headYRot);
			CreatureAnimation(itemNumber, headingAngle, 0);
		}

		SyncDragonBackSegment(item);
	}


	// TODO: Fix, now this function is activating also in the back part, causing a crash due broken pointers to another non-existent backItem.
	void CollideDragon(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll)
	{
		auto& frontItem = g_Level.Items[itemNumber];
		
		if (frontItem.ItemFlags[0] == NO_ITEM)
			return;
		
		short& backItemNumber = frontItem.ItemFlags[0];
		auto& backItem = g_Level.Items[backItemNumber];

		if (!TestBoundsCollide(&frontItem, playerItem, coll->Setup.Radius))
		{
			if (!TestBoundsCollide(&backItem, playerItem, coll->Setup.Radius))
				return;
		}

		if (!TestCollision(&frontItem, playerItem))
		{
			if (!TestCollision(&backItem, playerItem))
				return;
		}

		//TODO: Polish Dagger Interaction
		if (frontItem.Animation.ActiveState == DRAGON_STATE_DEFEAT)
		{
			// TODO: No trig.
			int rx = playerItem->Pose.Position.x - frontItem.Pose.Position.x;
			int rz = playerItem->Pose.Position.z - frontItem.Pose.Position.z;
			float sinY = phd_sin(frontItem.Pose.Orientation.y);
			float cosY = phd_cos(frontItem.Pose.Orientation.y);

			int sideShift = rx * sinY + rz * cosY;
			if (sideShift > DRAGON_LCOL && sideShift < DRAGON_RCOL)
			{
				int shift = rx * cosY - rz * sinY;
				if (shift <= DRAGON_DISTANCE_NEAR && shift >= DRAGON_DISTANCE_FAR)
					return;

				int angle = playerItem->Pose.Orientation.y - frontItem.Pose.Orientation.y;

				int animNumber = frontItem.Animation.AnimNumber - Objects[ID_DRAGON_BACK].animIndex;
				int frameNumber = frontItem.Animation.FrameNumber - GetAnimData(frontItem).frameBase;

				if ((animNumber == DRAGON_ANIM_DEFEATED ||
						(animNumber == (DRAGON_ANIM_DEFEATED + 1) && frameNumber <= DRAGON_ALMOST_LIVE)) &&
					IsHeld(In::Action) &&
					!playerItem->Animation.IsAirborne &&
					frontItem.ObjectNumber == ID_DRAGON_BACK &&
					shift <= DRAGON_MID &&
					shift > (DRAGON_DISTANCE_NEAR - 350) &&
					sideShift > -350 &&
					sideShift < 350 &&
					angle > (ANGLE(45.0f) - ANGLE(30.0f)) &&
					angle < (ANGLE(45.0f) + ANGLE(30.0f)))
				{
					SetAnimation(*playerItem, LA_BUTTON_SMALL_PUSH);
					AnimateItem(LaraItem);

					// TODO: Reimplement dagger pickup animation when state transitions
					// from ID_LARA_EXTRA_ANIMS to ID_LARA are possible. -- Adngel 2023.10.03
					/*SetAnimation(*playerItem, ID_LARA_EXTRA_ANIMS, LEA_PULL_DAGGER_FROM_DRAGON);
					playerItem->Pose = item.Pose;
					playerItem->Animation.IsAirborne = false;
					playerItem->Animation.Velocity.y = 0.0f;
					playerItem->Animation.Velocity.z = 0.0f;

					if (item.RoomNumber != playerItem->RoomNumber)
						ItemNewRoom(playerItem->Index, item.RoomNumber);*/
					
					// TODO: Check. This code was used in old dagger pickup method.
					/*Lara.ExtraAnim = 1;
					Lara.Control.HandStatus = HandStatus::Busy;
					Lara.HitDirection = -1;
					
					playerItem->Model.MeshIndex[LM_RHAND] = Objects[ID_LARA_EXTRA_ANIMS].meshIndex + LM_RHAND;*/

					if (frontItem.ObjectNumber == ID_DRAGON_FRONT)
					{
						backItemNumber = NO_ITEM;
					}
					else if (frontItem.ObjectNumber == ID_DRAGON_BACK)
					{
						auto frontItemNumber = frontItem.NextItem;
						auto& frontPart = g_Level.Items[frontItemNumber];
						backItemNumber = NO_ITEM;
					}
						
					return;
				}

				if (shift < DRAGON_MID)
				{
					shift = DRAGON_DISTANCE_NEAR - shift;
				}
				else
				{
					shift = DRAGON_DISTANCE_FAR - shift;
				}

				// TODO: Check. This code was used in old dagger pickup method.
				/*laraItem->Pose.Position.x += shift * cosY;
				laraItem->Pose.Position.z -= shift * sinY;*/

				return;
			}
		}

		ItemPushItem(&frontItem, playerItem, coll, 1, 0);
	}
}
