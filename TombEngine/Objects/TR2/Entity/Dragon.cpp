#include "framework.h"
#include "Objects/TR2/Entity/Dragon.h"

#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/control/lot.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/Setup.h"
#include "Specific/Input/Input.h"


using namespace TEN::Input;
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
	//const auto DragonBackSpineJoints = std::vector<unsigned int>{ 21, 22, 23 };
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
		InitializeCreature(itemNumber);
		auto& item = g_Level.Items[itemNumber];
		item.Status = ITEM_INVISIBLE;

		SetAnimation(item, DRAGON_ANIM_IDLE);
		
		//Create the back part
		int backItemNumber = CreateItem();
		if (backItemNumber == NO_ITEM)
			return;

		auto& itemBack = g_Level.Items[backItemNumber];

		itemBack.ObjectNumber = ID_DRAGON_BACK;
		InitializeCreature(backItemNumber);

		itemBack.Pose = item.Pose;
		itemBack.RoomNumber = item.RoomNumber;
		itemBack.Model.Color = item.Model.Color;
		itemBack.Status = ITEM_INVISIBLE;
		
		SetAnimation(itemBack, DRAGON_ANIM_IDLE);

		item.ItemFlags[0] = backItemNumber;
		itemBack.ItemFlags[0] = itemNumber;

		//itemBack.MeshBits = 0x1FFFFF;

		g_Level.NumItems ++;
	}

	void ControlDragon(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;
		
		auto& item = g_Level.Items[itemNumber];

		int backItemNumber = item.ItemFlags[0];
		auto& itemBack = g_Level.Items[backItemNumber];

		if (item.ObjectNumber == ID_DRAGON_BACK)
			return;

		auto* creature = GetCreatureInfo(&item);

		short headingAngle = 0;
		short head = 0;

		bool isAhead;

		if (item.HitPoints <= 0)
		{
			if (item.Animation.ActiveState != DRAGON_STATE_DEFEAT)
			{
				SetAnimation(item, DRAGON_ANIM_DEATH);
				item.ItemFlags[1] = 0;
			}
			else
			{
				if (item.ItemFlags[1] >= 0)
				{
					//Defeat routine

					//SpawnBartoliLight(item, 1);
					item.ItemFlags[1]++;

					if (item.ItemFlags[1] == DRAGON_LIVE_TIME)
						item.Animation.TargetState = DRAGON_STATE_IDLE;

					if (item.ItemFlags[1] == DRAGON_LIVE_TIME + DRAGON_ALMOST_LIVE)
						item.HitPoints = Objects[ID_DRAGON_FRONT].HitPoints / 2;
				}
				else
				{
					//Death routine

					//if (item.ItemFlags[1] > -20)
						//SpawnBartoliLight(item, 2);

					if (item.ItemFlags[1] == -100)
					{
						//CreateDragonBone(itemNumber);
					}
					else if (item.ItemFlags[1] == -200)
					{
						DisableEntityAI(itemNumber);
						KillItem(backItemNumber);
						itemBack.Status = ITEM_DEACTIVATED;
						KillItem(itemNumber);
						item.Status = ITEM_DEACTIVATED;
						return;
					}
					else if (item.ItemFlags[1] < -100)
					{
						item.Pose.Position.y += 10;
						itemBack.Pose.Position.y += 10;
					}

					item.ItemFlags[1]--;
					return;
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

			isAhead = (ai.ahead && ai.distance > DRAGON_CLOSE_RANGE && ai.distance < DRAGON_IDLE_RANGE);

			//Contact Damage
			if (item.TouchBits.TestAny())
				DoDamage(creature->Enemy, DRAGON_CONTACT_DAMAGE);

			//States Machine
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
						else if (ai.ahead && ai.distance < DRAGON_CLOSE_RANGE && !creature->Flags)
						{
							creature->Flags = 1;

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
						DoDamage(creature->Enemy, DRAGON_SWIPE_ATTACK_DAMAGE);
						creature->Flags = 0;
					}

					break;

				case DRAGON_STATE_SWIPE_RIGHT:
					if (item.TouchBits.Test(DragonSwipeAttackJointsRight))
					{
						DoDamage(creature->Enemy, DRAGON_SWIPE_ATTACK_DAMAGE);
						creature->Flags = 0;
					}

					break;

				case DRAGON_STATE_WALK:
					creature->Flags = 0;

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
					creature->Flags = 0;

					break;

				case DRAGON_STATE_TURN_RIGHT:
					item.Pose.Orientation.y += (ANGLE(1.0f) - headingAngle);
					creature->Flags = 0;

					break;

				case DRAGON_STATE_AIM_1:
					item.Pose.Orientation.y -= headingAngle;

					if (ai.ahead)
						head = -ai.angle;

					if (isAhead)
					{
						item.Animation.TargetState = DRAGON_STATE_FIRE_1;
						creature->Flags = 30;
					}
					else
					{
						item.Animation.TargetState = DRAGON_STATE_AIM_1;
						creature->Flags = 0;
					}

					break;

				case DRAGON_STATE_FIRE_1:
					item.Pose.Orientation.y -= headingAngle;
					SoundEffect(SFX_TR2_DRAGON_FIRE, &item.Pose);

					if (ai.ahead)
						head = -ai.angle;

					if (creature->Flags)
					{
						if (ai.ahead)
							//TODO spawn fire function
							//SpawnDragonFireBreath(&item, DragonMouthBite, Vector3i(0, 0, 300), creature->Enemy);

						creature->Flags--;
					}
					else
					{
						item.Animation.TargetState = DRAGON_STATE_IDLE;
					}

					break;
			}

		}

		CreatureJoint(&item, 0, head);
		CreatureAnimation(itemNumber, headingAngle, 0);
		
		itemBack.Animation.ActiveState = item.Animation.ActiveState;
		itemBack.Animation.AnimNumber = Objects[ID_DRAGON_BACK].animIndex + (item.Animation.AnimNumber - Objects[ID_DRAGON_FRONT].animIndex);
		itemBack.Animation.FrameNumber = GetAnimData(itemBack).frameBase + (item.Animation.FrameNumber - GetAnimData(item).frameBase);
		itemBack.Pose = item.Pose;

		if (itemBack.RoomNumber != item.RoomNumber)
			ItemNewRoom(backItemNumber, item.RoomNumber);
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
}
