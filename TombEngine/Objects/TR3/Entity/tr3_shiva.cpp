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
#include "Lara/lara_helpers.h"
#include "Renderer/Renderer11Enums.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Math/Random.h"
#include "Specific/setup.h"

using namespace TEN::Math::Random;
using std::vector;

namespace TEN::Entities::Creatures::TR3
{
	constexpr auto SHIVA_WALK_TURN_RATE_MAX = ANGLE(4.0f);
	constexpr auto SHIVA_ATTACK_TURN_RATE_MAX = ANGLE(4.0f);
	constexpr auto SHIVA_DOWNWARD_ATTACK_RANGE = SQUARE(SECTOR(1));
	constexpr auto SHIVA_GRAB_ATTACK_RANGE = SQUARE(SECTOR(1.25f));
	constexpr auto SHIVA_GRAB_ATTACK_DAMAGE = 150;
	constexpr auto SHIVA_DOWNWARD_ATTACK_DAMAGE = 180;
	constexpr auto SHIVA_MESH_SWITCH_TIME = 50000;
	constexpr auto LARA_ANIM_SHIVA_DEATH = 7; // TODO: move it to LaraExtraAnims enum.

	const auto ShivaBiteLeft  = BiteInfo(Vector3(0.0f, 0.0f, 920.0f), 13);
	const auto ShivaBiteRight = BiteInfo(Vector3(0.0f, 0.0f, 920.0f), 22);
	const auto ShivaEffectMeshPosition = Vector3i(0, 0, 0);
	const vector<unsigned int> ShivaAttackLeftJoints	 = { 10, 13 };
	const vector<unsigned int> ShivaAttackRightJoints = { 22, 25 };

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

	void SwitchShivaMeshToStone(ItemInfo& item, int meshID)
	{
		item.Model.MeshIndex[meshID] = Objects[ID_SHIVA_STATUE].meshIndex + meshID;
	}

	void SwitchShivaMeshToNormal(ItemInfo& item, int meshID)
	{
		item.Model.MeshIndex[meshID] = Objects[ID_SHIVA].meshIndex + meshID;
	}

	void InitialiseShiva(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		InitialiseCreature(itemNumber);
		SetAnimation(item, SHIVA_ANIM_INACTIVE);
		item->SetFlags(0, 0); // mesh id, used for swapping mesh.
		if (item->TestFlags(1, 0))
		{
			const auto& obj = Objects[item->ObjectNumber];
			for (int meshID = 0; meshID < obj.nmeshes; meshID++)
				SwitchShivaMeshToStone(*item, meshID);
			item->SetFlags(2, 1); // continue transition until finished.
		}
	}

	void TriggerShivaSmoke(int x, int y, int z, int roomNumber)
	{
		auto* sptr = GetFreeParticle();
		bool uw = TestEnvironment(ENV_FLAG_WATER, x, y, z, roomNumber);
		if (uw)
		{
			sptr->sR = 144;
			sptr->sG = 144;
			sptr->sB = 144;
			sptr->dR = 64;
			sptr->dG = 64;
			sptr->dB = 64;
		}
		else
		{
			sptr->sR = 0;
			sptr->sG = 0;
			sptr->sB = 0;
			sptr->dR = 192;
			sptr->dG = 192;
			sptr->dB = 208;
		}

		sptr->colFadeSpeed = 8;
		sptr->fadeToBlack = 64;
		sptr->sLife = sptr->life = (GetRandomControl() & 31) + 96;
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

		int size = (GetRandomControl() & 31) + 128;
		sptr->size = sptr->sSize = size / 4;
		sptr->dSize = size;
		size += (GetRandomControl() & 31) + 32;
		sptr->size = sptr->sSize = size / 8;
		sptr->dSize = size;
		sptr->on = true;
	}

	void ShivaDamage(ItemInfo* item, CreatureInfo* creature, int damage)
	{
		if (!creature->Flags && item->TouchBits.Test(ShivaAttackRightJoints))
		{
			DoDamage(creature->Enemy, damage);
			CreatureEffect(item, ShivaBiteRight, DoBloodSplat);
			SoundEffect(SFX_TR2_CRUNCH2, &item->Pose);
			creature->Flags = 1;
		}

		if (!creature->Flags && item->TouchBits.Test(ShivaAttackLeftJoints))
		{
			DoDamage(creature->Enemy, damage);
			CreatureEffect(item, ShivaBiteLeft, DoBloodSplat);
			SoundEffect(SFX_TR2_CRUNCH2, &item->Pose);
			creature->Flags = 1;
		}
	}

	// True = dead for sure.
	bool DoShivaSwapMesh(ItemInfo& item, bool isDeath)
	{
		auto& creature = *GetCreatureInfo(&item);

		if (creature.Flags == 0 && (item.TestFlags(2, 1) || item.TestFlags(2, 2)))
		{
			creature.Flags = 1;

			const auto& obj = Objects[item.ObjectNumber];
			if (isDeath && item.ItemFlags[0] < 0)
				item.SetFlags(2, 0);
			else if (!isDeath && item.ItemFlags[0] >= obj.nmeshes)
				item.SetFlags(2, 0);
			else
			{
				auto pos = GetJointPosition(&item, item.ItemFlags[0], ShivaEffectMeshPosition);
				TriggerShivaSmoke(pos.x, pos.y, pos.z, item.RoomNumber);
				if (isDeath)
				{
					SwitchShivaMeshToStone(item, item.ItemFlags[0]);
					item.ItemFlags[0]--;
				}
				else
				{
					SwitchShivaMeshToNormal(item, item.ItemFlags[0]);
					item.ItemFlags[0]++;
				}
			}
		}
		else
		{
			creature.Flags--;
		}

		if (item.TestFlags(2, 0) && !isDeath)
		{
			item.Animation.TargetState = SHIVA_STATE_IDLE;
			creature.Flags = -45;
			item.SetFlags(1, 0);
			item.SetFlags(1, 1); // is alive (for savegame).
		}
		else if (item.TestFlags(2, 0) && isDeath)
		{
			item.SetFlags(1, 0);
			item.SetFlags(1, 2); // no more alive.
			return true;
		}

		return false;
	}

	void ShivaControl(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;
		auto* item = &g_Level.Items[itemNumber];
		auto* creature = GetCreatureInfo(item);
		short angle = 0;
		short tilt = 0;
		auto extraHeadRot = EulerAngles::Zero;
		auto extraTorsoRot = EulerAngles::Zero;

		if (item->HitPoints <= 0)
		{
			const auto& obj = Objects[item->ObjectNumber];
			if (item->Animation.ActiveState != SHIVA_STATE_DEATH)
			{
				SetAnimation(item, SHIVA_ANIM_DEATH);
				item->ItemFlags[0] = obj.nmeshes - 1;
				item->SetFlags(2, 2); // redo swap to stone.
			}

			int frameEnd = g_Level.Anims[obj.animIndex + SHIVA_ANIM_DEATH].frameEnd - 1;
			if (item->Animation.FrameNumber >= frameEnd)
			{
				item->Animation.FrameNumber = frameEnd; // block frame until mesh is switched.
				if (DoShivaSwapMesh(*item, true))
					CreatureDie(itemNumber, false);
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
				creature->Target.x = creature->Enemy->Pose.Position.x;
				creature->Target.z = creature->Enemy->Pose.Position.z;
			}

			bool isLaraAlive = creature->Enemy->HitPoints > 0;
			angle = CreatureTurn(item, creature->MaxTurn);

			switch (item->Animation.ActiveState)
			{
			case SHIVA_STATE_INACTIVE:
				creature->MaxTurn = 0;
				DoShivaSwapMesh(*item, false);
				break;

			case SHIVA_STATE_IDLE:
				creature->MaxTurn = 0;
				if (creature->Flags < 0)
				{
					creature->Flags++;
					TriggerShivaSmoke(item->Pose.Position.x + (GetRandomControl() & 0x5FF) - 0x300, (item->Pose.Position.y - CLICK(1)) - (GetRandomControl() & 0x5FF), item->Pose.Position.z + (GetRandomControl() & 0x5FF) - 0x300, item->RoomNumber);
					return;
				}
				creature->Flags = 0;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

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
					if (TestProbability(0.0325f))
						item->Animation.TargetState = SHIVA_STATE_WALK_FORWARD;
				}
				else if (AI.bite && AI.distance < SHIVA_GRAB_ATTACK_RANGE)
				{
					item->Animation.TargetState = SHIVA_STATE_GRAB_ATTACK;
					creature->Flags = 0;
				}
				else if (AI.bite && AI.distance < SHIVA_DOWNWARD_ATTACK_RANGE)
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

				if (AI.bite && AI.distance < SHIVA_DOWNWARD_ATTACK_RANGE ||
				   (item->Animation.FrameNumber == GetFrameNumber(item, 0) && !creature->Flags) ||
				   !AI.ahead)
				{
					item->Animation.TargetState = SHIVA_STATE_IDLE;
					creature->Flags = 0;
				}
				else if (creature->Flags)
					item->Animation.TargetState = SHIVA_STATE_GUARD_IDLE;

				if (item->Animation.FrameNumber == GetFrameNumber(item, 0) && creature->Flags > 1)
					creature->Flags -= 2;

				break;

			case SHIVA_STATE_WALK_FORWARD:
				creature->MaxTurn = SHIVA_WALK_TURN_RATE_MAX;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if (creature->Mood == MoodType::Escape)
					item->Animation.TargetState = SHIVA_STATE_IDLE;
				else if (creature->Mood == MoodType::Bored)
					item->Animation.TargetState = SHIVA_STATE_IDLE;
				else if (AI.bite && AI.distance < SHIVA_DOWNWARD_ATTACK_RANGE)
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

				if ((AI.bite && AI.distance < SHIVA_DOWNWARD_ATTACK_RANGE) ||
				    (item->Animation.FrameNumber == GetFrameNumber(item, 0) && !creature->Flags))
				{
					item->Animation.TargetState = SHIVA_STATE_WALK_FORWARD;
					creature->Flags = 0;
				}
				else if (creature->Flags)
					item->Animation.TargetState = SHIVA_STATE_WALK_FORWARD_GUARDING;

				if (item->Animation.FrameNumber == GetFrameNumber(item, 0))
					creature->Flags = 0;

				break;

			case SHIVA_STATE_WALK_BACK:
				creature->MaxTurn = SHIVA_WALK_TURN_RATE_MAX;

				if (AI.ahead)
					extraHeadRot.y = AI.angle;

				if ((AI.ahead && AI.distance < SHIVA_DOWNWARD_ATTACK_RANGE) ||
				    (item->Animation.FrameNumber == GetFrameNumber(item, 0) &&
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

				if (item->Animation.FrameNumber == GetFrameNumber(item, 10) ||
					item->Animation.FrameNumber == GetFrameNumber(item, 21) ||
					item->Animation.FrameNumber == GetFrameNumber(item, 33))
				{
					CreatureEffect(item, ShivaBiteRight, DoBloodSplat);
					CreatureEffect(item, ShivaBiteLeft, DoBloodSplat);
				}

				break;
			}

			// Dispatch kill animation.
			if (isLaraAlive && LaraItem->HitPoints <= 0)
			{
				CreatureKill(item, SHIVA_ANIM_KILL, LARA_ANIM_SHIVA_DEATH, SHIVA_STATE_KILL, LS_DEATH);
				return;
			}
		}

		CreatureTilt(item, tilt);
		extraHeadRot.y -= extraTorsoRot.y;

		CreatureJoint(item, 0, extraTorsoRot.y);
		CreatureJoint(item, 1, extraTorsoRot.x);
		CreatureJoint(item, 2, extraHeadRot.y);
		CreatureJoint(item, 3, extraHeadRot.x);
		CreatureAnimation(itemNumber, angle, tilt);
	}

	void ShivaHit(ItemInfo& target, ItemInfo& source, std::optional<GameVector> pos, int damage, int grenade, int jointIndex)
	{
		const auto& player = *GetLaraInfo(&source);
		const auto& object = Objects[target.ObjectNumber];

		// If shiva is guarded, then don't damage him, do the ricochet instead.
		if ((target.Animation.ActiveState == SHIVA_STATE_WALK_FORWARD_GUARDING ||
			target.Animation.ActiveState == SHIVA_STATE_GUARD_IDLE) && pos.has_value())
		{
			SoundEffect(SFX_TR4_BADDY_SWORD_RICOCHET, &target.Pose);
			TriggerRicochetSpark(*pos, source.Pose.Orientation.y, 3, 0);
		}
		// Else do the basic hit effect (blood + damage)
		else if (object.hitEffect == HitEffect::Blood && pos.has_value())
		{
			DoBloodSplat(pos->x, pos->y, pos->z, 10, source.Pose.Orientation.y, pos->RoomNumber);
			DoItemHit(&target, damage, grenade);
		}
	}
}
