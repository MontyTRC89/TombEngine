#include "framework.h"
#include "Objects/TR3/Entity/tr3_shiva.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/items.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Game/camera.h"
#include "Renderer/Renderer11Enums.h"

using std::vector;

namespace TEN::Entities::TR3
{
	BITE_INFO ShivaBiteLeft = { 0, 0, 920, 13 };
	BITE_INFO ShivaBiteRight = { 0, 0, 920, 22 };
	const vector<int> ShivaAttackLeftJoints = { 10, 13 };
	const vector<int> ShivaAttackRightJoints = { 22, 25 };

	constexpr auto SHIVA_GRAB_ATTACK_DAMAGE = 150;
	constexpr auto SHIVA_DOWNWARD_ATTACK_DAMAGE = 180;

	#define SHIVA_WALK_TURN_ANGLE ANGLE(4.0f)
	#define SHIVA_ATTACK_TURN_ANGLE ANGLE(4.0f)

	constexpr auto LARA_ANIM_SHIVA_DEATH = 7;

	enum ShivaState
	{
		SHIVA_STATE_IDLE = 0,
		SHIVA_STATE_WALK_FORWARD = 1,
		SHIVA_STATE_GUARD_IDLE = 2,
		SHIVA_STATE_WALK_FORWARD_GUARDING = 3,
		SHIVA_STATE_INACTIVE = 4,
		SHIVA_STATE_GRAB_ATTACK = 5,
		SHIVA_STATE_KILL = 6,
		SHIVA_STATE_DOWNWARD_ATTACK = 7,
		SHIVA_STATE_WALK_BACK = 8,
		SHIVA_STATE_DEATH = 9
	};

	enum ShivaAnim
	{
		SHIVA_ANIM_IDLE = 0,
		SHIVA_ANIM_IDLE_TO_WALK_RIGHT = 1,
		SHIVA_ANIM_WALK_TO_IDLE_RIGHT = 2,
		SHIVA_ANIM_WALK_TO_IDLE_LEFT = 3,
		SHIVA_ANIM_IDLE_TO_GUARD_IDLE = 4,
		SHIVA_ANIM_GUARD_IDLE = 5,
		SHIVA_ANIM_GUARD_IDLE_TO_IDLE = 6,
		SHIVA_ANIM_WALK_FORWARD = 7,
		SHIVA_ANIM_WALK_FORWARD_GUARDED = 8,
		SHIVA_ANIM_WALK_FORWARD_TO_GUARDED_RIGHT_3 = 9,
		SHIVA_ANIM_WALK_FORWARD_TO_GUARDED_LEFT_1 = 10,
		SHIVA_ANIM_WALK_FORWARD_TO_GUARDED_LEFT_2 = 11,
		SHIVA_ANIM_WALK_FORWARD_TO_GUARDED_RIGHT_1 = 12,
		SHIVA_ANIM_WALK_FORWARD_TO_GUARDED_RIGHT_2 = 13,
		SHIVA_ANIM_INACTIVE = 14,
		SHIVA_ANIM_WALK_FORWARD_UNGUARD_RIGHT = 15,
		SHIVA_ANIM_WALK_FORWARD_UNGUARD_LEFT = 16,
		SHIVA_ANIM_GRAB_ATTACK = 17,
		SHIVA_ANIM_KILL = 18,
		SHIVA_ANIM_DOWNWARD_ATTACK = 19,
		SHIVA_ANIM_WALK_BACK_LEFT = 20,
		SHIVA_ANIM_WALK_BACK_RIGHT = 21,
		SHIVA_ANIM_DEATH = 22
	};

	static void TriggerShivaSmoke(long x, long y, long z, long uw)
	{
		long dx = LaraItem->Pose.Position.x - x;
		long dz = LaraItem->Pose.Position.z - z;

		if (dx < -SECTOR(16) || dx > SECTOR(16) ||
			dz < -SECTOR(16) || dz > SECTOR(16))
		{
			return;
		}

		auto* sptr = GetFreeParticle();

		sptr->on = 1;
		if (uw)
		{
			sptr->sR = 0;
			sptr->sG = 0;
			sptr->sB = 0;
			sptr->dR = 192;
			sptr->dG = 192;
			sptr->dB = 208;
		}
		else
		{
			sptr->sR = 144;
			sptr->sG = 144;
			sptr->sB = 144;
			sptr->dR = 64;
			sptr->dG = 64;
			sptr->dB = 64;
		}

		sptr->colFadeSpeed = 8;
		sptr->fadeToBlack = 64;
		sptr->sLife = sptr->life = (GetRandomControl() & 31) + 96;

		if (uw)
			sptr->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		else
			sptr->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;

		sptr->extras = 0;
		sptr->dynamic = -1;

		sptr->x = x + (GetRandomControl() & 31) - 16;
		sptr->y = y + (GetRandomControl() & 31) - 16;
		sptr->z = z + (GetRandomControl() & 31) - 16;
		sptr->xVel = ((GetRandomControl() & 4095) - 2048) / 4;
		sptr->yVel = (GetRandomControl() & 255) - 128;
		sptr->zVel = ((GetRandomControl() & 4095) - 2048) / 4;

		if (uw)
		{
			sptr->yVel /= 16;
			sptr->y += 32;
			sptr->friction = 4 | (16);
		}
		else
			sptr->friction = 6;

		sptr->flags = SP_SCALE | SP_DEF | SP_ROTATE | SP_EXPDEF;
		sptr->rotAng = GetRandomControl() & 4095;

		if (GetRandomControl() & 1)
			sptr->rotAdd = -(GetRandomControl() & 15) - 16;
		else
			sptr->rotAdd = (GetRandomControl() & 15) + 16;

		sptr->scalar = 3;

		if (uw)
			sptr->gravity = sptr->maxYvel = 0;
		else
		{
			sptr->gravity = -(GetRandomControl() & 3) - 3;
			sptr->maxYvel = -(GetRandomControl() & 3) - 4;
		}

		long size = (GetRandomControl() & 31) + 128;

		sptr->size = sptr->sSize = size / 4;
		sptr->dSize = size;
		size += (GetRandomControl() & 31) + 32;
		sptr->size = sptr->sSize = size / 8;
		sptr->dSize = size;
	}

	static void ShivaDamage(ItemInfo* item, CreatureInfo* creature, int damage)
	{
		if (!(creature->Flags) && item->TestBits(JointBitType::Touch, ShivaAttackRightJoints))
		{
			CreatureEffect(item, &ShivaBiteRight, DoBloodSplat);
			DoDamage(creature->Enemy, damage);
			SoundEffect(SFX_TR2_CRUNCH2, &item->Pose);
			creature->Flags = 1;
		}

		if (!(creature->Flags) && item->TestBits(JointBitType::Touch, ShivaAttackLeftJoints))
		{
			CreatureEffect(item, &ShivaBiteLeft, DoBloodSplat);
			DoDamage(creature->Enemy, damage);
			SoundEffect(SFX_TR2_CRUNCH2, &item->Pose);
			creature->Flags = 1;
		}
	}

	void InitialiseShiva(short itemNumber)
	{
		ClearItem(itemNumber);

		auto* item = &g_Level.Items[itemNumber];
		item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + SHIVA_ANIM_INACTIVE;
		auto* anim = &g_Level.Anims[item->Animation.AnimNumber];

		item->Animation.FrameNumber = anim->frameBase;
		item->Animation.ActiveState = anim->ActiveState;
	}

	void ShivaControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto* item = &g_Level.Items[itemNumber];
		auto* shiva = GetCreatureInfo(item);

		auto pos = Vector3Int(0, 0, 256);
		bool laraAlive = LaraItem->HitPoints > 0;

		Vector3Shrt extraHeadRot;
		Vector3Shrt extraTorsoRot;
		short angle = 0;
		short tilt = 0;

		if (item->HitPoints <= 0)
		{
			if (item->Animation.ActiveState != SHIVA_STATE_DEATH)
			{
				item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + SHIVA_ANIM_DEATH;
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				item->Animation.ActiveState = SHIVA_STATE_DEATH;
			}
		}
		else
		{
			AI_INFO AI;
			CreatureAIInfo(item, &AI);

			GetCreatureMood(item, &AI, VIOLENT);
			CreatureMood(item, &AI, VIOLENT);

			if (shiva->Mood == MoodType::Escape)
			{
				shiva->Target.x = LaraItem->Pose.Position.x;
				shiva->Target.z = LaraItem->Pose.Position.z;
			}

			angle = CreatureTurn(item, shiva->MaxTurn);

			if (item->Animation.ActiveState != SHIVA_STATE_INACTIVE)
				item->MeshBits = 0xFFFFFFFF;

			int effectMesh = 0;

			switch (item->Animation.ActiveState)
			{
			case SHIVA_STATE_INACTIVE:
				shiva->MaxTurn = 0;

				if (!shiva->Flags)
				{
					if (item->MeshBits == 0)
						effectMesh = 0;

					item->MeshBits = (item->MeshBits * 2) + 1;
					shiva->Flags = 1;

					GetJointAbsPosition(item, &pos, effectMesh++);
					TriggerExplosionSparks(pos.x, pos.y, pos.z, 2, 0, 0, item->RoomNumber);
					TriggerShivaSmoke(pos.x, pos.y, pos.z, 1);

				}
				else
					shiva->Flags--;

				if (item->MeshBits == 0x7FFFFFFF)
				{
					item->Animation.TargetState = SHIVA_STATE_IDLE;
					shiva->Flags = -45;
					effectMesh = 0;
				}

				break;

			case SHIVA_STATE_IDLE:
				shiva->MaxTurn = 0;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (shiva->Flags < 0)
				{
					shiva->Flags++;
					TriggerShivaSmoke(item->Pose.Position.x + (GetRandomControl() & 0x5FF) - 0x300, pos.y - (GetRandomControl() & 0x5FF), item->Pose.Position.z + (GetRandomControl() & 0x5FF) - 0x300, 1);
					break;
				}

				if (shiva->Flags == 1)
					shiva->Flags = 0;

				if (shiva->Mood == MoodType::Escape)
				{
					int x = item->Pose.Position.x + SECTOR(1) * phd_sin(item->Pose.Orientation.y + ANGLE(180.0f));
					int z = item->Pose.Position.z + SECTOR(1) * phd_cos(item->Pose.Orientation.y + ANGLE(180.0f));
					auto box = GetCollision(x, item->Pose.Position.y, z, item->RoomNumber).BottomBlock->Box;

					if (box != NO_BOX && !(g_Level.Boxes[box].flags & BLOCKABLE) && !shiva->Flags)
						item->Animation.TargetState = SHIVA_STATE_WALK_BACK;
					else
						item->Animation.TargetState = SHIVA_STATE_GUARD_IDLE;
				}
				else if (shiva->Mood == MoodType::Bored)
				{
					int random = GetRandomControl();
					if (random < 0x400)
						item->Animation.TargetState = SHIVA_STATE_WALK_FORWARD;
				}
				else if (AI.bite && AI.distance < pow(SECTOR(1.25f), 2))
				{
					item->Animation.TargetState = SHIVA_STATE_GRAB_ATTACK;
					shiva->Flags = 0;
				}
				else if (AI.bite && AI.distance < pow(SECTOR(4) / 3, 2))
				{
					item->Animation.TargetState = SHIVA_STATE_DOWNWARD_ATTACK;
					shiva->Flags = 0;
				}
				else if (item->HitStatus && AI.ahead)
				{
					item->Animation.TargetState = SHIVA_STATE_GUARD_IDLE;
					shiva->Flags = 4;
				}
				else
					item->Animation.TargetState = SHIVA_STATE_WALK_FORWARD;

				break;

			case SHIVA_STATE_GUARD_IDLE:
				shiva->MaxTurn = 0;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (item->HitStatus || shiva->Mood == MoodType::Escape)
					shiva->Flags = 4;

				if (AI.bite && AI.distance < pow(SECTOR(4) / 3, 2) ||
					(item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase &&
						!shiva->Flags) ||
					!AI.ahead)
				{
					item->Animation.TargetState = SHIVA_STATE_IDLE;
					shiva->Flags = 0;
				}
				else if (shiva->Flags)
					item->Animation.TargetState = SHIVA_STATE_GUARD_IDLE;


				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase &&
					shiva->Flags > 1)
				{
					shiva->Flags -= 2;
				}

				break;

			case SHIVA_STATE_WALK_FORWARD:
				shiva->MaxTurn = SHIVA_WALK_TURN_ANGLE;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (shiva->Mood == MoodType::Escape)
					item->Animation.TargetState = SHIVA_STATE_IDLE;
				else if (shiva->Mood == MoodType::Bored)
					item->Animation.TargetState = SHIVA_STATE_IDLE;
				else if (AI.bite && AI.distance < pow(SECTOR(4) / 3, 2))
				{
					item->Animation.TargetState = SHIVA_STATE_IDLE;
					shiva->Flags = 0;
				}
				else if (item->HitStatus)
				{
					item->Animation.TargetState = SHIVA_STATE_WALK_FORWARD_GUARDING;
					shiva->Flags = 4;
				}

				break;

			case SHIVA_STATE_WALK_FORWARD_GUARDING:
				shiva->MaxTurn = SHIVA_WALK_TURN_ANGLE;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (item->HitStatus)
					shiva->Flags = 4;

				if (AI.bite && AI.distance < pow(SECTOR(1.25f), 2) ||
					(item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase &&
						!shiva->Flags))
				{
					item->Animation.TargetState = SHIVA_STATE_WALK_FORWARD;
					shiva->Flags = 0;
				}
				else if (shiva->Flags)
					item->Animation.TargetState = SHIVA_STATE_WALK_FORWARD_GUARDING;

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
					shiva->Flags = 0;

				break;

			case SHIVA_STATE_WALK_BACK:
				shiva->MaxTurn = SHIVA_WALK_TURN_ANGLE;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (AI.ahead && AI.distance < pow(SECTOR(4) / 3, 2) ||
					(item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase &&
						!shiva->Flags))
				{
					item->Animation.TargetState = SHIVA_STATE_IDLE;
				}
				else if (item->HitStatus)
				{
					item->Animation.TargetState = SHIVA_STATE_IDLE;
					shiva->Flags = 4;
				}

				break;

			case SHIVA_STATE_GRAB_ATTACK:
				shiva->MaxTurn = SHIVA_ATTACK_TURN_ANGLE;

				if (AI.ahead)
				{
					extraHeadRot.y = AI.angle;
					extraTorsoRot = Vector3Shrt(AI.xAngle, AI.angle, 0);
				}

				ShivaDamage(item, shiva, SHIVA_GRAB_ATTACK_DAMAGE);
				break;

			case SHIVA_STATE_DOWNWARD_ATTACK:
				shiva->MaxTurn = SHIVA_ATTACK_TURN_ANGLE;
				extraHeadRot.y = AI.angle;
				extraTorsoRot.y = AI.angle;

				if (AI.xAngle > 0)
					extraTorsoRot.x = AI.xAngle;

				ShivaDamage(item, shiva, SHIVA_DOWNWARD_ATTACK_DAMAGE);
				break;

			case SHIVA_STATE_KILL:
				shiva->MaxTurn = 0;
				extraHeadRot = Vector3Shrt();
				extraTorsoRot = Vector3Shrt();

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + SHIVA_ANIM_WALK_FORWARD_TO_GUARDED_LEFT_1 ||
					item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + SHIVA_ANIM_WALK_BACK_RIGHT ||
					item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 33) // TODO: Huh? No such anim index exists for the Shiva.
				{
					CreatureEffect(item, &ShivaBiteRight, DoBloodSplat);
					CreatureEffect(item, &ShivaBiteLeft, DoBloodSplat);
				}

				break;
			}
		}

		// Dispatch kill animation
		if (laraAlive && LaraItem->HitPoints <= 0)
		{
			item->Animation.TargetState = SHIVA_STATE_KILL;

			if (LaraItem->RoomNumber != item->RoomNumber)
				ItemNewRoom(Lara.ItemNumber, item->RoomNumber);

			LaraItem->Pose.Position = item->Pose.Position;
			LaraItem->Pose.Orientation = Vector3Shrt(0, item->Pose.Orientation.y, 0);
			LaraItem->Animation.IsAirborne = false;

			LaraItem->Animation.AnimNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex + LARA_ANIM_SHIVA_DEATH;
			LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
			LaraItem->Animation.ActiveState = LS_DEATH;
			LaraItem->Animation.TargetState = LS_DEATH;

			LaraItem->HitPoints = NOT_TARGETABLE;
			Lara.Air = -1;
			Lara.Control.HandStatus = HandStatus::Special;
			Lara.Control.Weapon.GunType = LaraWeaponType::None;
			Camera.targetDistance = SECTOR(4);
			Camera.flags = CF_FOLLOW_CENTER;
			return;
		}

		CreatureTilt(item, tilt);

		extraHeadRot.y -= extraTorsoRot.y;

		CreatureJoint(item, 0, extraTorsoRot.y);
		CreatureJoint(item, 1, extraTorsoRot.x);
		CreatureJoint(item, 2, extraHeadRot.y);
		CreatureJoint(item, 3, extraHeadRot.x);
		CreatureAnimation(itemNumber, angle, tilt);
	}
}
