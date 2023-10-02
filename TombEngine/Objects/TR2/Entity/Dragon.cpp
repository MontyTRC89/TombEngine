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
// frameCounter: Frame counter for temporary defeat and death.

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
		DRAGON_STATE_SWIPE_LEFT = 9,
		DRAGON_STATE_SWIPE_RIGHT = 10,
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

		short backItemNumber = frontItem.ItemFlags[0];
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

	static void InitializeDragonBack(ItemInfo& item)
	{
		int backItemNumber = CreateItem();
		if (backItemNumber == NO_ITEM)
		{
			TENLog("Failed to create dragon back.", LogLevel::Warning);
			return;
		}
		auto& backItem = g_Level.Items[backItemNumber];

		//InitializeCreature(backItem.Index);
		SetAnimation(backItem, DRAGON_ANIM_IDLE);

		// TODO: Check if necessary.
		backItem.ObjectNumber = ID_DRAGON_BACK;
		backItem.Pose = item.Pose;
		backItem.RoomNumber = item.RoomNumber;
		backItem.Model.Color = item.Model.Color;
		backItem.MeshBits.Clear(DragonBackSpineJoints); // TODO: Check what this is.
		InitializeItem(backItem.Index);

		// Store ID of back segment item number.
		item.ItemFlags[0] = backItemNumber;
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

	static void UpdateDragonBack(ItemInfo& item)
	{
		auto& backItem = g_Level.Items[item.ItemFlags[0]];

		backItem.Status = item.Status;
		if (backItem.Status == ITEM_DEACTIVATED)
		{
			KillItem(backItem.Index);
			item.ItemFlags[0] = NO_ITEM;
			return;
		}

		// TODO: Better way.
		backItem.Animation.ActiveState = item.Animation.ActiveState;
		backItem.Animation.AnimNumber = Objects[ID_DRAGON_BACK].animIndex + (item.Animation.AnimNumber - Objects[ID_DRAGON_FRONT].animIndex);
		backItem.Animation.FrameNumber = GetAnimData(backItem).frameBase + (item.Animation.FrameNumber - GetAnimData(item).frameBase);

		backItem.Pose = item.Pose;
		if (backItem.RoomNumber != item.RoomNumber)
			ItemNewRoom(backItem.Index, item.RoomNumber);
	}

	static void SpawnDragonLightEffect(const ItemInfo& item, DragonLightEffectType type)
	{
		auto pos = item.Pose.Position.ToVector3();
		pos.y -= CLICK(1);

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

			SpawnDynamicLight(pos, color, falloff);
		}
			break;

		case DragonLightEffectType::Red:
		{
			auto color = Color(
				Random::GenerateFloat(0.8f, 0.9f),
				Random::GenerateFloat(0.2f, 0.3f),
				Random::GenerateFloat(0.0f, 0.1f));
			float falloff = Random::GenerateFloat(0.1f, 0.2f);

			SpawnDynamicLight(pos, color, falloff);
		}
			break;
		}
	}

	// TODO: Smoke and sparks.
	static void SpawnDragonFireBreath(const ItemInfo& item, const CreatureBiteInfo& bite, const ItemInfo& targetItem, float vel)
	{
		constexpr auto FIRE_COUNT = 3;

		for (int i = 0; i < FIRE_COUNT; i++)
		{
			auto& fire = *GetFreeParticle();

			auto origin = GetJointPosition(item, bite.BoneID, bite.Position);
			auto target = GetJointPosition(LaraItem, LM_HIPS);

			auto dir = (target - origin).ToVector3();
			dir.Normalize();
			dir *= vel;

			// TODO: Animate sprite. Can't be done here.
			fire.spriteIndex = Objects[ID_FIRE_SPRITES].meshIndex;

			fire.on = true;
			fire.sR = 255 - (GetRandomControl() & 0x1F);
			fire.sG = 64;
			fire.sB = 38;
			fire.dR = 128 + (GetRandomControl() & 0x3F);
			fire.dG = 80 + (GetRandomControl() & 0x3F);
			fire.dB = 32;
			fire.colFadeSpeed = 12;
			fire.fadeToBlack = 8;
			fire.blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;

			fire.x = (GetRandomControl() & 0x1F) + origin.x - 16;
			fire.y = (GetRandomControl() & 0x1F) + origin.y - 16;
			fire.z = (GetRandomControl() & 0x1F) + origin.z - 16;

			int v = (GetRandomControl() & 0x3F) + 192;
			fire.life =
			fire.sLife = v / 6;

			fire.xVel = v * (dir.x) / 10;
			fire.yVel = v * (dir.y) / 10;
			fire.zVel = v * (dir.z) / 10;

			fire.friction = 85;
			fire.gravity = -16 - (GetRandomControl() & 0x1F);
			fire.maxYvel = 0;
			fire.flags = SP_FIRE | SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;

			fire.scalar = 6;
			fire.dSize = (v * ((GetRandomControl() & 7) + 60)) / 256;
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
		auto& frameCounter = item.ItemFlags[1];

		short headingAngle = 0;
		short headYRot = 0;

		bool isAhead = false;

		if (item.HitPoints <= 0)
		{
			if (item.Animation.ActiveState != DRAGON_STATE_DEFEAT)
			{
				SetAnimation(item, DRAGON_ANIM_DEATH);
				frameCounter = 0;
			}
			else
			{
				// Temporary defeat.
				if (frameCounter >= 0)
				{
					SpawnDragonLightEffect(item, DragonLightEffectType::Yellow);
					frameCounter++;

					if (frameCounter == DRAGON_LIVE_TIME)
						item.Animation.TargetState = DRAGON_STATE_IDLE;

					if (frameCounter == DRAGON_LIVE_TIME + DRAGON_ALMOST_LIVE)
						item.HitPoints = Objects[ID_DRAGON_FRONT].HitPoints / 2;
				}
				// Death.
				else
				{
					if (frameCounter > -20)
						SpawnDragonLightEffect(item, DragonLightEffectType::Red);

					if (frameCounter == -100)
					{
						InitializeDragonBones (itemNumber);
					}
					else if (frameCounter == -200)
					{
						DisableEntityAI(itemNumber);
						KillItem(itemNumber);
						item.Status = ITEM_DEACTIVATED;
					}
					else if (frameCounter < -100)
					{
						item.Pose.Position.y += 10;
					}

					frameCounter--;
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

			isAhead = (ai.ahead && ai.distance > DRAGON_NEAR_RANGE && ai.distance < DRAGON_IDLE_RANGE);

			// Contact damage.
			if (item.TouchBits.TestAny())
				DoDamage(creature.Enemy, DRAGON_CONTACT_DAMAGE);

			switch (item.Animation.ActiveState)
			{
			case DRAGON_STATE_IDLE:
				item.Pose.Orientation.y -= headingAngle;

				if (!isAhead)
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
							item.Animation.TargetState = DRAGON_STATE_SWIPE_LEFT;
						}
						else
						{
							item.Animation.TargetState = DRAGON_STATE_SWIPE_RIGHT;
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

			case DRAGON_STATE_SWIPE_LEFT:
				if (item.TouchBits.Test(DragonSwipeAttackJointsLeft))
				{
					DoDamage(creature.Enemy, DRAGON_SWIPE_ATTACK_DAMAGE);
					creature.Flags = 0;
				}

				break;

			case DRAGON_STATE_SWIPE_RIGHT:
				if (item.TouchBits.Test(DragonSwipeAttackJointsRight))
				{
					DoDamage(creature.Enemy, DRAGON_SWIPE_ATTACK_DAMAGE);
					creature.Flags = 0;
				}

				break;

			case DRAGON_STATE_WALK:
				creature.Flags = 0;

				if (isAhead)
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
				if (headingAngle > -DRAGON_TURN_THRESHOLD_ANGLE || isAhead)
					item.Animation.TargetState = DRAGON_STATE_WALK;

				break;

			case DRAGON_STATE_MOVE_RIGHT:
				if (headingAngle < DRAGON_TURN_THRESHOLD_ANGLE || isAhead)
					item.Animation.TargetState = DRAGON_STATE_WALK;

				break;

			case DRAGON_STATE_TURN_LEFT:
				item.Pose.Orientation.y += -(ANGLE(1.0f) - headingAngle);
				creature.Flags = 0;
				break;

			case DRAGON_STATE_TURN_RIGHT:
				item.Pose.Orientation.y += (ANGLE(1.0f) - headingAngle);
				creature.Flags = 0;
				break;

			case DRAGON_STATE_AIM_1:
				item.Pose.Orientation.y -= headingAngle;

				if (ai.ahead)
					headYRot = -ai.angle;

				if (isAhead)
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

		if (frameCounter >= 0)
		{
			CreatureJoint(&item, 0, headYRot);
			CreatureAnimation(itemNumber, headingAngle, 0);
			UpdateDragonBack(item);
		}
	}

	void CollideDragon(short itemNumber, ItemInfo* playerItem, CollisionInfo* coll)
	{
		auto& item = g_Level.Items[itemNumber];

		int backItemNumber = item.ItemFlags[0];
		auto& backItem = g_Level.Items[backItemNumber];

		if (!TestBoundsCollide(&item, playerItem, coll->Setup.Radius))
		{
			if (!TestBoundsCollide(&backItem, playerItem, coll->Setup.Radius))
				return;
		}

		if (!TestCollision(&item, playerItem))
		{
			if (!TestCollision(&backItem, playerItem))
				return;
		}

		if (item.Animation.ActiveState == DRAGON_STATE_DEFEAT)
		{
			// TODO: No trig.
			int rx = playerItem->Pose.Position.x - item.Pose.Position.x;
			int rz = playerItem->Pose.Position.z - item.Pose.Position.z;
			float sinY = phd_sin(item.Pose.Orientation.y);
			float cosY = phd_cos(item.Pose.Orientation.y);

			int sideShift = rx * sinY + rz * cosY;
			if (sideShift > DRAGON_LCOL && sideShift < DRAGON_RCOL)
			{
				int shift = rx * cosY - rz * sinY;
				if (shift <= DRAGON_DISTANCE_NEAR && shift >= DRAGON_DISTANCE_FAR)
					return;

				int angle = playerItem->Pose.Orientation.y - item.Pose.Orientation.y;

				int animNumber = item.Animation.AnimNumber - Objects[ID_DRAGON_BACK].animIndex;
				int frameNumber = item.Animation.FrameNumber - GetAnimData(item).frameBase;

				if ((animNumber == DRAGON_ANIM_DEFEATED ||
						(animNumber == (DRAGON_ANIM_DEFEATED + 1) && frameNumber <= DRAGON_ALMOST_LIVE)) &&
					IsHeld(In::Action) &&
					!playerItem->Animation.IsAirborne &&
					item.ObjectNumber == ID_DRAGON_BACK &&
					shift <= DRAGON_MID &&
					shift > (DRAGON_DISTANCE_NEAR - 350) &&
					sideShift > -350 &&
					sideShift < 350 &&
					angle > (ANGLE(45.0f) - ANGLE(30.0f)) &&
					angle < (ANGLE(45.0f) + ANGLE(30.0f)))
				{
					/*
					// TODO: Reimplement the Dagger Pickup animation when the transition from ID_LARA_EXTRA_ANIMS to ID_LARA get solved
					SetAnimation(*playerItem, ID_LARA_EXTRA_ANIMS, LEA_PULL_DAGGER_FROM_DRAGON);
					playerItem->Pose = item.Pose;
					playerItem->Animation.IsAirborne = false;
					playerItem->Animation.Velocity.y = 0.0f;
					playerItem->Animation.Velocity.z = 0.0f;

					if (item.RoomNumber != playerItem->RoomNumber)
						ItemNewRoom(playerItem->Index, item.RoomNumber);
					*/

					SetAnimation(*playerItem, LA_BUTTON_SMALL_PUSH);
					AnimateItem(LaraItem);

					/*
					// TODO: Review, This code was used with the old Dagger Pickup method
					Lara.ExtraAnim = 1;
					Lara.Control.HandStatus = HandStatus::Busy;
					Lara.HitDirection = -1;
					*/

					//playerItem->Model.MeshIndex[LM_RHAND] = Objects[ID_LARA_EXTRA_ANIMS].meshIndex + LM_RHAND;

					if (item.ObjectNumber == ID_DRAGON_FRONT)
					{
						item.ItemFlags[1] = -1;
					}
					else if (item.ObjectNumber == ID_DRAGON_BACK)
					{
						auto frontItemNumber = item.NextItem;
						auto& frontPart = g_Level.Items[frontItemNumber];
						frontPart.ItemFlags[1] = -1;
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

				/*
				// TODO: Review, This code was used with the old Dagger Pickup method
				laraItem->Pose.Position.x += shift * cosY;
				laraItem->Pose.Position.z -= shift * sinY;
				*/

				return;
			}
		}

		ItemPushItem(&item, playerItem, coll, 1, 0);
	}
}
