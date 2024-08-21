#include "framework.h"
#include "Objects/TR4/Entity/EnemyJeep.h"

#include "Game/animation.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/Point.h"
#include "Game/control/box.h"
#include "Game/control/lot.h"
#include "Game/control/trigger.h"
#include "Game/effects/effects.h"
#include "Game/effects/smoke.h"
#include "Game/effects/tomb4fx.h"
#include "Game/itemdata/creature_info.h"
#include "Game/items.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/misc.h"
#include "Game/people.h"
#include "Game/Setup.h"
#include "Math/Math.h"
#include "Renderer/Renderer.h"
#include "Sound/sound.h"
#include "Specific/level.h"

using namespace TEN::Renderer;
using namespace TEN::Effects::Smoke;
using namespace TEN::Collision::Point;
using namespace TEN::Math;

/// item.ItemFlags[1] = AI_X2 behaviour
/// item.ItemFlags[2] = Wheel rotation
/// item.ItemFlags[3] = Grenade cooldown
/// item.ItemFlags[4] = Behaviour when idle
/// item.ItemFlags[5] = Wait radius

namespace TEN::Entities::TR4
{
	enum EnemyJeepAnim
	{
		ENEMY_JEEP_ANIM_MOVE_START = 0,
		ENEMY_JEEP_ANIM_MOVE_STOP = 1,
		ENEMY_JEEP_ANIM_MOVING = 2,
		ENEMY_JEEP_ANIM_TURN_LEFT_START = 3,
		ENEMY_JEEP_ANIM_TURN_LEFT = 4,
		ENEMY_JEEP_ANIM_TURN_LEFT_END = 5,
		ENEMY_JEEP_ANIM_FALL_END = 6,
		ENEMY_JEEP_ANIM_FALL_2_STEPS = 7,
		ENEMY_JEEP_ANIM_JUMP_2_STEP_PIT = 8,
		ENEMY_JEEP_ANIM_IDLE = 9,
		ENEMY_JEEP_ANIM_TURN_RIGHT_START = 10,
		ENEMY_JEEP_ANIM_TURN_RIGHT = 11,
		ENEMY_JEEP_ANIM_TURN_RIGHT_END = 12
	};

	enum EnemyJeepState
	{
		ENEMY_JEEP_STATE_IDLE = 0,
		ENEMY_JEEP_STATE_MOVE = 1,
		ENEMY_JEEP_STATE_STOP = 2,
		ENEMY_JEEP_STATE_TURN_LEFT = 3,
		ENEMY_JEEP_STATE_TURN_RIGHT = 4,
		ENEMY_JEEP_STATE_DROP_LAND = 5,

		// New state added to allow customization through state.

		ENEMY_JEEP_STATE_DROP = 6,
		ENEMY_JEEP_STATE_JUMP_PIT = 7,
		ENEMY_JEEP_STATE_CUSTOM_DROP = 8,
		ENEMY_JEEP_STATE_CUSTOM_JUMP_PIT = 9
	};

	enum EnemyJeepOcb
	{
		EJ_NO_PLAYER_VEHICLE_REQUIRED = 1 // Makes enemy jeep start directly instead of waiting for player to enter a vehicle.
	};

	enum EnemyJeepX2Ocb
	{
		X2_DROP_GRENADE = 1,
		X2_DROP = 2,
		X2_JUMP_PIT = 3,
		// Need ocb 4 + block distance
		// Example: 4 + 1024 = 4 block distance + wait behaviour.
		X2_WAIT_UNTIL_LARA_NEAR = 4,
		X2_DISAPPEAR = 5,
		X2_ACTIVATE_HEAVY_TRIGGER = 6,
		X2_CUSTOM_DROP = 7, // Another drop step for customization.
		X2_CUSTOM_JUMP_PIT = 8, // Another jump steps for customization.
	};

	constexpr auto ENEMY_JEEP_GRENADE_VELOCITY = 32.0f;
	constexpr auto ENEMY_JEEP_GRENADE_TIMER = 150;
	constexpr auto ENEMY_JEEP_CENTER_MESH = 11;
	constexpr auto ENEMY_JEEP_RIGHT_LIGHT_MESHBITS = 15;
	constexpr auto ENEMY_JEEP_LEFT_LIGHT_MESHBITS = 17;
	constexpr auto ENEMY_JEEP_GRENADE_COOLDOWN_TIME = 15;
	constexpr auto ENEMY_JEEP_PLAYER_IS_NEAR = BLOCK(6.0f);
	constexpr auto ENEMY_JEEP_NEAR_X1_NODE_DISTANCE = BLOCK(1.0f);
	constexpr auto ENEMY_JEEP_NEAR_X2_NODE_DISTANCE = BLOCK(0.3f);
	constexpr auto ENEMY_JEEP_PITCH_MAX = 120.0f;
	constexpr auto ENEMY_JEEP_PITCH_WHEEL_SPEED_MULTIPLIER = 12.0f;
	constexpr auto ENEMY_JEEP_WHEEL_LEFTRIGHT_TURN_MINIMUM = ANGLE(12.0f);

	const auto EnemyJeepGrenadeBite = CreatureBiteInfo(Vector3(0.0f, -640.0f, -768.0f), ENEMY_JEEP_CENTER_MESH);
	const auto EnemyJeepRightLightBite = CreatureBiteInfo(Vector3(200.0f, -144.0f, -768.0f), ENEMY_JEEP_CENTER_MESH);
	const auto EnemyJeepLeftLightBite = CreatureBiteInfo(Vector3(-200.0f, -144.0f, -768.0f), ENEMY_JEEP_CENTER_MESH);

	// Enable/Disable the light mesh that are on the back of the jeep.
	static void DrawEnemyJeepLightMesh(ItemInfo& item, bool enabled)
	{
		if (enabled)
		{
			item.MeshBits.Set(ENEMY_JEEP_RIGHT_LIGHT_MESHBITS); // Left blinking light.
			item.MeshBits.Set(ENEMY_JEEP_LEFT_LIGHT_MESHBITS); // Right blinking light.
		}
		else
		{
			item.MeshBits.Clear(ENEMY_JEEP_RIGHT_LIGHT_MESHBITS); // Left blinking light.
			item.MeshBits.Clear(ENEMY_JEEP_LEFT_LIGHT_MESHBITS); // Right blinking light.
		}
	}

	// Draw 2 dynamic light near the light mesh on the back of the jeep.
	static void SpawnEnemyJeepLight(ItemInfo* item)
	{
		auto jointPos = GetJointPosition(item, EnemyJeepRightLightBite);
		TriggerDynamicLight(jointPos.x, jointPos.y, jointPos.z, 10, 64, 0, 0);

		jointPos = GetJointPosition(item, EnemyJeepLeftLightBite);
		TriggerDynamicLight(jointPos.x, jointPos.y, jointPos.z, 10, 64, 0, 0);
	}

	static void SpawnEnemyJeepGrenade(ItemInfo& item)
	{
		auto grenadeIndex = CreateItem();
		if (grenadeIndex == NO_VALUE || item.ItemFlags[3] > 0)
			return;

		auto* grenade = &g_Level.Items[grenadeIndex];
		grenade->ObjectNumber = ID_GRENADE;
		grenade->RoomNumber = item.RoomNumber;
		grenade->Model.Color = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
		auto grenadePos = GetJointPosition(item, EnemyJeepGrenadeBite);
		auto grenadeposF = Vector3(grenadePos.x, grenadePos.y, grenadePos.z);

		grenade->Pose.Orientation.x = item.Pose.Orientation.x;
		grenade->Pose.Orientation.y = item.Pose.Orientation.y + ANGLE(180.0f);
		grenade->Pose.Orientation.z = 0;
		grenade->Pose.Position.x = grenadePos.x + (BLOCK(0.10f) * phd_sin(grenade->Pose.Orientation.y));
		grenade->Pose.Position.y = grenadePos.y;
		grenade->Pose.Position.z = grenadePos.z + (BLOCK(0.10f) * phd_cos(grenade->Pose.Orientation.y));
		InitializeItem(grenadeIndex);

		for (int i = 0; i < 9; i++)
			SpawnGunSmokeParticles(grenadeposF, Vector3(0, 0, 1), item.RoomNumber, 1, LaraWeaponType::RocketLauncher, 32);

		if (GetRandomControl() & 3)
		{
			grenade->ItemFlags[0] = (int)ProjectileType::Grenade;
		}
		else
		{
			grenade->ItemFlags[0] = (int)ProjectileType::FragGrenade;
		}

		grenade->Animation.Velocity.z = ENEMY_JEEP_GRENADE_VELOCITY;
		grenade->Animation.Velocity.y = CLICK(1) * phd_sin(grenade->Pose.Orientation.x);
		grenade->Animation.ActiveState = grenade->Pose.Orientation.x;
		grenade->Animation.TargetState = grenade->Pose.Orientation.y;
		grenade->Animation.RequiredState = NO_VALUE;
		grenade->HitPoints = ENEMY_JEEP_GRENADE_TIMER; // Timer of the grenade.
		item.ItemFlags[3] = ENEMY_JEEP_GRENADE_COOLDOWN_TIME;

		AddActiveItem(grenadeIndex);
		SoundEffect(SFX_TR4_GRENADEGUN_FIRE, &item.Pose);
;	}

	static void RotateTowardTarget(ItemInfo& item, const short angle, short turnRate)
	{
		if (abs(angle) < turnRate)
		{
			item.Pose.Orientation.y += angle;
		}
		else if (angle < 0)
		{
			item.Pose.Orientation.y -= turnRate;
		}
		else
		{
			item.Pose.Orientation.y += turnRate;
		}
	}

	// Check for X1 and X2 AI object and do a path based on X1 ocb, check behaviour with X2 like throw grenade or stop and wait for X sec...
	static void DoNodePath(ItemInfo& item)
	{
		auto& creature = *GetCreatureInfo(&item);

		// Use it to setup the path
		FindAITargetObject(item, ID_AI_X1, creature.LocationAI, false);
		if (Vector3i::Distance(item.Pose.Position, creature.Enemy->Pose.Position) <= ENEMY_JEEP_NEAR_X1_NODE_DISTANCE)
			creature.LocationAI++;

		// Use it to get behaviour if you arrive on X2 ai without modifing the creature.Enemy.
		auto flags = AITargetFlags{};
		flags.CheckDistance = true;
		flags.CheckOcb = false;
		flags.ObjectID = ID_AI_X2;
		flags.Ocb = NO_VALUE;
		flags.CheckSameZone = false;
		flags.DistanceMax = ENEMY_JEEP_NEAR_X2_NODE_DISTANCE;

		if (FindAITargetObject(item, flags))
		{
			DrawDebugSphere(flags.FoundItem.Pose.Position.ToVector3(), 128.0f, Color(1, 1, 0), RendererDebugPage::WireframeMode, true);
			item.ItemFlags[1] = flags.FoundItem.TriggerFlags;
		}
	}

	// Process the AI_X2 ocb and do any required query, like drop grenade or jump pit.
	static void ProcessBehaviour(ItemInfo& item)
	{
		switch (item.ItemFlags[1])
		{
		 // Drop grenade.
		case X2_DROP_GRENADE:
			SpawnEnemyJeepGrenade(item);
			break;

		 // Drop 2 step or more.
		case X2_DROP:
			item.Animation.TargetState = ENEMY_JEEP_STATE_DROP;
			break;

		 // Jump 2 step pit.
		case X2_JUMP_PIT:
			item.Animation.TargetState = ENEMY_JEEP_STATE_JUMP_PIT;
			break;

		// Make the entity disappear/kill itself.
		case X2_DISAPPEAR:
			item.Status = ITEM_INVISIBLE;
			item.Flags |= IFLAG_INVISIBLE;
			RemoveActiveItem(item.Index);
			DisableEntityAI(item.Index);
			break;

		// Make the entity start heavy trigger below it.
		case X2_ACTIVATE_HEAVY_TRIGGER:
			TestTriggers(item.Pose.Position.x, item.Pose.Position.y, item.Pose.Position.z, item.RoomNumber, true, 0);
			break;

		case X2_CUSTOM_DROP:
			item.Animation.TargetState = ENEMY_JEEP_STATE_CUSTOM_DROP;
			break;

		case X2_CUSTOM_JUMP_PIT:
			item.Animation.TargetState = ENEMY_JEEP_STATE_CUSTOM_JUMP_PIT;
			break;

		default:
			bool waitBehaviour = (item.ItemFlags[1] & X2_WAIT_UNTIL_LARA_NEAR) != 0;
			if (waitBehaviour)
			{
				item.Animation.TargetState = ENEMY_JEEP_STATE_STOP;
				item.ItemFlags[4] = 1;
				item.ItemFlags[5] = item.ItemFlags[1] - X2_WAIT_UNTIL_LARA_NEAR;
			}

			break;
		}

		item.ItemFlags[1] = 0; // Reset X2 flags to avoid behaviour loop.
	}

	static bool IsJeepIdle(int activeState)
	{
		return activeState == ENEMY_JEEP_STATE_IDLE ||
			   activeState == ENEMY_JEEP_STATE_STOP;
	}

	static bool IsJeepMoving(int activeState)
	{
		return activeState == ENEMY_JEEP_STATE_MOVE ||
			   activeState == ENEMY_JEEP_STATE_TURN_LEFT ||
			   activeState == ENEMY_JEEP_STATE_TURN_RIGHT;
	}

	static bool IsJeepJumpingOrDropping(int activeState, bool onlyJump = false)
	{
		if (onlyJump)
		{
			return activeState == ENEMY_JEEP_STATE_JUMP_PIT ||
				activeState == ENEMY_JEEP_STATE_CUSTOM_JUMP_PIT;
		}
		else
		{
			return activeState == ENEMY_JEEP_STATE_JUMP_PIT ||
				activeState == ENEMY_JEEP_STATE_DROP ||
				activeState == ENEMY_JEEP_STATE_CUSTOM_DROP ||
				activeState == ENEMY_JEEP_STATE_CUSTOM_JUMP_PIT;
		}
	}

	void InitializeEnemyJeep(short itemNumber)
	{
		auto& item = g_Level.Items[itemNumber];
		item.ItemFlags[2] = 0;
		item.ItemFlags[3] = 0;
		item.ItemFlags[4] = NO_VALUE;
		item.ItemFlags[5] = NO_VALUE;

		InitializeCreature(itemNumber);
		SetAnimation(item, ENEMY_JEEP_ANIM_IDLE);
		DrawEnemyJeepLightMesh(item, true);
	}

	void ControlEnemyJeep(short itemNumber)
	{
		if (!CreatureActive(itemNumber))
			return;

		auto& item = g_Level.Items[itemNumber];
		auto& creature = *GetCreatureInfo(&item);
		auto& object = Objects[item.ObjectNumber];

		AI_INFO ai = {};

		if (item.ItemFlags[3] > 0)
			item.ItemFlags[3]--;
		item.ItemFlags[3] = std::clamp<short>(item.ItemFlags[3], 0, ENEMY_JEEP_GRENADE_COOLDOWN_TIME);

		if (item.HitPoints <= 0)
		{

		}
		else
		{
			CreatureAIInfo(&item, &ai);

			// Manage light mesh and dynamic light based on state.
			if (IsJeepIdle(item.Animation.ActiveState))
			{
				DrawEnemyJeepLightMesh(item, true);
				SpawnEnemyJeepLight(&item);
			}
			else
			{
				DrawEnemyJeepLightMesh(item, false);
			}

			// This jump/drop check need to be there or else the jeep could miss a AI_X1
			// and potentially could break, anyway it's still weird
			// to see it going back to valid the missed AI.
			if (IsJeepMoving(item.Animation.ActiveState) ||
				IsJeepJumpingOrDropping(item.Animation.ActiveState))
			{
				DoNodePath(item);
			}
			
			if (IsJeepMoving(item.Animation.ActiveState))
				RotateTowardTarget(item, ai.angle, ANGLE(5.0f));

			switch (item.Animation.ActiveState)
			{
			case ENEMY_JEEP_STATE_IDLE:
				switch (item.ItemFlags[4])
				{
				// Wait for player to enter a vehicle.
				default:
				case 0:
					if ((item.TriggerFlags & EJ_NO_PLAYER_VEHICLE_REQUIRED) != 0 ||
						Lara.Context.Vehicle != NO_VALUE)
						item.Animation.TargetState = ENEMY_JEEP_STATE_MOVE;
					break;

				 // Wait until player is near.
				case 1:
					if (item.ItemFlags[5] != NO_VALUE &&
						Vector3i::Distance(LaraItem->Pose.Position, item.Pose.Position) <= item.ItemFlags[5])
					{
						item.Animation.TargetState = ENEMY_JEEP_STATE_MOVE;
						item.ItemFlags[4] = NO_VALUE; // Remove state.
						item.ItemFlags[5] = NO_VALUE; // Remove radius.
					}

					break;
				}

				break;

			case ENEMY_JEEP_STATE_MOVE:
				if (ai.angle < -ENEMY_JEEP_WHEEL_LEFTRIGHT_TURN_MINIMUM)
				{
					item.Animation.TargetState = ENEMY_JEEP_STATE_TURN_LEFT;
				}
				else if (ai.angle > ENEMY_JEEP_WHEEL_LEFTRIGHT_TURN_MINIMUM)
				{
					item.Animation.TargetState = ENEMY_JEEP_STATE_TURN_RIGHT;
				}

				break;

			case ENEMY_JEEP_STATE_TURN_LEFT:
			case ENEMY_JEEP_STATE_TURN_RIGHT:
				if (abs(ai.angle) <= ENEMY_JEEP_WHEEL_LEFTRIGHT_TURN_MINIMUM)
					item.Animation.TargetState = ENEMY_JEEP_STATE_MOVE;

				break;
			}
		}

		ProcessBehaviour(item);
		CreatureAnimation(itemNumber, 0, 0);

		if (IsJeepJumpingOrDropping(item.Animation.ActiveState, true))
		{
			// Required, else the entity will go back to previous position (before the jump)
			creature.LOT.IsJumping = true;
		}
		else
		{
			creature.LOT.IsJumping = false;
			AlignEntityToSurface(&item, Vector2(object.radius / 3, (object.radius / 3) * 1.33f), 0.8f);
		}

		// Didn't used Move there because we need the move sound when jump/drop and rotating left/right.
		if (!IsJeepIdle(item.Animation.ActiveState))
		{
			float pitch = std::clamp(0.4f + (float)abs(item.Animation.Velocity.z) / (float)ENEMY_JEEP_PITCH_MAX, 0.6f, 1.4f);
			for (int i = 0; i < 4; i++)
				creature.JointRotation[i] -= ANGLE(pitch * ENEMY_JEEP_PITCH_WHEEL_SPEED_MULTIPLIER);

			SoundEffect(SFX_TR4_VEHICLE_JEEP_MOVING, &item.Pose, SoundEnvironment::Land, pitch, 1.5f);
		}
		else
		{
			for (int i = 0; i < 4; i++)
				creature.JointRotation[i] = 0;

			SoundEffect(SFX_TR4_VEHICLE_JEEP_IDLE, &item.Pose);
		}
	}
}
