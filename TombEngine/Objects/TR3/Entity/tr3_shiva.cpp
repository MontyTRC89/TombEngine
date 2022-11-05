#include "framework.h"
#include "Objects/TR3/Entity/tr3_shiva.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/control/box.h"
#include "Game/effects/effects.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/misc.h"
#include "Math/Math.h"
#include "Renderer/Renderer11Enums.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Math;

namespace TEN::Entities::Creatures::TR3
{
	constexpr auto SHIVA_GRAB_ATTACK_DAMAGE		= 150;
	constexpr auto SHIVA_DOWNWARD_ATTACK_DAMAGE = 180;

	constexpr auto LARA_ANIM_SHIVA_DEATH = 7;

	const auto SHIVA_WALK_TURN_RATE_MAX	  = ANGLE(4.0f);
	const auto SHIVA_ATTACK_TURN_RATE_MAX = ANGLE(4.0f);

	const auto ShivaBiteLeft  = BiteInfo(Vector3(0.0f, 0.0f, 920.0f), 13);
	const auto ShivaBiteRight = BiteInfo(Vector3(0.0f, 0.0f, 920.0f), 22);
	const auto ShivaAttackLeftJoints  = std::vector<unsigned int>{ 10, 13 };
	const auto ShivaAttackRightJoints = std::vector<unsigned int>{ 22, 25 };

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

	// TODO
	enum ShivaFlags
	{

	};

	void TriggerShivaSmoke(long x, long y, long z, long uw)
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

	void ShivaDamage(ItemInfo* item, CreatureInfo* creature, int damage)
	{
		if (!(creature->Flags) && item->TouchBits.Test(ShivaAttackRightJoints))
		{
			DoDamage(creature->Enemy, damage);
			CreatureEffect(item, ShivaBiteRight, DoBloodSplat);
			SoundEffect(SFX_TR2_CRUNCH2, &item->Pose);
			creature->Flags = 1;
		}

		if (!(creature->Flags) && item->TouchBits.Test(ShivaAttackLeftJoints))
		{
			DoDamage(creature->Enemy, damage);
			CreatureEffect(item, ShivaBiteLeft, DoBloodSplat);
			SoundEffect(SFX_TR2_CRUNCH2, &item->Pose);
			creature->Flags = 1;
		}
	}

	void InitialiseShiva(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];

		ClearItem(itemNumber);

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
		auto* creature = GetCreatureInfo(item);

		auto pos = Vector3i(0, 0, CLICK(1));
		bool isLaraAlive = LaraItem->HitPoints > 0;

		short angle = 0;
		short tilt = 0;
		EulerAngles extraHeadRot = EulerAngles::Zero;
		EulerAngles extraTorsoRot = EulerAngles::Zero;

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

			GetCreatureMood(item, &AI, true);
			CreatureMood(item, &AI, true);

			if (creature->Mood == MoodType::Escape)
			{
				creature->Target.x = LaraItem->Pose.Position.x;
				creature->Target.z = LaraItem->Pose.Position.z;
			}

			angle = CreatureTurn(item, creature->MaxTurn);

			if (item->Animation.ActiveState != SHIVA_STATE_INACTIVE)
				item->MeshBits = ALL_JOINT_BITS;

			int effectMesh = 0;
			auto pos = Vector3i(0, 0, CLICK(1));

			switch (item->Animation.ActiveState)
			{
			case SHIVA_STATE_INACTIVE:
				creature->MaxTurn = 0;

				if (!creature->Flags)
				{
					if (!item->MeshBits.TestAny())
						effectMesh = 0;

					item->MeshBits = (item->MeshBits.ToPackedBits() * 2) + 1;
					creature->Flags = 1;

					pos = GetJointPosition(item, effectMesh++, pos);
					TriggerExplosionSparks(pos.x, pos.y, pos.z, 2, 0, 0, item->RoomNumber);
					TriggerShivaSmoke(pos.x, pos.y, pos.z, 1);

				}
				else
					creature->Flags--;

				if (item->MeshBits == 0x7FFFFFFF)
				{
					item->Animation.TargetState = SHIVA_STATE_IDLE;
					creature->Flags = -45;
					effectMesh = 0;
				}

				break;

			case SHIVA_STATE_IDLE:
				creature->MaxTurn = 0;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (creature->Flags < 0)
				{
					creature->Flags++;
					TriggerShivaSmoke(item->Pose.Position.x + (GetRandomControl() & 0x5FF) - 0x300, pos.y - (GetRandomControl() & 0x5FF), item->Pose.Position.z + (GetRandomControl() & 0x5FF) - 0x300, 1);
					break;
				}

				if (creature->Flags == 1)
					creature->Flags = 0;

				if (creature->Mood == MoodType::Escape)
				{
					int x = item->Pose.Position.x + SECTOR(1) * phd_sin(item->Pose.Orientation.y + ANGLE(180.0f));
					int z = item->Pose.Position.z + SECTOR(1) * phd_cos(item->Pose.Orientation.y + ANGLE(180.0f));
					auto box = GetCollision(x, item->Pose.Position.y, z, item->RoomNumber).BottomBlock->Box;

					if (box != NO_BOX && !(g_Level.Boxes[box].flags & BLOCKABLE) && !creature->Flags)
						item->Animation.TargetState = SHIVA_STATE_WALK_BACK;
					else
						item->Animation.TargetState = SHIVA_STATE_GUARD_IDLE;
				}
				else if (creature->Mood == MoodType::Bored)
				{
					if (Random::TestProbability(0.0325f))
						item->Animation.TargetState = SHIVA_STATE_WALK_FORWARD;
				}
				else if (AI.bite && AI.distance < pow(SECTOR(1.25f), 2))
				{
					item->Animation.TargetState = SHIVA_STATE_GRAB_ATTACK;
					creature->Flags = 0;
				}
				else if (AI.bite && AI.distance < pow(SECTOR(4) / 3, 2))
				{
					item->Animation.TargetState = SHIVA_STATE_DOWNWARD_ATTACK;
					creature->Flags = 0;
				}
				else if (item->HitStatus && AI.ahead)
				{
					item->Animation.TargetState = SHIVA_STATE_GUARD_IDLE;
					creature->Flags = 4;
				}
				else
					item->Animation.TargetState = SHIVA_STATE_WALK_FORWARD;

				break;

			case SHIVA_STATE_GUARD_IDLE:
				creature->MaxTurn = 0;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (item->HitStatus || creature->Mood == MoodType::Escape)
					creature->Flags = 4;

				if (AI.bite && AI.distance < pow(SECTOR(4) / 3, 2) ||
					(item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase &&
						!creature->Flags) ||
					!AI.ahead)
				{
					item->Animation.TargetState = SHIVA_STATE_IDLE;
					creature->Flags = 0;
				}
				else if (creature->Flags)
					item->Animation.TargetState = SHIVA_STATE_GUARD_IDLE;


				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase &&
					creature->Flags > 1)
				{
					creature->Flags -= 2;
				}

				break;

			case SHIVA_STATE_WALK_FORWARD:
				creature->MaxTurn = SHIVA_WALK_TURN_RATE_MAX;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = SHIVA_STATE_IDLE;
				else if (creature->Mood == MoodType::Bored)
					item->Animation.TargetState = SHIVA_STATE_IDLE;
				else if (AI.bite && AI.distance < pow(SECTOR(4) / 3, 2))
				{
					item->Animation.TargetState = SHIVA_STATE_IDLE;
					creature->Flags = 0;
				}
				else if (item->HitStatus)
				{
					item->Animation.TargetState = SHIVA_STATE_WALK_FORWARD_GUARDING;
					creature->Flags = 4;
				}

				break;

			case SHIVA_STATE_WALK_FORWARD_GUARDING:
				creature->MaxTurn = SHIVA_WALK_TURN_RATE_MAX;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (item->HitStatus)
					creature->Flags = 4;

				if (AI.bite && AI.distance < pow(SECTOR(1.25f), 2) ||
					(item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase &&
						!creature->Flags))
				{
					item->Animation.TargetState = SHIVA_STATE_WALK_FORWARD;
					creature->Flags = 0;
				}
				else if (creature->Flags)
					item->Animation.TargetState = SHIVA_STATE_WALK_FORWARD_GUARDING;

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
					creature->Flags = 0;

				break;

			case SHIVA_STATE_WALK_BACK:
				creature->MaxTurn = SHIVA_WALK_TURN_RATE_MAX;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (AI.ahead && AI.distance < pow(SECTOR(4) / 3, 2) ||
					(item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase &&
						!creature->Flags))
				{
					item->Animation.TargetState = SHIVA_STATE_IDLE;
				}
				else if (item->HitStatus)
				{
					item->Animation.TargetState = SHIVA_STATE_IDLE;
					creature->Flags = 4;
				}

				break;

			case SHIVA_STATE_GRAB_ATTACK:
				creature->MaxTurn = SHIVA_ATTACK_TURN_RATE_MAX;

				if (AI.ahead)
				{
					extraHeadRot.y = AI.angle;
					extraTorsoRot = EulerAngles(AI.xAngle, AI.angle, 0);
				}

				ShivaDamage(item, creature, SHIVA_GRAB_ATTACK_DAMAGE);
				break;

			case SHIVA_STATE_DOWNWARD_ATTACK:
				creature->MaxTurn = SHIVA_ATTACK_TURN_RATE_MAX;
				extraHeadRot.y = AI.angle;
				extraTorsoRot.y = AI.angle;

				if (AI.xAngle > 0)
					extraTorsoRot.x = AI.xAngle;

				ShivaDamage(item, creature, SHIVA_DOWNWARD_ATTACK_DAMAGE);
				break;

			case SHIVA_STATE_KILL:
				creature->MaxTurn = 0;
				extraHeadRot = EulerAngles::Zero;
				extraTorsoRot = EulerAngles::Zero;

				if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + SHIVA_ANIM_WALK_FORWARD_TO_GUARDED_LEFT_1 ||
					item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + SHIVA_ANIM_WALK_BACK_RIGHT ||
					item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase + 33) // TODO: Huh? No such anim index exists for the Shiva.
				{
					CreatureEffect(item, ShivaBiteRight, DoBloodSplat);
					CreatureEffect(item, ShivaBiteLeft, DoBloodSplat);
				}

				break;
			}
		}

		// Dispatch kill animation.
		if (isLaraAlive && LaraItem->HitPoints <= 0)
		{
			item->Animation.TargetState = SHIVA_STATE_KILL;

			if (LaraItem->RoomNumber != item->RoomNumber)
				ItemNewRoom(Lara.ItemNumber, item->RoomNumber);

			LaraItem->Animation.AnimNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex + LARA_ANIM_SHIVA_DEATH;
			LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
			LaraItem->Animation.ActiveState = LS_DEATH;
			LaraItem->Animation.TargetState = LS_DEATH;
			LaraItem->Animation.IsAirborne = false;
			LaraItem->Pose.Position = item->Pose.Position;
			LaraItem->Pose.Orientation = EulerAngles(0, item->Pose.Orientation.y, 0);
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
