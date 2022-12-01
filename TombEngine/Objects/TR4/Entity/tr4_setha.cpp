#include "framework.h"
#include "Objects/TR4/Entity/tr4_setha.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/effects/effects.h"
#include "Game/effects/item_fx.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Math/Math.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Math;
using namespace TEN::Effects::Items;

namespace TEN::Entities::TR4
{
	enum SethaState
	{
		SETH_STATE_IDLE = 1,
		SETH_STATE_WALK_FORWARD = 2,
		SETH_STATE_RUN_FORWARD = 3,
		SETH_STATE_KNEEL_ATTACK = 4,
		SETH_STATE_JUMP = 5,
		SETH_STATE_GET_SHOT = 6,
		SETH_STATE_GET_HEAVY_SHOT = 7,
		SETH_STATE_ATTACK_GRAB = 8,
		SETH_STATE_ATTACK_KILL_LARA = 9,
		SETH_STATE_REGENERATE = 10,
		SETH_STATE_SHOOT_RIGHT_AND_LEFT = 11,
		SETH_STATE_JUMP_AIR_SHOOT = 12,
		SETH_STATE_BIG_PROJECTILE = 13,
		SETH_STATE_FLY = 14,
		SETH_STATE_FLY_AIR_SHOOT = 15,
		SETH_STATE_FLY_GET_HEAVY_SHOT = 16
	};

	enum SethaAnim
	{
		SETH_ANIM_SHOOT_RIGHT_AND_LEFT = 0,
		SETH_ANIM_PREPARE_JUMP = 1,
		SETH_ANIM_JUMP = 2,
		SETH_ANIM_JUMP_LANDING = 3,
		SETH_ANIM_JUMP_AIR_SHOOT = 4,
		SETH_ANIM_AIM_BIG_PROJECTILE = 5,
		SETH_ANIM_SHOOT_BIG_PROJECTILE = 6,
		SETH_ANIM_UNAIM_BIG_PROJECTILE = 7,
		SETH_ANIM_WALK_FORWARD = 8,
		SETH_ANIM_WALK_FORWARD_TO_RUN = 9,
		SETH_ANIM_RUN_FORWARD = 10,
		SETH_ANIM_IDLE_GET_SHOT = 11,
		SETH_ANIM_IDLE = 12,
		SETH_ANIM_ATTACK_GRAB = 13,
		SETH_ANIM_ATTACK_KILL_LARA = 14,
		SETH_ANIM_PREPARE_KNEEL_ATTACK = 15,
		SETH_ANIM_KNEEL_ATTACK = 16,
		SETH_ANIM_GET_HEAVY_SHOT_SOMERSAULT = 17,
		SETH_ANIM_GET_HEAVY_SHOT_SOMERSAULT_END = 18,
		SETH_ANIM_REGENERATE = 19,
		SETH_ANIM_WALK_FORWARD_TO_IDLE = 20,
		SETH_ANIM_IDLE_TO_WALK = 21,
		SETH_ANIM_RUN_FORWARD_TO_IDLE = 22,
		SETH_ANIM_RUN_FORWARD_TO_WALK =23,
		SETH_ANIM_FLY_AIR_SHOOT = 24,
		SETH_ANIM_FLY_GET_HEAVY_SHOT = 25,
		SETH_ANIM_IDLE_JUMP_TO_FLY = 26,
		SETH_ANIM_FLY_TO_LAND = 27,
		SETH_ANIM_FLY_IDLE = 28
	};


	const auto SethaBite1	= BiteInfo(Vector3(0.0f, 220.0f, 50.0f), 17);
	const auto SethaBite2	= BiteInfo(Vector3(0.0f, 220.0f, 50.0f), 13);
	const auto SethaAttack1 = BiteInfo(Vector3(-16.0f, 200.0f, 32.0f), 13);
	const auto SethaAttack2 = BiteInfo(Vector3(16.0f, 200.0f, 32.0f), 17);
	constexpr auto LARA_ANIM_SETH_DEATH_ANIM = 14;

	void InitialiseSetha(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		ClearItem(itemNumber);
		InitialiseCreature(itemNumber);
		SetAnimation(item, SETH_ANIM_IDLE);
	}

	void SethaKill(ItemInfo* item, ItemInfo* laraItem)
	{
		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + SETH_ANIM_ATTACK_KILL_LARA;
		item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
		item->Animation.TargetState = SETH_STATE_ATTACK_KILL_LARA;
		item->Animation.ActiveState = SETH_STATE_ATTACK_KILL_LARA;

		LaraItem->Animation.AnimNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex + LARA_ANIM_SETH_DEATH_ANIM;
		LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
		LaraItem->Animation.ActiveState = 13;
		LaraItem->Animation.TargetState = 13;
		LaraItem->Animation.IsAirborne = false;
		LaraItem->Pose = Pose(item->Pose.Position, 0, item->Pose.Orientation.y, 0);
		if (item->RoomNumber != LaraItem->RoomNumber)
			ItemNewRoom(Lara.ItemNumber, item->RoomNumber);
		AnimateItem(LaraItem);
		Lara.ExtraAnim = 1;
		LaraItem->HitPoints = -1;
		Lara.HitDirection = -1;
		Lara.Air = -1;
		Lara.Control.HandStatus = HandStatus::Busy;
		Lara.Control.Weapon.GunType = LaraWeaponType::None;

		Camera.pos.RoomNumber = LaraItem->RoomNumber;
		Camera.type = CameraType::Chase;
		Camera.flags = CF_FOLLOW_CENTER;
		Camera.targetAngle = ANGLE(170.0f);
		Camera.targetElevation = -ANGLE(25.0f);
	}

	void TriggerSethaSparks1(int x, int y, int z, short xv, short yv, short zv)
	{
		int dx = LaraItem->Pose.Position.x - x;
		int dz = LaraItem->Pose.Position.z - z;

		if (dx >= -SECTOR(16) && dx <= SECTOR(16) &&
			dz >= -SECTOR(16) && dz <= SECTOR(16))
		{
			auto* spark = GetFreeParticle();

			spark->on = 1;
			spark->sR = 0;
			spark->sG = 0;
			spark->sB = 0;
			spark->dR = 64;
			spark->dG = (GetRandomControl() & 0x7F) + 64;
			spark->dB = spark->dG + 32;
			spark->life = 16;
			spark->sLife = 16;
			spark->colFadeSpeed = 4;
			spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
			spark->fadeToBlack = 4;
			spark->x = x;
			spark->y = y;
			spark->z = z;
			spark->xVel = xv;
			spark->yVel = yv;
			spark->zVel = zv;
			spark->friction = 34;
			spark->scalar = 1;
			spark->sSize = spark->size = (GetRandomControl() & 3) + 4;
			spark->maxYvel = 0;
			spark->gravity = 0;
			spark->dSize = (GetRandomControl() & 1) + 1;
			spark->flags = 0;
		}
	}

	void TriggerSethaSparks2(short itemNumber, char node, int size)
	{
		int dx = LaraItem->Pose.Position.x - g_Level.Items[itemNumber].Pose.Position.x;
		int dz = LaraItem->Pose.Position.z - g_Level.Items[itemNumber].Pose.Position.z;

		if (dx >= -SECTOR(16) && dx <= SECTOR(16) &&
			dz >= -SECTOR(16) && dz <= SECTOR(16))
		{
			auto* spark = GetFreeParticle();

			spark->on = 1;
			spark->sR = 0;
			spark->sG = 0;
			spark->sB = 0;
			spark->dR = 0;
			spark->dG = (GetRandomControl() & 0x7F) + 32;
			spark->dB = spark->dG + 64;
			spark->fadeToBlack = 8;
			spark->colFadeSpeed = (GetRandomControl() & 3) + 4;
			spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
			spark->life = spark->sLife = (GetRandomControl() & 7) + 20;
			spark->x = (GetRandomControl() & 0xF) - 8;
			spark->y = 0;
			spark->z = (GetRandomControl() & 0xF) - 8;
			spark->xVel = GetRandomControl() - 128;
			spark->yVel = 0;
			spark->zVel = GetRandomControl() - 128;
			spark->friction = 5;
			spark->flags = SP_NODEATTACH | SP_EXPDEF | SP_ITEM | SP_ROTATE | SP_SCALE | SP_DEF;
			spark->rotAng = GetRandomControl() & 0xFFF;

			if (Random::TestProbability(1 / 2.0f))
				spark->rotAdd = -32 - (GetRandomControl() & 0x1F);
			else
				spark->rotAdd = (GetRandomControl() & 0x1F) + 32;

			spark->maxYvel = 0;
			spark->gravity = (GetRandomControl() & 0x1F) + 16;
			spark->fxObj = itemNumber;
			spark->nodeNumber = node;
			spark->scalar = 2;
			spark->sSize = spark->size = GetRandomControl() & 0xF + size;
			spark->dSize = spark->size / 16;
		}
	}

	void SethaThrowAttack(Pose* pose, short roomNumber, int flags)
	{
		short fxNumber = CreateNewEffect(roomNumber);
		if (fxNumber != -1)
		{
			auto* fx = &EffectList[fxNumber];

			fx->pos.Position.x = pose->Position.x;
			fx->pos.Position.y = pose->Position.y - (GetRandomControl() & 0x3F) - 32;
			fx->pos.Position.z = pose->Position.z;

			fx->pos.Orientation.x = pose->Orientation.x;
			fx->pos.Orientation.y = pose->Orientation.y;
			fx->pos.Orientation.z = 0;
			fx->roomNumber = roomNumber;
			fx->counter = 2 * GetRandomControl() + -ANGLE(180.0f);
			fx->flag1 = flags;
			fx->objectNumber = ID_ENERGY_BUBBLES;
			fx->speed = (GetRandomControl() & 0x1F) - (flags == 1 ? 64 : 0) + 96;
			fx->frameNumber = Objects[ID_ENERGY_BUBBLES].meshIndex + 2 * flags;
		}
	}

	void SethaControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);

		int x = item->Pose.Position.x;
		int y = item->Pose.Position.y;
		int z = item->Pose.Position.z;

		int dx = 870 * phd_sin(item->Pose.Orientation.y);
		int dz = 870 * phd_cos(item->Pose.Orientation.y);

		int ceiling = GetCollision(x, y, z, item->RoomNumber).Position.Ceiling;

		x += dx;
		z += dz;
		int height1 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

		x += dx;
		z += dz;
		int height2 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

		x += dx;
		z += dz;
		int height3 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

		bool canJump = false;
		if ((y < (height1 - CLICK(1.5f)) || y < (height2 - CLICK(1.5f))) &&
			(y < (height3 + CLICK(1)) && y > (height3 - CLICK(1)) || height3 == NO_HEIGHT))
		{
			canJump = true;
		}

		x = item->Pose.Position.x - dx;
		z = item->Pose.Position.z - dz;
		int height4 = GetCollision(x, y, z, item->RoomNumber).Position.Floor;

		AI_INFO AI;
		short angle = 0;

		if (item->HitPoints <= 0)
			item->HitPoints = 0;
		else
		{
			if (item->AIBits & AMBUSH)
				GetAITarget(creature);
			else
				creature->Enemy = LaraItem;

			CreatureAIInfo(item, &AI);

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			angle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case SETH_STATE_IDLE:
				creature->LOT.IsJumping = false;
				creature->Flags = 0;

				if (item->Animation.RequiredState != NO_STATE)
				{
					item->Animation.TargetState = item->Animation.RequiredState;
					break;
				}
				else if (AI.distance < pow(SECTOR(1), 2) && AI.bite)
				{
					item->Animation.TargetState = SETH_STATE_ATTACK_GRAB;
					break;
				}
				else if (LaraItem->Pose.Position.y >= (item->Pose.Position.y - SECTOR(1)))
				{
					if (AI.distance < pow(SECTOR(2.5f), 2) &&
						AI.ahead && Random::TestProbability(1.0f / 2) &&
						Targetable(item, &AI))
					{
						item->Animation.TargetState = SETH_STATE_SHOOT_RIGHT_AND_LEFT;
						item->ItemFlags[0] = 0;
						break;
					}
					else if (ceiling != NO_HEIGHT &&
						ceiling < (item->Pose.Position.y - SECTOR(1.75f)) &&
						height4 != NO_HEIGHT &&
						height4 > (item->Pose.Position.y - SECTOR(1)) &&
						Random::TestProbability(1.0f / 2))
					{
						item->Pose.Position.y -= SECTOR(1.5f);
						if (Targetable(item, &AI))
						{
							item->Pose.Position.y += SECTOR(1.5f);
							item->Animation.TargetState = SETH_STATE_JUMP_AIR_SHOOT;
							item->ItemFlags[0] = 0;
						}
						else
						{
							item->Pose.Position.y += SECTOR(1.5f);
							item->Animation.TargetState = SETH_STATE_WALK_FORWARD;
						}

						break;
					}
					else
					{
						if (AI.distance < pow(SECTOR(3), 2) &&
							AI.angle < SECTOR(6) &&
							AI.angle > -SECTOR(6) &&
							AI.ahead)
						{
							if (Targetable(item, &AI))
							{
								item->Animation.TargetState = SETH_STATE_KNEEL_ATTACK;
								break;
							}
						}
						else if (AI.distance < pow(SECTOR(4), 2) &&
							AI.angle < ANGLE(45.0f) &&
							AI.angle > -ANGLE(45.0f) &&
							height4 != NO_HEIGHT &&
							height4 >= (item->Pose.Position.y - CLICK(1)) &&
							Targetable(item, &AI))
						{
							item->Animation.TargetState = SETH_STATE_BIG_PROJECTILE;
							item->ItemFlags[0] = 0;
							break;
						}
						else if (canJump)
						{
							item->Animation.TargetState = SETH_STATE_JUMP;
							break;
						}
					}
				}
				else
				{
					if (creature->ReachedGoal)
					{
						item->Animation.TargetState = SETH_STATE_FLY;
						break;
					}
					else
					{
						item->AIBits = AMBUSH;
						creature->HurtByLara = true;
					}
				}

				item->Animation.TargetState = SETH_STATE_WALK_FORWARD;
				break;

			case SETH_STATE_WALK_FORWARD:
				creature->MaxTurn = ANGLE(7.0f);

				if (AI.bite &&
					AI.distance < pow(SECTOR(4), 2) ||
					canJump || creature->ReachedGoal)
				{
					item->Animation.TargetState = SETH_STATE_IDLE;
				}
				else if (AI.distance > pow(SECTOR(3), 2))
					item->Animation.TargetState = SETH_STATE_RUN_FORWARD;
				
				break;

			case SETH_STATE_RUN_FORWARD:
				creature->MaxTurn = ANGLE(11.0f);

				if (AI.bite &&
					AI.distance < pow(SECTOR(4), 2) ||
					canJump || creature->ReachedGoal)
				{
					item->Animation.TargetState = SETH_STATE_IDLE;
				}
				else if (AI.distance < pow(SECTOR(3), 2))
					item->Animation.TargetState = SETH_STATE_WALK_FORWARD;
				
				break;

			case SETH_STATE_KNEEL_ATTACK:
				if (canJump)
				{
					if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + SETH_ANIM_PREPARE_KNEEL_ATTACK &&
						item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
					{
						creature->MaxTurn = 0;
						creature->ReachedGoal = true;
					}
				}

				if (!creature->Flags)
				{
					if (item->TouchBits.TestAny())
					{
						if (item->Animation.AnimNumber == Objects[item->ObjectNumber].animIndex + SETH_ANIM_KNEEL_ATTACK)
						{
							if (item->TouchBits & 0xE000)
							{
								DoDamage(creature->Enemy, 200);
								CreatureEffect2(item, SethaBite1, 25, -1, DoBloodSplat);
								creature->Flags = 1;
							}

							if (item->TouchBits & 0xE0000)
							{
								DoDamage(creature->Enemy, 200);
								CreatureEffect2(item, SethaBite2, 25, -1, DoBloodSplat);
								creature->Flags = 1;
							}
						}
					}
				}

				break;

			case SETH_STATE_JUMP:
				creature->MaxTurn = 0;
				creature->ReachedGoal = true;
				break;

			case SETH_STATE_GET_HEAVY_SHOT:
				if (item->Animation.AnimNumber == Objects[item->Animation.AnimNumber].animIndex + SETH_ANIM_GET_HEAVY_SHOT_SOMERSAULT &&
					item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd)
				{
					if (Random::TestProbability(1 / 2.0f))
						item->Animation.RequiredState = SETH_STATE_REGENERATE;
				}

				break;

			case SETH_STATE_ATTACK_GRAB:
				creature->MaxTurn = 0;

				if (abs(AI.angle) >= ANGLE(3.0f))
				{
					if (AI.angle >= 0)
						item->Pose.Orientation.y += ANGLE(3.0f);
					else
						item->Pose.Orientation.y -= ANGLE(3.0f);
				}
				else
					item->Pose.Orientation.y += AI.angle;

				if (!creature->Flags)
				{
					if (item->TouchBits.TestAny())
					{
						if (item->Animation.FrameNumber > g_Level.Anims[item->Animation.AnimNumber].frameBase + SETH_ANIM_PREPARE_KNEEL_ATTACK &&
							item->Animation.FrameNumber < g_Level.Anims[item->Animation.AnimNumber].frameBase + SETH_ANIM_IDLE_JUMP_TO_FLY)
						{
							DoDamage(creature->Enemy, 250);
							CreatureEffect2(item, SethaBite1, 25, -1, DoBloodSplat);
							creature->Flags = 1;
						}
					}
				}

				if (LaraItem->HitPoints < 0)
				{
					SethaKill(item, LaraItem);
					ItemCustomBurn(LaraItem, Vector3(0.0f, 0.8f, 0.1f), Vector3(0.0f, 0.9f, 0.8f), 6 * FPS);
					creature->MaxTurn = 0;
					return;
				}

				break;

			case SETH_STATE_SHOOT_RIGHT_AND_LEFT:
			case SETH_STATE_JUMP_AIR_SHOOT:
			case SETH_STATE_BIG_PROJECTILE:
			case SETH_STATE_FLY_AIR_SHOOT:
				creature->MaxTurn = 0;

				if (item->Animation.ActiveState == SETH_STATE_FLY_AIR_SHOOT)
					creature->Target.y = LaraItem->Pose.Position.y;

				if (abs(AI.angle) >= ANGLE(3.0f))
				{
					if (AI.angle >= 0)
						item->Pose.Orientation.y += ANGLE(3.0f);
					else
						item->Pose.Orientation.y -= ANGLE(3.0f);
					
					SethaAttack(itemNumber);
				}
				else
				{
					item->Pose.Orientation.y += AI.angle;
					SethaAttack(itemNumber);
				}

				break;

			case SETH_STATE_FLY:
				if (item->Animation.AnimNumber != Objects[item->Animation.AnimNumber].animIndex + SETH_ANIM_IDLE_JUMP_TO_FLY)
				{
					item->Animation.IsAirborne = false;
					creature->MaxTurn = 0;
					creature->Target.y = LaraItem->Pose.Position.y;
					creature->LOT.Fly = 16;

					if (abs(AI.angle) >= ANGLE(3.0f))
					{
						if (AI.angle >= 0)
							item->Pose.Orientation.y += ANGLE(3.0f);
						else
							item->Pose.Orientation.y -= ANGLE(3.0f);
					}
					else
						item->Pose.Orientation.y += AI.angle;
				}

				if (LaraItem->Pose.Position.y <= (item->Floor - SECTOR(0.5f)))
				{
					if (Targetable(item, &AI))
					{
						item->ItemFlags[0] = 0;
						item->Animation.TargetState = SETH_STATE_FLY_AIR_SHOOT;
					}
				}
				else
				{
					item->Animation.IsAirborne = true;
					creature->LOT.Fly = 0;

					if ((item->Pose.Position.y - item->Floor) > 0)
						item->Animation.TargetState = SETH_STATE_IDLE;
				}

				break;

			default:
				break;
			}
		}

		if (item->HitStatus)
		{
			if ((Lara.Control.Weapon.GunType == LaraWeaponType::Shotgun || Lara.Control.Weapon.GunType == LaraWeaponType::Revolver) &&
				AI.distance < pow(SECTOR(2), 2) &&
				!(creature->LOT.IsJumping))
			{
				if (item->Animation.ActiveState != SETH_STATE_JUMP_AIR_SHOOT)
				{
					if (item->Animation.ActiveState <= SETH_STATE_BIG_PROJECTILE)
					{
						if (abs(height4 - item->Pose.Position.y) >= SECTOR(0.5f))
						{
							item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + SETH_ANIM_IDLE_GET_SHOT;
							item->Animation.ActiveState = SETH_STATE_GET_SHOT;
							item->Animation.TargetState = SETH_STATE_GET_SHOT;
						}
						else
						{
							item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + SETH_ANIM_GET_HEAVY_SHOT_SOMERSAULT;
							item->Animation.ActiveState = SETH_STATE_GET_HEAVY_SHOT;
							item->Animation.TargetState = SETH_STATE_GET_HEAVY_SHOT;
						}
					}
					else
					{
						item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + SETH_ANIM_FLY_GET_HEAVY_SHOT;
						item->Animation.ActiveState = SETH_STATE_FLY_GET_HEAVY_SHOT;
						item->Animation.TargetState = SETH_STATE_FLY_GET_HEAVY_SHOT;
					}

					item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				}
			}
		}

		CreatureAnimation(itemNumber, angle, 0);
	}

	void SethaAttack(int itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		item->ItemFlags[0]++;

		auto pos1 = GetJointPosition(item, SethaAttack1.meshNum, Vector3i(SethaAttack1.Position));
		auto pos2 = GetJointPosition(item, SethaAttack2.meshNum, Vector3i(SethaAttack2.Position));

		int size;

		switch (item->Animation.ActiveState)
		{
		case SETH_STATE_SHOOT_RIGHT_AND_LEFT:
		case SETH_STATE_FLY_AIR_SHOOT:
			if (item->ItemFlags[0] < 78 && (GetRandomControl() & 0x1F) < item->ItemFlags[0])
			{
				for (int i = 0; i < 2; i++)
				{
					auto pos = Vector3i(
						(GetRandomControl() & 0x7FF) + pos1.x - SECTOR(1),
						(GetRandomControl() & 0x7FF) + pos1.y - SECTOR(1),
						(GetRandomControl() & 0x7FF) + pos1.z - SECTOR(1)
					);

					TriggerSethaSparks1(
						pos.x, pos.y, pos.z,
						(pos1.x - pos.x),
						(pos1.y - pos.y),
						(SECTOR(1) - (GetRandomControl() & 0x7FF)));

					pos = Vector3i(
						(GetRandomControl() & 0x7FF) + pos2.x - SECTOR(1),
						(GetRandomControl() & 0x7FF) + pos2.y - SECTOR(1),
						(GetRandomControl() & 0x7FF) + pos2.z - SECTOR(1)
					);

					TriggerSethaSparks1(
						pos.x, pos.y, pos.z,
						(pos2.x - pos.x) * 8,
						(pos2.y - pos.y) * 8,
						(SECTOR(1) - (GetRandomControl() & 0x7FF)) * 8);
				}
			}

			size = item->ItemFlags[0] * 2;
			if (size > 128)
				size = 128;

			if ((Wibble & 0xF) == 8)
			{
				if (item->ItemFlags[0] < 127)
					TriggerSethaSparks2(itemNumber, 2, size);
			}
			else if (!(Wibble & 0xF) && item->ItemFlags[0] < 103)
				TriggerSethaSparks2(itemNumber, 3, size);

			if (item->ItemFlags[0] >= 96 && item->ItemFlags[0] <= 99)
			{
				auto pos = GetJointPosition(item, SethaAttack1.meshNum, Vector3i(SethaAttack1.Position.x, SethaAttack1.Position.y * 2, SethaAttack1.Position.z));
				auto orient = Geometry::GetOrientToPoint(pos1.ToVector3(), pos.ToVector3());
				auto attackPose = Pose(pos1, orient);
				SethaThrowAttack(&attackPose, item->RoomNumber, 0);
			}
			else if (item->ItemFlags[0] >= 122 && item->ItemFlags[0] <= 125)
			{
				auto pos = GetJointPosition(item, SethaAttack2.meshNum, Vector3i(SethaAttack2.Position.x, SethaAttack2.Position.y * 2, SethaAttack2.Position.z));
				auto orient = Geometry::GetOrientToPoint(pos2.ToVector3(), pos.ToVector3());
				auto attackPose = Pose(pos2, orient);
				SethaThrowAttack(&attackPose, item->RoomNumber, 0);
			}

			break;

		case SETH_STATE_JUMP_AIR_SHOOT:
			size = item->ItemFlags[0] * 4;
			if (size > 160)
				size = 160;

			if ((Wibble & 0xF) == 8)
			{
				if (item->ItemFlags[0] < 132)
					TriggerSethaSparks2(itemNumber, 2, size);
			}
			else if (!(Wibble & 0xF) && item->ItemFlags[0] < 132)
				TriggerSethaSparks2(itemNumber, 3, size);

			if (item->ItemFlags[0] >= 60 && item->ItemFlags[0] <= 74 ||
				item->ItemFlags[0] >= 112 && item->ItemFlags[0] <= 124)
			{
				if (Wibble & 4)
				{
					auto pos = GetJointPosition(item, SethaAttack1.meshNum, Vector3i(SethaAttack1.Position.x, SethaAttack1.Position.y * 2, SethaAttack1.Position.z));
					auto orient = Geometry::GetOrientToPoint(pos1.ToVector3(), pos.ToVector3());
					auto attackPose = Pose(pos1, orient);
					SethaThrowAttack(&attackPose, item->RoomNumber, 0);

					pos = GetJointPosition(item, SethaAttack2.meshNum, Vector3i(SethaAttack2.Position.x, SethaAttack2.Position.y * 2, SethaAttack2.Position.z));
					orient = Geometry::GetOrientToPoint(pos2.ToVector3(), pos.ToVector3());
					attackPose = Pose(pos2, orient);
					SethaThrowAttack(&attackPose, item->RoomNumber, 0);
				}
			}

			break;

		case SETH_STATE_BIG_PROJECTILE:
			if (item->ItemFlags[0] > 40 &&
				item->ItemFlags[0] < 100 &&
				(GetRandomControl() & 7) < item->ItemFlags[0] - 40)
			{
				for (int i = 0; i < 2; i++)
				{
					auto pos = Vector3i(
						(GetRandomControl() & 0x7FF) + pos1.x - SECTOR(1),
						(GetRandomControl() & 0x7FF) + pos1.y - SECTOR(1),
						(GetRandomControl() & 0x7FF) + pos1.z - SECTOR(1)
					);

					TriggerSethaSparks1(
						pos.x, pos.y, pos.z,
						(pos1.x - pos.x),
						(pos1.y - pos.y),
						(SECTOR(1) - (GetRandomControl() & 0x7FF)));

					pos = Vector3i(
						(GetRandomControl() & 0x7FF) + pos2.x - SECTOR(1),
						(GetRandomControl() & 0x7FF) + pos2.y - SECTOR(1),
						(GetRandomControl() & 0x7FF) + pos2.z - SECTOR(1)
					);

					TriggerSethaSparks1(
						pos.x, pos.y, pos.z,
						(pos2.x - pos.x),
						(pos2.y - pos.y),
						(SECTOR(1) - (GetRandomControl() & 0x7FF)));
				}
			}

			size = item->ItemFlags[0] * 2;
			if (size > 128)
				size = 128;

			if ((Wibble & 0xF) == 8)
			{
				if (item->ItemFlags[0] < 103)
					TriggerSethaSparks2(itemNumber, 2, size);
			}
			else if (!(Wibble & 0xF) && item->ItemFlags[0] < 103)
				TriggerSethaSparks2(itemNumber, 3, size);

			if (item->ItemFlags[0] == 102)
			{
				auto pos = GetJointPosition(item, SethaAttack1.meshNum, Vector3i(SethaAttack2.Position.x, SethaAttack2.Position.y * 2, SethaAttack2.Position.z));
				auto orient = Geometry::GetOrientToPoint(pos1.ToVector3(), pos.ToVector3());
				auto attackPose = Pose(pos1, orient);
				SethaThrowAttack(&attackPose, item->RoomNumber, 1);
			}

			break;

		default:
			break;
		}
	}
}
