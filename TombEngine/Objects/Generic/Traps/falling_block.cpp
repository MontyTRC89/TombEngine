#include "framework.h"
#include "Objects/Generic/Traps/falling_block.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/floordata.h"
#include "Game/effects/debris.h"
#include "Game/room.h"
#include "Math/Random.h"
#include "Sound/sound.h"
#include "Specific/level.h"
#include "Specific/setup.h"

using namespace TEN::Floordata;
using namespace TEN::Math::Random;

constexpr auto FALLINGBLOCK_INITIAL_SPEED		= 10;
constexpr auto FALLINGBLOCK_MAX_SPEED			= 100;
constexpr auto FALLINGBLOCK_FALL_VELOCITY		= 4;
constexpr auto FALLINGBLOCK_FALL_ROTATION_SPEED = 1;
constexpr auto FALLINGBLOCK_DELAY				= 52;
constexpr auto FALLINGBLOCK_WIBBLE				= 3;
constexpr auto FALLINGBLOCK_HEIGHT_TOLERANCE	= 8;
constexpr auto FALLINGBLOCK_CRUMBLE_DELAY		= 100;

void InitialiseFallingBlock(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	g_Level.Items[itemNumber].MeshBits = 1;
	TEN::Floordata::UpdateBridgeItem(itemNumber);

	// Set mutators to EulerAngles identity by default.
	for (auto& mutator : item->Model.Mutators)
		mutator.Rotation = EulerAngles::Zero;
}

void FallingBlockCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
{
	auto* item = &g_Level.Items[itemNumber];

	if (!item->ItemFlags[0] && !item->TriggerFlags &&
		abs(item->Pose.Position.y - laraItem->Pose.Position.y) < FALLINGBLOCK_HEIGHT_TOLERANCE)
	{
		if (!((item->Pose.Position.x ^ laraItem->Pose.Position.x) & 0xFFFFFC00) && !((laraItem->Pose.Position.z ^ item->Pose.Position.z) & 0xFFFFFC00))
		{
			SoundEffect(SFX_TR4_ROCK_FALL_CRUMBLE, &item->Pose);
			AddActiveItem(itemNumber);

			item->ItemFlags[0] = 0;
			item->Status = ITEM_ACTIVE;
			item->Flags |= CODE_BITS;
		}
	}
}

void FallingBlockControl(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (item->TriggerFlags)
	{
		item->TriggerFlags--;
	}
	else
	{
		if (item->ItemFlags[0])
		{
			if (item->ItemFlags[0] < FALLINGBLOCK_DELAY)
			{
				// Subtly wobble all meshes separately.
				for (auto& mutator : item->Model.Mutators)
				{
					mutator.Rotation = EulerAngles(
						ANGLE(GenerateFloat(-FALLINGBLOCK_WIBBLE, FALLINGBLOCK_WIBBLE)),
						ANGLE(GenerateFloat(-FALLINGBLOCK_WIBBLE, FALLINGBLOCK_WIBBLE)),
						ANGLE(GenerateFloat(-FALLINGBLOCK_WIBBLE, FALLINGBLOCK_WIBBLE)));
				}
			}
			else
			{
				// Make rotational falling movement with random seed.
				for (int i = 0; i < item->Model.Mutators.size(); i++)
				{
					auto rotRate = i % 2 ? FALLINGBLOCK_FALL_ROTATION_SPEED : -FALLINGBLOCK_FALL_ROTATION_SPEED;
					rotRate += i % 3 ? (rotRate / 2) : rotRate;

					item->Model.Mutators[i].Rotation += EulerAngles(
						ANGLE(rotRate + GenerateFloat(-1, 1)),
						ANGLE(rotRate + GenerateFloat(-1, 1)),
						ANGLE(rotRate + GenerateFloat(-1, 1)));
				}

				if (item->ItemFlags[0] == FALLINGBLOCK_DELAY)
					item->ItemFlags[1] = FALLINGBLOCK_INITIAL_SPEED;

				if (item->ItemFlags[1] > 0)
				{
					item->ItemFlags[1] += FALLINGBLOCK_FALL_VELOCITY;

					if (item->ItemFlags[1] > FALLINGBLOCK_MAX_SPEED)
						item->ItemFlags[1] = FALLINGBLOCK_MAX_SPEED;

					item->Pose.Position.y += item->ItemFlags[1];
				}

				if (GetDistanceToFloor(itemNumber) >= 0)
				{
					// If crumbled before actual delay (e.g. too low position), force delay to be correct
					if (item->ItemFlags[0] < FALLINGBLOCK_DELAY)
						item->ItemFlags[0] = FALLINGBLOCK_DELAY;

					// Convert object to shatter item
					ShatterItem.yRot = item->Pose.Orientation.y;
					ShatterItem.meshIndex = Objects[item->ObjectNumber].meshIndex;
					ShatterItem.color = item->Model.Color;
					ShatterItem.sphere.x = item->Pose.Position.x;
					ShatterItem.sphere.y = item->Pose.Position.y - CLICK(1); // So debris won't spawn below floor
					ShatterItem.sphere.z = item->Pose.Position.z;
					ShatterItem.bit = 0;
					ShatterImpactData.impactDirection = Vector3(0, -(float)item->ItemFlags[1] / (float)FALLINGBLOCK_MAX_SPEED, 0);
					ShatterImpactData.impactLocation = { (float)ShatterItem.sphere.x, (float)ShatterItem.sphere.y, (float)ShatterItem.sphere.z };
					ShatterObject(&ShatterItem, nullptr, 0, item->RoomNumber, false);

					SoundEffect(SFX_TR4_ROCK_FALL_LAND, &item->Pose);
					KillItem(itemNumber);
				}
			}

			item->ItemFlags[0]++;
		}
		else
		{
			item->MeshBits = -2;
			item->ItemFlags[0]++;
			SoundEffect(SFX_TR4_ROCK_FALL_CRUMBLE, &item->Pose);
		}
	}
}

std::optional<int> FallingBlockFloor(short itemNumber, int x, int y, int z)
{
	ItemInfo* item = &g_Level.Items[itemNumber];
	if (!item->MeshBits.TestAny() || item->ItemFlags[0] >= FALLINGBLOCK_DELAY)
		return std::nullopt;

	return GetBridgeItemIntersect(itemNumber, x, y, z, false);
}

std::optional<int> FallingBlockCeiling(short itemNumber, int x, int y, int z)
{
	ItemInfo* item = &g_Level.Items[itemNumber];

	if (!item->MeshBits.TestAny() || item->ItemFlags[0] >= FALLINGBLOCK_DELAY)
		return std::nullopt;

	return GetBridgeItemIntersect(itemNumber, x, y, z, true);
}

int FallingBlockFloorBorder(short itemNumber)
{
	return GetBridgeBorder(itemNumber, false);
}

int FallingBlockCeilingBorder(short itemNumber)
{
	return GetBridgeBorder(itemNumber, true);
}
