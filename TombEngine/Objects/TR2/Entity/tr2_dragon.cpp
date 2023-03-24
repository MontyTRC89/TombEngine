#include "framework.h"
#include "Objects/TR2/Entity/tr2_dragon.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/collide_item.h"
#include "Game/collision/sphere.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Specific/Input/Input.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include <tomb4fx.h>

using namespace TEN::Input;

namespace TEN::Entities::Creatures::TR2
{
	constexpr auto DRAGON_SWIPE_ATTACK_DAMAGE = 250;
	constexpr auto DRAGON_CONTACT_DAMAGE	  = 10;

	const auto DragonMouthBite = BiteInfo(Vector3(35.0f, 171.0f, 1168.0f), 12);
	const auto DragonSwipeAttackJointsLeft  = std::vector<unsigned int>{ 24, 25, 26, 27, 28, 29, 30 };
	const auto DragonSwipeAttackJointsRight = std::vector<unsigned int>{ 1, 2, 3, 4, 5, 6, 7 };
	const auto DragonBackSpineJoints		= std::vector<unsigned int>{ 21, 22, 23 };

	// TODO: Organise.
	#define DRAGON_LIVE_TIME (30 * 11)
	#define DRAGON_CLOSE_RANGE pow(BLOCK(3), 2)
	#define DRAGON_STATE_IDLE_RANGE pow(BLOCK(6), 2)
	#define DRAGON_FLAME_SPEED 200

	#define DRAGON_ALMOST_LIVE 100
	#define BOOM_TIME 130
	#define BOOM_TIME_MIDDLE 140
	#define BOOM_TIME_END 150

	#define BARTOLI_RANGE BLOCK(9)
	#define DRAGON_CLOSE 900
	#define DRAGON_FAR 2300
	#define DRAGON_MID ((DRAGON_CLOSE + DRAGON_FAR) / 2)
	#define DRAGON_LCOL -CLICK(2)
	#define DRAGON_RCOL CLICK(2)

	#define DRAGON_STATE_WALK_TURN ANGLE(2.0f)
	#define DRAGON_NEED_TURN ANGLE(1.0f)
	#define DRAGON_TURN_TURN ANGLE(1.0f)

	enum DragonState
	{
		// No state 0.
		DRAGON_STATE_WALK = 1,
		DRAGON_STATE_LEFT = 2,
		DRAGON_STATE_RIGHT = 3,
		DRAGON_STATE_AIM_1 = 4,
		DRAGON_STATE_FIRE_1 = 5,
		DRAGON_STATE_IDLE = 6,
		DRAGON_STATE_TURN_LEFT = 7,
		DRAGON_STATE_TURN_RIGHT = 8,
		DRAGON_STATE_SWIPE_LEFT = 9,
		DRAGON_STATE_SWIPE_RIGHT = 10,
		DRAGON_STATE_DEATH = 11
	};

	// TODO
	enum DragonAnim
	{
		DRAGON_ANIM_DEATH = 21,
		DRAGON_ANIM_DEAD = 22
	};

	static void CreateBartoliLight(short ItemIndex, int type)
	{
		auto* item = &g_Level.Items[ItemIndex];

		if (type == 0)
			TriggerDynamicLight(item->Pose.Position.x, item->Pose.Position.y - CLICK(1), item->Pose.Position.z, (GetRandomControl() & 150) + 25, (GetRandomControl() & 30) + 200, (GetRandomControl() & 25) + 200, (GetRandomControl() & 20) + 200);
		else if (type == 1)
			TriggerDynamicLight(item->Pose.Position.x, item->Pose.Position.y - CLICK(1), item->Pose.Position.z, (GetRandomControl() & 75) + 25, (GetRandomControl() & 30) + 200, (GetRandomControl() & 25) + 100, (GetRandomControl() & 20) + 50);
		else if (type == 2)
			TriggerDynamicLight(item->Pose.Position.x, item->Pose.Position.y - CLICK(1), item->Pose.Position.z, (GetRandomControl() & 20) + 25, (GetRandomControl() & 30) + 200, (GetRandomControl() & 25) + 50, (GetRandomControl() & 20) + 0);
	}

	static void createExplosion(ItemInfo* item)
	{
		short explosionIndex = CreateItem();

		if (explosionIndex != NO_ITEM)
		{
			auto* explosionItem = &g_Level.Items[explosionIndex];

			switch (item->Timer)
			{
			case BOOM_TIME:
				explosionItem->ObjectNumber = ID_SPHERE_OF_DOOM;
				break;

			case BOOM_TIME + 10:
				explosionItem->ObjectNumber = ID_SPHERE_OF_DOOM2;
				break;

			case BOOM_TIME + 20:
				explosionItem->ObjectNumber = ID_SPHERE_OF_DOOM3;
				break;
			}

			explosionItem->Pose.Position.x = item->Pose.Position.x;
			explosionItem->Pose.Position.y = item->Pose.Position.y + CLICK(1);
			explosionItem->Pose.Position.z = item->Pose.Position.z;
			explosionItem->RoomNumber = item->RoomNumber;
			explosionItem->Pose.Orientation = EulerAngles::Zero;
			explosionItem->Animation.Velocity.y = 0.0f;
			explosionItem->Animation.Velocity.z = 0.0f;

			InitialiseItem(explosionIndex);
			AddActiveItem(explosionIndex);

			explosionItem->Status = ITEM_ACTIVE;
		}
	}

	static void createDragonBone(short frontNumber)
	{
		short boneFront = CreateItem();
		short boneBack = CreateItem();

		if (boneBack != NO_ITEM && boneFront != NO_ITEM)
		{
			auto* item = &g_Level.Items[frontNumber];
			auto* dragonBack = &g_Level.Items[boneBack];

			dragonBack->ObjectNumber = ID_DRAGON_BONE_BACK;
			dragonBack->Pose = item->Pose;
			dragonBack->Pose.Orientation.x = 0;
			dragonBack->Pose.Orientation.z = 0;
			dragonBack->RoomNumber = item->RoomNumber;

			InitialiseItem(boneBack);

			auto* dragonFront = &g_Level.Items[boneFront];

			dragonFront->ObjectNumber = ID_DRAGON_BONE_FRONT;
			dragonFront->Pose = item->Pose;
			dragonFront->Pose.Orientation.x = 0;
			dragonFront->Pose.Orientation.z = 0;
			dragonFront->RoomNumber = item->RoomNumber;

			InitialiseItem(boneFront);

			dragonFront->MeshBits = 0xFF3FFFFF;
		}
	}

	void DragonCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* item = &g_Level.Items[itemNumber];

		if (!TestBoundsCollide(item, laraItem, coll->Setup.Radius))
			return;

		if (!TestCollision(item, laraItem))
			return;

		if (item->Animation.ActiveState == DRAGON_STATE_DEATH)
		{
			int rx = laraItem->Pose.Position.x - item->Pose.Position.x;
			int rz = laraItem->Pose.Position.z - item->Pose.Position.z;
			float sinY = phd_sin(item->Pose.Orientation.y);
			float cosY = phd_cos(item->Pose.Orientation.y);

			int sideShift = rx * sinY + rz * cosY;
			if (sideShift > DRAGON_LCOL && sideShift < DRAGON_RCOL)
			{
				int shift = rx * cosY - rz * sinY;
				if (shift <= DRAGON_CLOSE && shift >= DRAGON_FAR)
					return;

				int angle = laraItem->Pose.Orientation.y - item->Pose.Orientation.y;

				int anim = item->Animation.AnimNumber - Objects[ID_DRAGON_BACK].animIndex;
				int frame = item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase;

				if ((anim == DRAGON_ANIM_DEAD || (anim == DRAGON_ANIM_DEAD + 1 && frame <= DRAGON_ALMOST_LIVE)) &&
					TrInput & IN_ACTION &&
					item->ObjectNumber == ID_DRAGON_BACK &&
					!laraItem->Animation.IsAirborne &&
					shift <= DRAGON_MID &&
					shift > (DRAGON_CLOSE - 350) &&
					sideShift > -350 &&
					sideShift < 350 &&
					angle >(ANGLE(45.0f) - ANGLE(30.0f)) &&
					angle < (ANGLE(45.0f) + ANGLE(30.0f)))
				{
					laraItem->Animation.AnimNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex;
					laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
					laraItem->Animation.ActiveState = 0;
					laraItem->Animation.TargetState = 7;

					laraItem->Pose = item->Pose;
					laraItem->Animation.IsAirborne = false;
					laraItem->Animation.Velocity.y = 0.0f;
					laraItem->Animation.Velocity.z = 0.0f;

					if (item->RoomNumber != laraItem->RoomNumber)
						ItemNewRoom(Lara.ItemNumber, item->RoomNumber);

					AnimateItem(LaraItem);

					Lara.ExtraAnim = 1;
					Lara.Control.HandStatus = HandStatus::Busy;
					Lara.HitDirection = -1;

					laraItem->Model.MeshIndex[LM_RHAND] = Objects[ID_LARA_EXTRA_ANIMS].meshIndex + LM_RHAND;

					((CreatureInfo*)g_Level.Items[(short)item->Data].Data)->Flags = -1;

					return;
				}

				if (shift < DRAGON_MID)
					shift = DRAGON_CLOSE - shift;
				else
					shift = DRAGON_FAR - shift;

				laraItem->Pose.Position.x += shift * cosY;
				laraItem->Pose.Position.z -= shift * sinY;

				return;
			}
		}

		ItemPushItem(item, laraItem, coll, 1, 0);
	}

	static void TriggerFireBreath(ItemInfo* item, const BiteInfo& bite, const Vector3i& speed, ItemInfo* enemy)
	{
		for (int i = 0; i < 3; i++)
		{
			auto* spark = GetFreeParticle();

			spark->on = true;
			spark->sR = (GetRandomControl() & 0x1F) + 48;
			spark->sG = 38;
			spark->sB = 255;
			spark->dR = (GetRandomControl() & 0x3F) - 64;
			spark->dG = (GetRandomControl() & 0x3F) + -128;
			spark->dB = 32;
			spark->colFadeSpeed = 12;
			spark->fadeToBlack = 8;
			spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;

			auto pos1 = GetJointPosition(item, bite.meshNum, Vector3i(-4, -30, -4) + bite.Position);

			spark->x = (GetRandomControl() & 0x1F) + pos1.x - 16;
			spark->y = (GetRandomControl() & 0x1F) + pos1.y - 16;
			spark->z = (GetRandomControl() & 0x1F) + pos1.z - 16;

			auto pos2 = GetJointPosition(item, bite.meshNum, Vector3i(-4, -30, -4) + bite.Position + speed);

			int v = (GetRandomControl() & 0x3F) + 192;

			spark->life = spark->sLife = v / 6;

			spark->xVel = v * (pos2.x - pos1.x) / 10;
			spark->yVel = v * (pos2.y - pos1.y) / 10;
			spark->zVel = v * (pos2.z - pos1.z) / 10;

			spark->friction = 85;
			spark->gravity = -16 - (GetRandomControl() & 0x1F);
			spark->maxYvel = 0;
			spark->flags = SP_FIRE | SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;

			spark->scalar = 3;
			spark->dSize = (v * ((GetRandomControl() & 7) + 60)) / 256;
			spark->sSize = spark->dSize / 4;
			spark->size = spark->dSize / 2;
		}
	}

	void DragonControl(short backItemNumber)
	{
		auto* back = &g_Level.Items[backItemNumber];
		if (back->Data && back->ObjectNumber == ID_DRAGON_FRONT)
			return;

		short itemNumber = (short)back->Data;
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		short headingAngle = 0;
		short head = 0;

		bool isAhead;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != DRAGON_STATE_DEATH)
			{
				SetAnimation(item, 21);
				creature->Flags = 0;
			}
			else if (creature->Flags >= 0)
			{
				CreateBartoliLight(itemNumber, 1);
				creature->Flags++;

				if (creature->Flags == DRAGON_LIVE_TIME)
					item->Animation.TargetState = DRAGON_STATE_IDLE;

				if (creature->Flags == DRAGON_LIVE_TIME + DRAGON_ALMOST_LIVE)
					item->HitPoints = Objects[ID_DRAGON_FRONT].HitPoints / 2;
			}
			else
			{
				if (creature->Flags > -20)
					CreateBartoliLight(itemNumber, 2);

				if (creature->Flags == -100)
				{
					createDragonBone(itemNumber);
				}
				else if (creature->Flags == -200)
				{
					DisableEntityAI(itemNumber);
					KillItem(backItemNumber);
					back->Status = ITEM_DEACTIVATED;
					KillItem(itemNumber);
					item->Status = ITEM_DEACTIVATED;
					return;
				}
				else if (creature->Flags < -100)
				{
					item->Pose.Position.y += 10;
					back->Pose.Position.y += 10;
				}

				creature->Flags--;
				return;
			}
		}
		else
		{
			AI_INFO ai;
			CreatureAIInfo(item, &ai);

			GetCreatureMood(item, &ai, true);
			CreatureMood(item, &ai, true);
			headingAngle = CreatureTurn(item, DRAGON_STATE_WALK_TURN);

			isAhead = (ai.ahead && ai.distance > DRAGON_CLOSE_RANGE && ai.distance < DRAGON_STATE_IDLE_RANGE);

			if (item->TouchBits.TestAny())
				DoDamage(creature->Enemy, DRAGON_CONTACT_DAMAGE);

			switch (item->Animation.ActiveState)
			{
			case DRAGON_STATE_IDLE:
				item->Pose.Orientation.y -= headingAngle;

				if (!isAhead)
				{
					if (ai.distance > DRAGON_STATE_IDLE_RANGE || !ai.ahead)
					{
						item->Animation.TargetState = DRAGON_STATE_WALK;
					}
					else if (ai.ahead && ai.distance < DRAGON_CLOSE_RANGE && !creature->Flags)
					{
						creature->Flags = 1;
						if (ai.angle < 0)
							item->Animation.TargetState = DRAGON_STATE_SWIPE_LEFT;
						else
							item->Animation.TargetState = DRAGON_STATE_SWIPE_RIGHT;
					}
					else if (ai.angle < 0)
					{
						item->Animation.TargetState = DRAGON_STATE_TURN_LEFT;
					}
					else
					{
						item->Animation.TargetState = DRAGON_STATE_TURN_RIGHT;
					}
				}
				else
				{
					item->Animation.TargetState = DRAGON_STATE_AIM_1;
				}

				break;

			case DRAGON_STATE_SWIPE_LEFT:
				if (item->TouchBits.Test(DragonSwipeAttackJointsLeft))
				{
					DoDamage(creature->Enemy, DRAGON_SWIPE_ATTACK_DAMAGE);
					creature->Flags = 0;
				}

				break;

			case DRAGON_STATE_SWIPE_RIGHT:
				if (item->TouchBits.Test(DragonSwipeAttackJointsRight))
				{
					DoDamage(creature->Enemy, DRAGON_SWIPE_ATTACK_DAMAGE);
					creature->Flags = 0;
				}

				break;

			case DRAGON_STATE_WALK:
				creature->Flags = 0;

				if (isAhead)
					item->Animation.TargetState = DRAGON_STATE_IDLE;
				else if (headingAngle < -DRAGON_NEED_TURN)
				{
					if (ai.distance < DRAGON_STATE_IDLE_RANGE && ai.ahead)
						item->Animation.TargetState = DRAGON_STATE_IDLE;
					else
						item->Animation.TargetState = DRAGON_STATE_LEFT;
				}
				else if (headingAngle > DRAGON_NEED_TURN)
				{
					if (ai.distance < DRAGON_STATE_IDLE_RANGE && ai.ahead)
						item->Animation.TargetState = DRAGON_STATE_IDLE;
					else
						item->Animation.TargetState = DRAGON_STATE_RIGHT;
				}

				break;

			case DRAGON_STATE_LEFT:
				if (headingAngle > -DRAGON_NEED_TURN || isAhead)
					item->Animation.TargetState = DRAGON_STATE_WALK;

				break;

			case DRAGON_STATE_RIGHT:
				if (headingAngle < DRAGON_NEED_TURN || isAhead)
					item->Animation.TargetState = DRAGON_STATE_WALK;

				break;

			case DRAGON_STATE_TURN_LEFT:
				item->Pose.Orientation.y += -(ANGLE(1.0f) - headingAngle);
				creature->Flags = 0;
				break;

			case DRAGON_STATE_TURN_RIGHT:
				item->Pose.Orientation.y += (ANGLE(1.0f) - headingAngle);
				creature->Flags = 0;
				break;

			case DRAGON_STATE_AIM_1:
				item->Pose.Orientation.y -= headingAngle;

				if (ai.ahead)
					head = -ai.angle;

				if (isAhead)
				{
					item->Animation.TargetState = DRAGON_STATE_FIRE_1;
					creature->Flags = 30;
				}
				else
				{
					item->Animation.TargetState = DRAGON_STATE_AIM_1;
					creature->Flags = 0;
				}

				break;

			case DRAGON_STATE_FIRE_1:
				item->Pose.Orientation.y -= headingAngle;
				SoundEffect(SFX_TR2_DRAGON_FIRE, &item->Pose);

				if (ai.ahead)
					head = -ai.angle;

				if (creature->Flags)
				{
					if (ai.ahead)
						TriggerFireBreath(item, DragonMouthBite, Vector3i(0, 0, 300), creature->Enemy);
					
					creature->Flags--;
				}
				else
				{
					item->Animation.TargetState = DRAGON_STATE_IDLE;
				}

				break;
			}
		}

		CreatureJoint(item, 0, head);
		CreatureAnimation(itemNumber, headingAngle, 0);

		back->Animation.ActiveState = item->Animation.ActiveState;
		back->Animation.AnimNumber = Objects[ID_DRAGON_BACK].animIndex + (item->Animation.AnimNumber - Objects[ID_DRAGON_FRONT].animIndex);
		back->Animation.FrameNumber = g_Level.Anims[back->Animation.AnimNumber].frameBase + (item->Animation.FrameNumber - g_Level.Anims[item->Animation.AnimNumber].frameBase);
		back->Pose = item->Pose;

		if (back->RoomNumber != item->RoomNumber)
			ItemNewRoom(backItemNumber, item->RoomNumber);
	}

	static void CreateDragonFront(ItemInfo* bartoliItem, ItemInfo* dragonBackItem)
	{
		short frontItemNumber = CreateItem();

		if (frontItemNumber != NO_ITEM && dragonBackItem != nullptr)
		{
			auto* dragonFrontItem = &g_Level.Items[frontItemNumber];
			dragonFrontItem->Pose.Position = bartoliItem->Pose.Position;
			dragonFrontItem->Pose.Orientation.y = bartoliItem->Pose.Orientation.y;
			dragonFrontItem->RoomNumber = bartoliItem->RoomNumber;
			dragonFrontItem->ObjectNumber = ID_DRAGON_FRONT;
			dragonFrontItem->Model.Color = bartoliItem->Model.Color;
			InitialiseItem(frontItemNumber);
			g_Level.NumItems++;
			dragonBackItem->Data = frontItemNumber;
		}
		else
		{
			TENLog("Failed to create the dragon front from Marco Bartoli.", LogLevel::Warning);
		}
	}

	static void CreateDragon(ItemInfo* bartoliItem)
	{
		short backItemNumber = CreateItem();
		if (backItemNumber != NO_ITEM)
		{
			auto* dragonBackItem = &g_Level.Items[backItemNumber];
			dragonBackItem->Pose.Position = bartoliItem->Pose.Position;
			dragonBackItem->Pose.Orientation.y = bartoliItem->Pose.Orientation.y;
			dragonBackItem->RoomNumber = bartoliItem->RoomNumber;
			dragonBackItem->ObjectNumber = ID_DRAGON_BACK;
			dragonBackItem->Model.Color = bartoliItem->Model.Color;
			InitialiseItem(backItemNumber);

			// No need to draw it if alive.
			dragonBackItem->MeshBits.Clear(DragonBackSpineJoints);
			bartoliItem->Data = backItemNumber;
			g_Level.NumItems++;

			CreateDragonFront(bartoliItem, dragonBackItem);
		}
		else
		{
			TENLog("Failed to create dragon back from Marco Bartoli.", LogLevel::Warning);
		}
	}

	void BartoliControl(short itemNumber)
	{
		ItemInfo* back, *front;
		short frontItem, backItem;
		auto* item = &g_Level.Items[itemNumber];

		if (item->Timer)
		{
			item->Timer++;
			if (!(item->Timer & 7))
				Camera.bounce = item->Timer;

			CreateBartoliLight(itemNumber, 1);
			AnimateItem(item);

			if (item->Timer == BOOM_TIME ||
				item->Timer == BOOM_TIME + 10 ||
				item->Timer == BOOM_TIME + 20)
			{
				frontItem = CreateItem();
				if (frontItem != NO_ITEM)
				{
					front = &g_Level.Items[frontItem];

					if (item->Timer == BOOM_TIME)
					{
						front->ObjectNumber = ID_SPHERE_OF_DOOM;
					}
					else if (item->Timer == BOOM_TIME + 10)
					{
						front->ObjectNumber = ID_SPHERE_OF_DOOM2;
					}
					else
					{
						front->ObjectNumber = ID_SPHERE_OF_DOOM3;
					}

					front->Pose.Position.x = item->Pose.Position.x;
					front->Pose.Position.y = item->Pose.Position.y + CLICK(1);
					front->Pose.Position.z = item->Pose.Position.z;
					front->RoomNumber = item->RoomNumber;
					front->Model.Color = item->Model.Color;

					InitialiseItem(frontItem);
					AddActiveItem(frontItem);

					// Time before fading away.
					front->Timer = 100;
					front->Status = ITEM_ACTIVE;
				}
			}
			else if (item->Timer >= BOOM_TIME + 30)
			{
				CreateDragon(item);

				backItem = (short)item->Data;
				back = &g_Level.Items[backItem];

				frontItem = (short)back->Data;
				front = &g_Level.Items[frontItem];

				front->TouchBits = back->TouchBits = NO_JOINT_BITS;
				EnableEntityAI(frontItem, true);
				AddActiveItem(frontItem);
				AddActiveItem(backItem);
				back->Status = ITEM_ACTIVE;

				KillItem(itemNumber);
			}
		}
		else if (abs(LaraItem->Pose.Position.x - item->Pose.Position.x) < BARTOLI_RANGE &&
			abs(LaraItem->Pose.Position.z - item->Pose.Position.z) < BARTOLI_RANGE)
		{
			item->Timer = 1;
		}
	}

	void SphereOfDoomControl(short itemNumber)
	{
		// Expend over time.
		auto* item = &g_Level.Items[itemNumber];
		if (item->Timer > 0)
		{
			item->Timer--;
			if (!item->Model.Mutator.empty())
				item->Model.Mutator[0].Scale += Vector3(0.5f);
		}
		else
		{
			item->Model.Color.w -= 0.05f;
			if (item->Model.Color.w <= 0.0f)
				KillItem(itemNumber);
		}
	}
}
