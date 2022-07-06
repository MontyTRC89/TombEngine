#include "framework.h"
#include "Game/effects/debris.h"
#include "Game/collision/collide_room.h"
#include "Game/camera.h"
#include "Game/animation.h"
#include "Game/collision/floordata.h"
#include "Specific/level.h"
#include "Specific/setup.h"
#include "Game/room.h"
#include "Sound/sound.h"
#include "Specific/prng.h"

using namespace TEN::Floordata;
using namespace TEN::Math::Random;

constexpr auto FALLINGBLOCK_INITIAL_SPEED = 10;
constexpr auto FALLINGBLOCK_MAX_SPEED = 100;
constexpr auto FALLINGBLOCK_FALL_VELOCITY = 4;
constexpr auto FALLINGBLOCK_FALL_ROTATION_SPEED = 1;
constexpr auto FALLINGBLOCK_DELAY = 52;
constexpr auto FALLINGBLOCK_WIBBLE = 3;
constexpr auto FALLINGBLOCK_HEIGHT_TOLERANCE = 8;
constexpr auto FALLINGBLOCK_CRUMBLE_DELAY = 100;

void InitialiseFallingBlock(short itemNumber)
{
	auto item = &g_Level.Items[itemNumber];

	g_Level.Items[itemNumber].MeshBits = 1;
	TEN::Floordata::UpdateBridgeItem(itemNumber);

	// Set mutators to 0 by default
	for (int i = 0; i < item->Animation.Mutator.size(); i++)
		item->Animation.Mutator[i].Rotation = Vector3::Zero;
}

void FallingBlockCollision(short itemNum, ItemInfo* l, CollisionInfo* coll)
{
	ItemInfo* item = &g_Level.Items[itemNum];
	if (!item->ItemFlags[0] && !item->TriggerFlags && abs(item->Pose.Position.y - l->Pose.Position.y) < FALLINGBLOCK_HEIGHT_TOLERANCE)
	{
		if (!((item->Pose.Position.x ^ l->Pose.Position.x) & 0xFFFFFC00) && !((l->Pose.Position.z ^ item->Pose.Position.z) & 0xFFFFFC00))
		{
			SoundEffect(SFX_TR4_ROCK_FALL_CRUMBLE, &item->Pose);
			AddActiveItem(itemNum);

			item->ItemFlags[0] = 0;
			item->Status = ITEM_ACTIVE;
			item->Flags |= 0x3E00;
		}
	}
}

void FallingBlockControl(short itemNumber)
{
	ItemInfo* item = &g_Level.Items[itemNumber];

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
				// Subtly shake all meshes separately
				for (int i = 0; i < item->Animation.Mutator.size(); i++)
				{
					item->Animation.Mutator[i].Rotation.x = RADIAN * GenerateFloat(-FALLINGBLOCK_WIBBLE, FALLINGBLOCK_WIBBLE);
					item->Animation.Mutator[i].Rotation.y = RADIAN * GenerateFloat(-FALLINGBLOCK_WIBBLE, FALLINGBLOCK_WIBBLE);
					item->Animation.Mutator[i].Rotation.z = RADIAN * GenerateFloat(-FALLINGBLOCK_WIBBLE, FALLINGBLOCK_WIBBLE);
				}
			}
			else
			{
				// Make rotational falling movement with some random seed
				for (int i = 0; i < item->Animation.Mutator.size(); i++)
				{
					auto rotSpeed = i % 2 ? FALLINGBLOCK_FALL_ROTATION_SPEED : -FALLINGBLOCK_FALL_ROTATION_SPEED;
					rotSpeed += i % 3 ? rotSpeed / 2 : rotSpeed;
					item->Animation.Mutator[i].Rotation.x += RADIAN * rotSpeed + (RADIAN * GenerateFloat(-1, 1));
					item->Animation.Mutator[i].Rotation.y += RADIAN * rotSpeed + (RADIAN * GenerateFloat(-1, 1));
					item->Animation.Mutator[i].Rotation.z += RADIAN * rotSpeed + (RADIAN * GenerateFloat(-1, 1));
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
			}

			item->ItemFlags[0]++;

			if (GetDistanceToFloor(itemNumber) >= 0)
			{
				// If crumbled before actual delay (e.g. too low position), force delay to be correct
				if (item->ItemFlags[0] < FALLINGBLOCK_DELAY)
					item->ItemFlags[0] = FALLINGBLOCK_DELAY;

				// Convert object to shatter item
				ShatterItem.yRot = item->Pose.Orientation.y;
				ShatterItem.meshIndex = Objects[item->ObjectNumber].meshIndex;
				ShatterItem.sphere.x = item->Pose.Position.x;
				ShatterItem.sphere.y = item->Pose.Position.y - STEP_SIZE; // So debris won't spawn below floor
				ShatterItem.sphere.z = item->Pose.Position.z;
				ShatterItem.bit = 0;
				ShatterImpactData.impactDirection = Vector3(0, -(float)item->ItemFlags[1] / (float)FALLINGBLOCK_MAX_SPEED, 0);
				ShatterImpactData.impactLocation = { (float)ShatterItem.sphere.x, (float)ShatterItem.sphere.y, (float)ShatterItem.sphere.z };
				ShatterObject(&ShatterItem, nullptr, 0, item->RoomNumber, false);

				SoundEffect(SFX_TR4_ROCK_FALL_LAND, &item->Pose);
				KillItem(itemNumber);
			}
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
	if (!item->MeshBits || item->ItemFlags[0] >= FALLINGBLOCK_DELAY)
		return std::nullopt;

	return GetBridgeItemIntersect(itemNumber, x, y, z, false);
}

std::optional<int> FallingBlockCeiling(short itemNumber, int x, int y, int z)
{
	ItemInfo* item = &g_Level.Items[itemNumber];

	if (!item->MeshBits || item->ItemFlags[0] >= FALLINGBLOCK_DELAY)
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
