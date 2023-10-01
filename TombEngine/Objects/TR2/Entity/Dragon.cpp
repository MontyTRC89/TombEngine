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
// item.ItemFlags[0]: Used to store the itemNumber of the linked body object.
// item.ItemFlags[1]: Used to store the frame counter of the defeat and death routines

namespace TEN::Entities::Creatures::TR2
{
	constexpr auto DRAGON_SWIPE_ATTACK_DAMAGE = 250;
	constexpr auto DRAGON_CONTACT_DAMAGE = 10;

	constexpr auto DRAGON_LIVE_TIME = 30 * 11;
	constexpr auto DRAGON_ALMOST_LIVE = 100;

	constexpr auto DRAGON_CLOSE_RANGE = SQUARE(BLOCK(3));
	constexpr auto DRAGON_IDLE_RANGE = SQUARE(BLOCK(6));

	constexpr auto DRAGON_WALK_TURN_RATE_MAX = ANGLE(2.0f);
	constexpr auto DRAGON_TURN_THRESHOLD_ANGLE = ANGLE(1.0f);

	constexpr auto DRAGON_CLOSE = 900;
	constexpr auto DRAGON_FAR = 2300;
	constexpr auto DRAGON_MID = ((DRAGON_CLOSE + DRAGON_FAR) / 2);
	constexpr auto DRAGON_LCOL = -CLICK(2);
	constexpr auto DRAGON_RCOL = CLICK(2);

	const auto DragonMouthBite = CreatureBiteInfo(Vector3(35.0f, 171.0f, 1168.0f), 12);
	const auto DragonBackSpineJoints = std::vector<unsigned int>{ 21, 22, 23 };
	const auto DragonSwipeAttackJointsLeft = std::vector<unsigned int>{ 24, 25, 26, 27, 28, 29, 30 };
	const auto DragonSwipeAttackJointsRight = std::vector<unsigned int>{ 1, 2, 3, 4, 5, 6, 7 };

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

	void InitializeDragon(short itemNumber)
	{
		auto frontItemNumber = itemNumber;
		auto& frontItem = g_Level.Items[frontItemNumber];
		
		InitializeCreature(frontItemNumber);
		SetAnimation(frontItem, DRAGON_ANIM_IDLE);

		frontItem.ItemFlags[0] = NO_ITEM;

		InstantiateDragonBack(itemNumber);
	}

	void ControlDragon(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;
		
		auto& dragonItem = g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(&dragonItem);

		short headingAngle = 0;
		short head = 0;

		bool isAhead;

		if (dragonItem.HitPoints <= 0)
		{
			if (dragonItem.Animation.ActiveState != DRAGON_STATE_DEFEAT)
			{
				SetAnimation(dragonItem, DRAGON_ANIM_DEATH);
				dragonItem.ItemFlags[1] = 0;
			}
			else
			{
				if (dragonItem.ItemFlags[1] >= 0)
				{
					//Defeat routine

					DragonLightsManager(dragonItem, 1);
					dragonItem.ItemFlags[1]++;

					if (dragonItem.ItemFlags[1] == DRAGON_LIVE_TIME)
						dragonItem.Animation.TargetState = DRAGON_STATE_IDLE;

					if (dragonItem.ItemFlags[1] == DRAGON_LIVE_TIME + DRAGON_ALMOST_LIVE)
						dragonItem.HitPoints = Objects[ID_DRAGON_FRONT].HitPoints / 2;
				}
				else
				{
					//Death routine

					if (dragonItem.ItemFlags[1] > -20)
						DragonLightsManager(dragonItem, 2);

					if (dragonItem.ItemFlags[1] == -100)
					{
						InstantiateDragonBones (itemNumber);
					}
					else if (dragonItem.ItemFlags[1] == -200)
					{
						DisableEntityAI(itemNumber);
						KillItem(itemNumber);
						dragonItem.Status = ITEM_DEACTIVATED;
					}
					else if (dragonItem.ItemFlags[1] < -100)
					{
						dragonItem.Pose.Position.y += 10;
					}

					dragonItem.ItemFlags[1]--;
				}
			}
		}
		else
		{
			AI_INFO ai;
			CreatureAIInfo(&dragonItem, &ai);
			
			GetCreatureMood(&dragonItem, &ai, true);
			CreatureMood(&dragonItem, &ai, true);
			headingAngle = CreatureTurn(&dragonItem, DRAGON_WALK_TURN_RATE_MAX);

			isAhead = (ai.ahead && ai.distance > DRAGON_CLOSE_RANGE && ai.distance < DRAGON_IDLE_RANGE);

			//Contact Damage
			if (dragonItem.TouchBits.TestAny())
				DoDamage(creature->Enemy, DRAGON_CONTACT_DAMAGE);

			//States Machine
			switch (dragonItem.Animation.ActiveState)
			{
				case DRAGON_STATE_IDLE:
					dragonItem.Pose.Orientation.y -= headingAngle;

					if (!isAhead)
					{
						if (ai.distance > DRAGON_IDLE_RANGE || !ai.ahead)
						{
							dragonItem.Animation.TargetState = DRAGON_STATE_WALK;
						}
						else if (ai.ahead && ai.distance < DRAGON_CLOSE_RANGE && !creature->Flags)
						{
							creature->Flags = 1;

							if (ai.angle < 0)
							{
								dragonItem.Animation.TargetState = DRAGON_STATE_SWIPE_LEFT;
							}
							else
							{
								dragonItem.Animation.TargetState = DRAGON_STATE_SWIPE_RIGHT;
							}
						}
						else if (ai.angle < 0)
						{
							dragonItem.Animation.TargetState = DRAGON_STATE_TURN_LEFT;
						}
						else
						{
							dragonItem.Animation.TargetState = DRAGON_STATE_TURN_RIGHT;
						}
					}
					else
					{
						dragonItem.Animation.TargetState = DRAGON_STATE_AIM_1;
					}

					break;

				case DRAGON_STATE_SWIPE_LEFT:
					if (dragonItem.TouchBits.Test(DragonSwipeAttackJointsLeft))
					{
						DoDamage(creature->Enemy, DRAGON_SWIPE_ATTACK_DAMAGE);
						creature->Flags = 0;
					}

					break;

				case DRAGON_STATE_SWIPE_RIGHT:
					if (dragonItem.TouchBits.Test(DragonSwipeAttackJointsRight))
					{
						DoDamage(creature->Enemy, DRAGON_SWIPE_ATTACK_DAMAGE);
						creature->Flags = 0;
					}

					break;

				case DRAGON_STATE_WALK:
					creature->Flags = 0;

					if (isAhead)
					{
						dragonItem.Animation.TargetState = DRAGON_STATE_IDLE;
					}
					else if (headingAngle < -DRAGON_TURN_THRESHOLD_ANGLE)
					{
						if (ai.distance < DRAGON_IDLE_RANGE && ai.ahead)
						{
							dragonItem.Animation.TargetState = DRAGON_STATE_IDLE;
						}
						else
						{
							dragonItem.Animation.TargetState = DRAGON_STATE_MOVE_LEFT;
						}
					}
					else if (headingAngle > DRAGON_TURN_THRESHOLD_ANGLE)
					{
						if (ai.distance < DRAGON_IDLE_RANGE && ai.ahead)
						{
							dragonItem.Animation.TargetState = DRAGON_STATE_IDLE;
						}
						else
						{
							dragonItem.Animation.TargetState = DRAGON_STATE_MOVE_RIGHT;
						}
					}

					break;

				case DRAGON_STATE_MOVE_LEFT:
					if (headingAngle > -DRAGON_TURN_THRESHOLD_ANGLE || isAhead)
						dragonItem.Animation.TargetState = DRAGON_STATE_WALK;

					break;

				case DRAGON_STATE_MOVE_RIGHT:
					if (headingAngle < DRAGON_TURN_THRESHOLD_ANGLE || isAhead)
						dragonItem.Animation.TargetState = DRAGON_STATE_WALK;

					break;

				case DRAGON_STATE_TURN_LEFT:
					dragonItem.Pose.Orientation.y += -(ANGLE(1.0f) - headingAngle);
					creature->Flags = 0;

					break;

				case DRAGON_STATE_TURN_RIGHT:
					dragonItem.Pose.Orientation.y += (ANGLE(1.0f) - headingAngle);
					creature->Flags = 0;

					break;

				case DRAGON_STATE_AIM_1:
					dragonItem.Pose.Orientation.y -= headingAngle;

					if (ai.ahead)
						head = -ai.angle;

					if (isAhead)
					{
						dragonItem.Animation.TargetState = DRAGON_STATE_FIRE_1;
						creature->Flags = 30;
					}
					else
					{
						dragonItem.Animation.TargetState = DRAGON_STATE_AIM_1;
						creature->Flags = 0;
					}

					break;

				case DRAGON_STATE_FIRE_1:
					dragonItem.Pose.Orientation.y -= headingAngle;
					SoundEffect(SFX_TR2_DRAGON_FIRE, &dragonItem.Pose);

					if (ai.ahead)
						head = -ai.angle;

					if (creature->Flags)
					{
						if (ai.ahead)
							DragonFireBreath(&dragonItem, DragonMouthBite, 300, creature->Enemy);

						creature->Flags--;
					}
					else
					{
						dragonItem.Animation.TargetState = DRAGON_STATE_IDLE;
					}

					break;
			}

		}

		if (dragonItem.ItemFlags[1] >= 0)
		{
			CreatureJoint(&dragonItem, 0, head);
			CreatureAnimation(itemNumber, headingAngle, 0);
		}

		if (dragonItem.ItemFlags[0] != NO_ITEM)
		{
			UpdateDragonBack(itemNumber, dragonItem.ItemFlags[0]);
		}
		else
		{
			InstantiateDragonBack(itemNumber);
		}
	}

	void CollideDragon(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto& item = g_Level.Items[itemNumber];

		int backItemNumber = item.ItemFlags[0];
		auto& itemBack = g_Level.Items[backItemNumber];

		if (!TestBoundsCollide(&item, laraItem, coll->Setup.Radius))
			if (!TestBoundsCollide(&itemBack, laraItem, coll->Setup.Radius))
				return;

		if (!TestCollision(&item, laraItem))
			if (!TestCollision(&itemBack, laraItem))
				return;

		if (item.Animation.ActiveState == DRAGON_STATE_DEFEAT)
		{
			int rx = laraItem->Pose.Position.x - item.Pose.Position.x;
			int rz = laraItem->Pose.Position.z - item.Pose.Position.z;
			float sinY = phd_sin(item.Pose.Orientation.y);
			float cosY = phd_cos(item.Pose.Orientation.y);

			int sideShift = rx * sinY + rz * cosY;
			if (sideShift > DRAGON_LCOL && sideShift < DRAGON_RCOL)
			{
				int shift = rx * cosY - rz * sinY;
				if (shift <= DRAGON_CLOSE && shift >= DRAGON_FAR)
					return;

				int angle = laraItem->Pose.Orientation.y - item.Pose.Orientation.y;

				int anim = item.Animation.AnimNumber - Objects[ID_DRAGON_BACK].animIndex;
				int frame = item.Animation.FrameNumber - GetAnimData(item).frameBase;

				if ((anim == DRAGON_ANIM_DEFEATED || (anim == DRAGON_ANIM_DEFEATED + 1 && frame <= DRAGON_ALMOST_LIVE)) &&
					IsHeld(In::Action) &&
					item.ObjectNumber == ID_DRAGON_BACK &&
					!laraItem->Animation.IsAirborne &&
					shift <= DRAGON_MID &&
					shift > (DRAGON_CLOSE - 350) &&
					sideShift > -350 &&
					sideShift < 350 &&
					angle >(ANGLE(45.0f) - ANGLE(30.0f)) &&
					angle < (ANGLE(45.0f) + ANGLE(30.0f)))
				{
					/*
					// TODO: Reimplement the Dagger Pickup animation when the transition from ID_LARA_EXTRA_ANIMS to ID_LARA get solved
					SetAnimation(*laraItem, ID_LARA_EXTRA_ANIMS, LEA_PULL_DAGGER_FROM_DRAGON);
					laraItem->Pose = item.Pose;
					laraItem->Animation.IsAirborne = false;
					laraItem->Animation.Velocity.y = 0.0f;
					laraItem->Animation.Velocity.z = 0.0f;
					

					if (item.RoomNumber != laraItem->RoomNumber)
						ItemNewRoom(LaraItem->Index, item.RoomNumber);
					*/

					SetAnimation(*laraItem, LA_BUTTON_SMALL_PUSH);
					AnimateItem(LaraItem);

					/*
					// TODO: Review, This code was used with the old Dagger Pickup method
					Lara.ExtraAnim = 1;
					Lara.Control.HandStatus = HandStatus::Busy;
					Lara.HitDirection = -1;
					*/

					//laraItem->Model.MeshIndex[LM_RHAND] = Objects[ID_LARA_EXTRA_ANIMS].meshIndex + LM_RHAND;

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
					shift = DRAGON_CLOSE - shift;
				}
				else
				{
					shift = DRAGON_FAR - shift;
				}

				/*
				// TODO: Review, This code was used with the old Dagger Pickup method
				laraItem->Pose.Position.x += shift * cosY;
				laraItem->Pose.Position.z -= shift * sinY;
				*/

				return;
			}
		}

		ItemPushItem(&item, laraItem, coll, 1, 0);
	}

	void DragonFireBreath(ItemInfo* item, const CreatureBiteInfo& bite, int speed, ItemInfo* enemy)
	{
		constexpr auto COUNT = 3;

		for (int i = 0; i < COUNT; i++)
		{
			auto& fire = *GetFreeParticle();

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

			int rotation = (GetRandomControl() & 0x3FF) % 360; //A random value between 0 and 359
			fire.rotAng = ANGLE(rotation);

			// TODO: solve flame sprite animation.
			//float alpha = fmin(1, fmax(0, 1 - (fire.life / (float)fire.sLife)));
			float alpha = 20.0f;
			int sprite = (int)Lerp(Objects[ID_FIRE_SPRITES].meshIndex, Objects[ID_FIRE_SPRITES].meshIndex + (-Objects[ID_FIRE_SPRITES].nmeshes) - 1, alpha);
			fire.spriteIndex = sprite;

			auto posStart = GetJointPosition(item, bite.BoneID, Vector3i(-4, -30, -4) + bite.Position);
			auto posGoal = LaraItem->Pose.Position + Vector3i (0, -CLICK(1), 0);

			auto direction = (posGoal - posStart).ToVector3();
			direction.Normalize();
			direction *= speed;

			fire.x = (GetRandomControl() & 0x1F) + posStart.x - 16;
			fire.y = (GetRandomControl() & 0x1F) + posStart.y - 16;
			fire.z = (GetRandomControl() & 0x1F) + posStart.z - 16;

			int v = (GetRandomControl() & 0x3F) + 192;
			fire.life = fire.sLife = v / 6;

			fire.xVel = v * (direction.x) / 10;
			fire.yVel = v * (direction.y) / 10;
			fire.zVel = v * (direction.z) / 10;

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

	void DragonLightsManager(const ItemInfo& item, int type)
	{
		switch (type)
		{
			case 0:
				TriggerDynamicLight(
					item.Pose.Position.x, item.Pose.Position.y - CLICK(1), item.Pose.Position.z,
					(GetRandomControl() & 150) + 25, //Strong
					(GetRandomControl() & 30) + 200, (GetRandomControl() & 25) + 200, (GetRandomControl() & 20) + 200); //White
				break;

			case 1:
				TriggerDynamicLight(
					item.Pose.Position.x, item.Pose.Position.y - CLICK(1), item.Pose.Position.z,
					(GetRandomControl() & 75) + 25, //Middle
					(GetRandomControl() & 30) + 200, (GetRandomControl() & 25) + 100, (GetRandomControl() & 20) + 50); //Yellowish
				break;

			case 2:
				TriggerDynamicLight(
					item.Pose.Position.x, item.Pose.Position.y - CLICK(1), item.Pose.Position.z,
					(GetRandomControl() & 20) + 25, //Weak
					(GetRandomControl() & 30) + 200, (GetRandomControl() & 25) + 50, (GetRandomControl() & 20) + 0); //Reddish
				break;

			default:
				break;
		}
	}

	void UpdateDragonBack(short frontItemNumber, short backItemNumber)
	{
		auto& backItem = g_Level.Items[backItemNumber];
		auto& frontItem = g_Level.Items[frontItemNumber];

		backItem.Status = frontItem.Status;
		if (backItem.Status == ITEM_DEACTIVATED)
		{
			KillItem(backItemNumber);
			frontItem.ItemFlags[0] = NO_ITEM;
			return;
		}

		backItem.Pose = frontItem.Pose;
		if (backItem.RoomNumber != frontItem.RoomNumber)
			ItemNewRoom(backItemNumber, frontItem.RoomNumber);

		backItem.Animation.ActiveState = frontItem.Animation.ActiveState;
		backItem.Animation.AnimNumber = Objects[ID_DRAGON_BACK].animIndex + (frontItem.Animation.AnimNumber - Objects[ID_DRAGON_FRONT].animIndex);
		backItem.Animation.FrameNumber = GetAnimData(backItem).frameBase + (frontItem.Animation.FrameNumber - GetAnimData(frontItem).frameBase);

		if (frontItem.ItemFlags[1] >= 0)
		{
			CreatureAnimation(backItemNumber, 0, 0);
		}
	}

	void InstantiateDragonBack(short itemNumber)
	{
		auto frontItemNumber = itemNumber;
		auto& frontItem = g_Level.Items[frontItemNumber];

		int backItemNumber = CreateItem();
		if (backItemNumber == NO_ITEM)
		{
			TENLog("Failed to create ID_DRAGON_BACK from ID_DRAGON_FRONT.", LogLevel::Warning);
			return;
		}
		auto& backItem = g_Level.Items[backItemNumber];

		backItem.ObjectNumber = ID_DRAGON_BACK;

		backItem.Pose = frontItem.Pose;
		backItem.RoomNumber = frontItem.RoomNumber;
		backItem.Model.Color = frontItem.Model.Color;
		backItem.MeshBits.SetAll();
		backItem.Name = "instantiated_Dragon_Back_" + std::to_string (backItemNumber);		
		
		g_Level.NumItems++;

		frontItem.ItemFlags[0] = backItemNumber;
	}

	void InstantiateDragonBones(short itemNumber)
	{
		short frontItemNumber = itemNumber;
		const auto& frontItem = g_Level.Items[frontItemNumber];
		
		short backItemNumber = frontItem.ItemFlags[0];
		const auto& backItem = g_Level.Items[backItemNumber];

		int boneFrontItemNumber = CreateItem();
		int boneBackItemNumber = CreateItem();
		if (boneBackItemNumber == NO_ITEM || boneFrontItemNumber == NO_ITEM)
			return;

		auto& boneFrontItem = g_Level.Items[boneFrontItemNumber];
		auto& boneBackItem = g_Level.Items[boneBackItemNumber];
						
		boneFrontItem.ObjectNumber = ID_DRAGON_BONE_FRONT;
		boneFrontItem.Pose = frontItem.Pose;
		boneFrontItem.Pose.Orientation.x = 0;
		boneFrontItem.Pose.Orientation.z = 0;
		boneFrontItem.RoomNumber = frontItem.RoomNumber;
		boneFrontItem.Model.Color = frontItem.Model.Color;
		InitializeItem(boneFrontItemNumber);

		boneBackItem.ObjectNumber = ID_DRAGON_BONE_BACK;
		boneBackItem.Pose = backItem.Pose;
		boneBackItem.Pose.Orientation.x = 0;
		boneBackItem.Pose.Orientation.z = 0;
		boneBackItem.RoomNumber = backItem.RoomNumber;
		boneBackItem.Model.Color = backItem.Model.Color;
		InitializeItem(boneBackItemNumber);
		
		boneFrontItem.MeshBits = 0xFF3FFFFF;
		boneBackItem.MeshBits = 0xFF3FFFFF;
	}
}
