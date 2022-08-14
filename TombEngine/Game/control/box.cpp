#include "framework.h"
#include "Game/control/box.h"

#include "Game/animation.h"
#include "Game/camera.h"
#include "Game/collision/sphere.h"
#include "Game/collision/collide_room.h"
#include "Game/control/control.h"
#include "Game/control/lot.h"
#include "Game/effects/tomb4fx.h"
#include "Game/itemdata/creature_info.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/items.h"
#include "Game/misc.h"
#include "Game/room.h"
#include "Specific/setup.h"
#include "Specific/trmath.h"
#include "Objects/objectslist.h"
#include "Objects/TR5/Object/tr5_pushableblock.h"
#include "Renderer/Renderer11.h"

#define CHECK_CLICK(x) CLICK(x) / 2
#define ESCAPE_DIST SECTOR(5)
#define STALK_DIST SECTOR(3)
#define REACHED_GOAL_RADIUS 640
#define ATTACK_RANGE pow(SECTOR(3), 2)
#define ESCAPE_CHANCE  0x800
#define RECOVER_CHANCE 0x100
#define BIFF_AVOID_TURN 1536
#define FEELER_DISTANCE CLICK(2)
#define FEELER_ANGLE ANGLE(45.0f)

#ifdef CREATURE_AI_PRIORITY_OPTIMIZATION
constexpr int HIGH_PRIO_RANGE = 8;
constexpr int MEDIUM_PRIO_RANGE = HIGH_PRIO_RANGE + HIGH_PRIO_RANGE * (HIGH_PRIO_RANGE / 6.0f);
constexpr int LOW_PRIO_RANGE = MEDIUM_PRIO_RANGE + MEDIUM_PRIO_RANGE * (MEDIUM_PRIO_RANGE / 24.0f);
constexpr int NONE_PRIO_RANGE = LOW_PRIO_RANGE + LOW_PRIO_RANGE * (LOW_PRIO_RANGE / 32.0f);
constexpr auto FRAME_PRIO_BASE = 4;
constexpr auto FRAME_PRIO_EXP = 1.5;
#endif // CREATURE_AI_PRIORITY_OPTIMIZATION

void DrawBox(int boxIndex, Vector3 color)
{
	if (boxIndex == NO_BOX)
		return;

	auto& currBox = g_Level.Boxes[boxIndex];

	float x = ((float)currBox.left + (float)(currBox.right - currBox.left) / 2.0f) * 1024.0f;
	auto  y = currBox.height - CLICK(1);
	float z = ((float)currBox.top + (float)(currBox.bottom - currBox.top) / 2.0f) * 1024.0f;


	auto center = Vector3(z, y, x);
	auto corner = Vector3(currBox.bottom * SECTOR(1), currBox.height + CLICK(1), currBox.right * SECTOR(1));
	auto extents = (corner - center) * 0.9f;
	auto dBox = BoundingOrientedBox(center, extents, Vector4::UnitY);

	for (int i = 0; i <= 10; i++)
	{
		dBox.Extents = extents + Vector3(i);
		TEN::Renderer::g_Renderer.AddDebugBox(dBox, Vector4(color.x, color.y, color.z, 1), RENDERER_DEBUG_PAGE::LOGIC_STATS);
	}
}

void DrawNearbyPathfinding(int boxIndex)
{
	if (boxIndex == NO_BOX)
		return;

	DrawBox(boxIndex, Vector3(0, 1, 1));

	auto& currBox = g_Level.Boxes[boxIndex];
	auto index = currBox.overlapIndex;

	while (true)
	{
		if (index >= g_Level.Overlaps.size())
			break;

		auto overlap = g_Level.Overlaps[index];

		DrawBox(overlap.box, Vector3(1, 1, 0));

		if (overlap.flags & BOX_END_BIT)
			break;
		else
			index++;
	}
}

void DropEntityPickups(ItemInfo* item)
{
	ItemInfo* pickup = NULL;

	for (short pickupNumber = item->CarriedItem; pickupNumber != NO_ITEM; pickupNumber = pickup->CarriedItem)
	{
		pickup = &g_Level.Items[pickupNumber];
		pickup->Pose.Position.x = (item->Pose.Position.x & -CLICK(1)) | CLICK(1);
		pickup->Pose.Position.z = (item->Pose.Position.z & -CLICK(1)) | CLICK(1);

		pickup->Pose.Position.y = GetCollision(pickup->Pose.Position.x, item->Pose.Position.y, pickup->Pose.Position.z, item->RoomNumber).Position.Floor;
		auto* bounds = GetBoundsAccurate(pickup);
		pickup->Pose.Position.y -= bounds->Y2;

		ItemNewRoom(pickupNumber, item->RoomNumber);
		pickup->Flags |= 32;
	}
}

bool MoveCreature3DPos(PHD_3DPOS* origin, PHD_3DPOS* target, int velocity, short angleDif, int angleAdd)
{
	auto differenceVector = target->Position - origin->Position;
	float distance = Vector3::Distance(origin->Position.ToVector3(), target->Position.ToVector3());

	if (velocity < distance)
		origin->Position += differenceVector * (velocity / distance);
	else
		origin->Position = target->Position;

	if (angleDif <= angleAdd)
	{
		if (angleDif >= -angleAdd)
			origin->Orientation.y = target->Orientation.y;
		else
			origin->Orientation.y -= angleAdd;
	}
	else
		origin->Orientation.y += angleAdd;

	if (origin->Position == target->Position &&
		origin->Orientation.y == target->Orientation.y)
	{
		return true;
	}

	return false;
}

void CreatureYRot2(PHD_3DPOS* srcPos, short angle, short angleAdd) 
{
	if (angleAdd < angle)
	{
		srcPos->Orientation.y += angleAdd;
		return;
	} 

	if (angle < -angleAdd)
	{
		srcPos->Orientation.y -= angleAdd;
		return;
	} 

	srcPos->Orientation.y += angle;
}

bool SameZone(CreatureInfo* creature, ItemInfo* target)
{
	int* zone = g_Level.Zones[creature->LOT.Zone][FlipStatus].data();
	auto* item = &g_Level.Items[creature->ItemNumber];

	auto* room = &g_Level.Rooms[item->RoomNumber];
	FloorInfo* floor = GetSector(room, item->Pose.Position.x - room->x, item->Pose.Position.z - room->z);
	if (floor->Box == NO_BOX)
		return false;

	item->BoxNumber = floor->Box;

	room = &g_Level.Rooms[target->RoomNumber];
	floor = GetSector(room, target->Pose.Position.x - room->x, target->Pose.Position.z - room->z);

	if (floor->Box == NO_BOX)
		return false;

	target->BoxNumber = floor->Box;

	return (zone[item->BoxNumber] == zone[target->BoxNumber]);
}

short AIGuard(CreatureInfo* creature) 
{
	auto* item = &g_Level.Items[creature->ItemNumber];
	if (item->AIBits & (GUARD | PATROL1))
		return 0;

	int random = GetRandomControl();

	if (random < 256)
	{
		creature->HeadRight = true;
		creature->HeadLeft = true;
	}
	else if (random < 384)
	{
		creature->HeadRight = false;
		creature->HeadLeft = true;
	}
	else if (random < 512)
	{
		creature->HeadRight = true;
		creature->HeadLeft = false;
	}

	if (!creature->HeadLeft)
		return (creature->HeadRight) << 12;

	if (creature->HeadRight)
		return 0;

	return -ANGLE(90.0f);
}

void AlertNearbyGuards(ItemInfo* item) 
{
	for (int i = 0; i < ActiveCreatures.size(); i++)
	{
		auto* currentCreature = ActiveCreatures[i];
		if (currentCreature->ItemNumber == NO_ITEM)
			continue;

		auto* currentTarget = &g_Level.Items[currentCreature->ItemNumber + i];
		if (item->RoomNumber == currentTarget->RoomNumber)
		{
			currentCreature->Alerted = true;
			continue;
		}

		int x = (currentTarget->Pose.Position.x - item->Pose.Position.x) / 64;
		int y = (currentTarget->Pose.Position.y - item->Pose.Position.y) / 64;
		int z = (currentTarget->Pose.Position.z - item->Pose.Position.z) / 64;

		int distance = (pow(x, 2) + pow(y, 2) + pow(z, 2));
		if (distance < SECTOR(8))
			currentCreature->Alerted = true;
	}
}

void AlertAllGuards(short itemNumber) 
{
	for (int i = 0; i < ActiveCreatures.size(); i++)
	{
		auto* creature = ActiveCreatures[i];
		if (creature->ItemNumber == NO_ITEM)
			continue;

		auto* target = &g_Level.Items[creature->ItemNumber];
		short objNumber = g_Level.Items[itemNumber].ObjectNumber;
		if (objNumber == target->ObjectNumber)
		{
			if (target->Status == ITEM_ACTIVE)
				creature->Alerted = true;
		}
	}
}

void CreatureKill(ItemInfo* item, int killAnim, int killState, int laraKillState)
{
	item->Animation.AnimNumber = Objects[item->ObjectNumber].animIndex + killAnim;
	item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
	item->Animation.ActiveState = killState;

	LaraItem->Animation.AnimNumber = Objects[ID_LARA_EXTRA_ANIMS].animIndex;
	LaraItem->Animation.FrameNumber = g_Level.Anims[LaraItem->Animation.AnimNumber].frameBase;
	LaraItem->Animation.ActiveState = 0;
	LaraItem->Animation.TargetState = laraKillState;

	LaraItem->Pose = item->Pose;
	LaraItem->Animation.IsAirborne = false;
	LaraItem->Animation.Velocity.z = 0;
	LaraItem->Animation.Velocity.y = 0;

	if (item->RoomNumber != LaraItem->RoomNumber)
		ItemNewRoom(Lara.ItemNumber, item->RoomNumber);

	AnimateItem(LaraItem);

	Lara.ExtraAnim = 1;
	Lara.Control.HandStatus = HandStatus::Busy;
	Lara.Control.Weapon.GunType = LaraWeaponType::None;
	Lara.HitDirection = -1;
	Lara.Air = -1;

	Camera.pos.roomNumber = LaraItem->RoomNumber; 
	Camera.type = CameraType::Chase;
	Camera.flags = CF_FOLLOW_CENTER;
	Camera.targetAngle = ANGLE(170.0f);
	Camera.targetElevation = -ANGLE(25.0f);

	// TODO: exist in TR5 but just commented in case.
	/*
	ForcedFixedCamera.x = item->pos.Position.x + (phd_sin(item->pos.Orientation.y) << 13) >> W2V_SHIFT;
	ForcedFixedCamera.y = item->pos.Position.y - WALL_SIZE;
	ForcedFixedCamera.z = item->pos.Position.z + (phd_cos(item->pos.Orientation.y) << 13) >> W2V_SHIFT;
	ForcedFixedCamera.roomNumber = item->roomNumber;
	UseForcedFixedCamera = true;
	*/
}

short CreatureEffect2(ItemInfo* item, BiteInfo bite, short velocity, short angle, std::function<CreatureEffectFunction> func)
{
	auto pos = Vector3Int(bite.x, bite.y, bite.z);
	GetJointAbsPosition(item, &pos, bite.meshNum);
	return func(pos.x, pos.y, pos.z, velocity, angle, item->RoomNumber);
}

short CreatureEffect(ItemInfo* item, BiteInfo bite, std::function<CreatureEffectFunction> func)
{
	auto pos = Vector3Int(bite.x, bite.y, bite.z);
	GetJointAbsPosition(item, &pos, bite.meshNum);
	return func(pos.x, pos.y, pos.z, item->Animation.Velocity.z, item->Pose.Orientation.y, item->RoomNumber);
}

void CreatureUnderwater(ItemInfo* item, int depth)
{
	int waterLevel = depth;
	int waterHeight = 0;

	if (depth < 0)
	{
		waterHeight = abs(depth);
		waterLevel = 0;
	}
	else
		waterHeight = GetWaterHeight(item);

	int y = waterHeight + waterLevel;

	if (item->Pose.Position.y < y)
	{
		int height = GetCollision(item).Position.Floor;

		item->Pose.Position.y = y;
		if (y > height)
			item->Pose.Position.y = height;

		if (item->Pose.Orientation.x > ANGLE(2.0f))
			item->Pose.Orientation.x -= ANGLE(2.0f);
		else if (item->Pose.Orientation.x > 0)
			item->Pose.Orientation.x = 0;
	}
}

void CreatureFloat(short itemNumber) 
{
	auto* item = &g_Level.Items[itemNumber];
	item->HitPoints = NOT_TARGETABLE;
	item->Pose.Orientation.x = 0;

	int waterLevel = GetWaterHeight(item);

	int y = item->Pose.Position.y;
	if (y > waterLevel)
		item->Pose.Position.y = y - 32;
	if (item->Pose.Position.y < waterLevel)
		item->Pose.Position.y = waterLevel;

	AnimateItem(item);

	auto probe = GetCollision(item);
	item->Floor = probe.Position.Floor;
	
	if (probe.RoomNumber != item->RoomNumber)
		ItemNewRoom(itemNumber, probe.RoomNumber);

	if (item->Pose.Position.y <= waterLevel)
	{
		if (item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameBase)
		{
			item->Pose.Position.y = waterLevel;
			item->Collidable = false;
			item->Status = ITEM_DEACTIVATED;
			DisableEntityAI(itemNumber);
			RemoveActiveItem(itemNumber);
			item->AfterDeath = 1;
		}
	}
}

void CreatureJoint(ItemInfo* item, short joint, short required) 
{
	if (!item->Data)
		return;

	auto* creature = GetCreatureInfo(item);

	short change = required - creature->JointRotation[joint];
	if (change > ANGLE(3.0f))
		change = ANGLE(3.0f);
	else if (change < -ANGLE(3.0f))
		change = -ANGLE(3.0f);

	creature->JointRotation[joint] += change;

	if (creature->JointRotation[joint] > ANGLE(70.0f))
		creature->JointRotation[joint] = ANGLE(70.0f);
	else if (creature->JointRotation[joint] < -ANGLE(70.0f))
		creature->JointRotation[joint] = -ANGLE(70.0f);
}

void CreatureTilt(ItemInfo* item, short angle) 
{
	angle = (angle << 2) - item->Pose.Orientation.z;

	if (angle < -ANGLE(3.0f))
		angle = -ANGLE(3.0f);
	else if (angle > ANGLE(3.0f))
		angle = ANGLE(3.0f);

	short theAngle = -ANGLE(3.0f);

	short absRot = abs(item->Pose.Orientation.z);
	if (absRot < ANGLE(15.0f) || absRot > ANGLE(30.0f))
		angle >>= 1;
	
	item->Pose.Orientation.z += angle;
}

short CreatureTurn(ItemInfo* item, short maxTurn)
{
	if (!item->Data || maxTurn == 0)
		return 0;

	auto* creature = GetCreatureInfo(item);
	short angle = 0;

	int x = creature->Target.x - item->Pose.Position.x;
	int z = creature->Target.z - item->Pose.Position.z;
	angle = phd_atan(z, x) - item->Pose.Orientation.y;
	int range = (item->Animation.Velocity.z * 16384) / maxTurn;
	int distance = pow(x, 2) + pow(z, 2);

	if (angle > FRONT_ARC || angle < -FRONT_ARC && distance < pow(range, 2))
		maxTurn /= 2;

	if (angle > maxTurn)
		angle = maxTurn;
	else if (angle < -maxTurn)
		angle = -maxTurn;

	item->Pose.Orientation.y += angle;

	return angle;
}

int CreatureAnimation(short itemNumber, short angle, short tilt)
{
	int xPos, zPos, ceiling, shiftX, shiftZ;
	short top;

	auto* item = &g_Level.Items[itemNumber];
	if (!item->Data)
		return false;

	auto* creature = GetCreatureInfo(item);
	auto* LOT = &creature->LOT;
	int* zone = g_Level.Zones[LOT->Zone][FlipStatus].data();

	int boxHeight;
	if (item->BoxNumber != NO_BOX)
		boxHeight = g_Level.Boxes[item->BoxNumber].height;
	else
		boxHeight = item->Floor;

	auto old = item->Pose.Position;

	AnimateItem(item);

	if (item->Status == ITEM_DEACTIVATED)
	{
		CreatureDie(itemNumber, false);
		return false;
	}

	auto* bounds = GetBoundsAccurate(item);

	int y = item->Pose.Position.y + bounds->Y1;

	short roomNumber = item->RoomNumber;
	GetFloor(old.x, y, old.z, &roomNumber);  
	FloorInfo* floor = GetFloor(item->Pose.Position.x, y, item->Pose.Position.z, &roomNumber);

	// TODO: Check why some blocks have box = -1 assigned to them -- Lwmte, 10.11.21
	if (floor->Box == NO_BOX)
		return false;

	int height = g_Level.Boxes[floor->Box].height;
	int nextHeight = 0;

	int nextBox;
	if (!Objects[item->ObjectNumber].nonLot)
		nextBox = LOT->Node[floor->Box].exitBox;
	else
	{
		floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
		height = g_Level.Boxes[floor->Box].height;
		nextBox = floor->Box;
	}

	if (nextBox == NO_BOX)
		nextHeight = height;
	else
		nextHeight = g_Level.Boxes[nextBox].height;

	if (floor->Box == NO_BOX || !LOT->IsJumping && (LOT->Fly == NO_FLYING && item->BoxNumber != NO_BOX && zone[item->BoxNumber] != zone[floor->Box] ||  boxHeight - height > LOT->Step ||  boxHeight - height < LOT->Drop))
	{
		xPos = item->Pose.Position.x / SECTOR(1);
		zPos = item->Pose.Position.z / SECTOR(1);
		shiftX = old.x / SECTOR(1);
		shiftZ = old.z / SECTOR(1);

		if (xPos < shiftX)
			item->Pose.Position.x = old.x & (~(SECTOR(1) - 1));
		else if (xPos > shiftX)
			item->Pose.Position.x = old.x | (SECTOR(1) - 1);

		if (zPos < shiftZ)
			item->Pose.Position.z = old.z & (~(SECTOR(1) - 1));
		else if (zPos > shiftZ)
			item->Pose.Position.z = old.z | (SECTOR(1) - 1);

		floor = GetFloor(item->Pose.Position.x, y, item->Pose.Position.z, &roomNumber);
		height = g_Level.Boxes[floor->Box].height;
		if (!Objects[item->ObjectNumber].nonLot)
			nextBox = LOT->Node[floor->Box].exitBox;
		else
		{
			floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
			height = g_Level.Boxes[floor->Box].height;
			nextBox = floor->Box;
		}

		if (nextBox == NO_BOX)
			nextHeight = height;
		else
			nextHeight = g_Level.Boxes[nextBox].height;
	}

	int x = item->Pose.Position.x;
	int z = item->Pose.Position.z;
	xPos = x & (SECTOR(1) - 1);
	zPos = z & (SECTOR(1) - 1);
	short radius = Objects[item->ObjectNumber].radius;
	shiftX = 0;
	shiftZ = 0;

	if (zPos < radius)
	{
		if (BadFloor(x, y, z - radius, height, nextHeight, roomNumber, LOT))
			shiftZ = radius - zPos;

		if (xPos < radius)
		{
			if (BadFloor(x - radius, y, z, height, nextHeight, roomNumber, LOT))
				shiftX = radius - xPos;
			else if (!shiftZ && BadFloor(x - radius, y, z - radius, height, nextHeight, roomNumber, LOT))
			{
				if (item->Pose.Orientation.y > -ANGLE(135.0f) && item->Pose.Orientation.y < ANGLE(45.0f))
					shiftZ = radius - zPos;
				else
					shiftX = radius - xPos;
			}
		}
		else if (xPos > SECTOR(1) - radius)
		{
			if (BadFloor(x + radius, y, z, height, nextHeight, roomNumber, LOT))
				shiftX = SECTOR(1) - radius - xPos;
			else if (!shiftZ && BadFloor(x + radius, y, z - radius, height, nextHeight, roomNumber, LOT))
			{
				if (item->Pose.Orientation.y > -ANGLE(45.0f) && item->Pose.Orientation.y < ANGLE(135.0f))
					shiftZ = radius - zPos;
				else
					shiftX = SECTOR(1) - radius - xPos;
			}
		}
	}
	else if (zPos > SECTOR(1) - radius)
	{
		if (BadFloor(x, y, z + radius, height, nextHeight, roomNumber, LOT))
			shiftZ = SECTOR(1) - radius - zPos;

		if (xPos < radius)
		{
			if (BadFloor(x - radius, y, z, height, nextHeight, roomNumber, LOT))
				shiftX = radius - xPos;
			else if (!shiftZ && BadFloor(x - radius, y, z + radius, height, nextHeight, roomNumber, LOT))
			{
				if (item->Pose.Orientation.y > -ANGLE(45.0f) && item->Pose.Orientation.y < ANGLE(135.0f))
					shiftX = radius - xPos;
				else
					shiftZ = SECTOR(1) - radius - zPos;
			}
		}
		else if (xPos > SECTOR(1) - radius)
		{
			if (BadFloor(x + radius, y, z, height, nextHeight, roomNumber, LOT))
				shiftX = SECTOR(1) - radius - xPos;
			else if (!shiftZ && BadFloor(x + radius, y, z + radius, height, nextHeight, roomNumber, LOT))
			{
				if (item->Pose.Orientation.y > -ANGLE(135.0f) && item->Pose.Orientation.y < ANGLE(45.0f))
					shiftX = SECTOR(1) - radius - xPos;
				else
					shiftZ = SECTOR(1) - radius - zPos;
			}
		}
	}
	else if (xPos < radius)
	{
		if (BadFloor(x - radius, y, z, height, nextHeight, roomNumber, LOT))
			shiftX = radius - xPos;
	}
	else if (xPos > SECTOR(1) - radius)
	{
		if (BadFloor(x + radius, y, z, height, nextHeight, roomNumber, LOT))
			shiftX = SECTOR(1) - radius - xPos;
	}

	item->Pose.Position.x += shiftX;
	item->Pose.Position.z += shiftZ;

	if (shiftX || shiftZ)
	{
		floor = GetFloor(item->Pose.Position.x, y, item->Pose.Position.z, &roomNumber);
		item->Pose.Orientation.y += angle;

		if (tilt)
			CreatureTilt(item, (tilt * 2));
	}

	short biffAngle;
	if (item->ObjectNumber != ID_TYRANNOSAUR && item->Animation.Velocity.z && item->HitPoints > 0)
		biffAngle = CreatureCreature(itemNumber);
	else
		biffAngle = 0;

	if (biffAngle)
	{
		if (abs(biffAngle) < BIFF_AVOID_TURN)
			item->Pose.Orientation.y -= BIFF_AVOID_TURN;
		else if (biffAngle > 0)
			item->Pose.Orientation.y -= BIFF_AVOID_TURN;
		else
			item->Pose.Orientation.y += BIFF_AVOID_TURN;

		return true;
	}

	if (LOT->Fly != NO_FLYING && item->HitPoints > 0)
	{
		int dy = creature->Target.y - item->Pose.Position.y;
		if (dy > LOT->Fly)
			dy = LOT->Fly;
		else if (dy < -LOT->Fly)
			dy = -LOT->Fly;

		height = GetFloorHeight(floor, item->Pose.Position.x, y, item->Pose.Position.z);
		if (item->Pose.Position.y + dy <= height)
		{
			if (Objects[item->ObjectNumber].waterCreature)
			{
				ceiling = GetCeiling(floor, item->Pose.Position.x, y, item->Pose.Position.z);

				if (item->ObjectNumber == ID_WHALE)
					top = CLICK(0.5f);
				else
					top = bounds->Y1;

				if (item->Pose.Position.y + top + dy < ceiling)
				{
					if (item->Pose.Position.y + top < ceiling)
					{
						item->Pose.Position.x = old.x;
						item->Pose.Position.z = old.z;
						dy = LOT->Fly;
					}
					else
						dy = 0;
				}
			}
			else
			{
				floor = GetFloor(item->Pose.Position.x, y + CLICK(1), item->Pose.Position.z, &roomNumber);
				if (TestEnvironment(ENV_FLAG_WATER, roomNumber) ||
					TestEnvironment(ENV_FLAG_SWAMP, roomNumber))
				{
					dy = -LOT->Fly;
				}
			}
		}
		else if (item->Pose.Position.y <= height)
		{
			dy = 0;
			item->Pose.Position.y = height;
		}
		else
		{
			item->Pose.Position.x = old.x;
			item->Pose.Position.z = old.z;
			dy = -LOT->Fly;
		}

		item->Pose.Position.y += dy;
		floor = GetFloor(item->Pose.Position.x, y, item->Pose.Position.z, &roomNumber);
		item->Floor = GetFloorHeight(floor, item->Pose.Position.x, y, item->Pose.Position.z);
 
		angle = (item->Animation.Velocity.z) ? phd_atan(item->Animation.Velocity.z, -dy) : 0;
		if (angle < -ANGLE(20.0f))
			angle = -ANGLE(20.0f);
		else if (angle > ANGLE(20.0f))
			angle = ANGLE(20.0f);

		if (angle < item->Pose.Orientation.x - ANGLE(1.0f))
			item->Pose.Orientation.x -= ANGLE(1.0f);
		else if (angle > item->Pose.Orientation.x + ANGLE(1.0f))
			item->Pose.Orientation.x += ANGLE(1.0f);
		else
			item->Pose.Orientation.x = angle;
	}
	else if (LOT->IsJumping)
	{
		floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
		int height2 = GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
		item->Floor = height2;

		if (LOT->IsMonkeying)
		{
			ceiling = GetCeiling(floor, item->Pose.Position.x, y, item->Pose.Position.z);
			item->Pose.Position.y = ceiling - bounds->Y1;
		}
		else
		{
			if (item->Pose.Position.y > item->Floor)
			{
				if (item->Pose.Position.y > (item->Floor + CLICK(1)))
				{
					item->Pose.Position.x = old.x;
					item->Pose.Position.y = old.y;
					item->Pose.Position.z = old.z;
				}
				else
					item->Pose.Position.y = item->Floor;
			}
		}
	} 
	else
	{
		floor = GetFloor(item->Pose.Position.x, y, item->Pose.Position.z, &roomNumber);
		ceiling = GetCeiling(floor, item->Pose.Position.x, y, item->Pose.Position.z);

		if (item->ObjectNumber == ID_TYRANNOSAUR || item->ObjectNumber == ID_SHIVA || item->ObjectNumber == ID_MUTANT2)
			top = CLICK(3);
		else
			top = bounds->Y1; // TODO: check if Y1 or Y2

		if (item->Pose.Position.y + top < ceiling)
		{
			item->Pose.Position.x = old.x;
			item->Pose.Position.z = old.z;
			item->Pose.Position.y = old.y;
		}

		floor = GetFloor(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z, &roomNumber);
		item->Floor = GetFloorHeight(floor, item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);

		if (item->Pose.Position.y > item->Floor)
			item->Pose.Position.y = item->Floor;
		else if (item->Floor - item->Pose.Position.y > CLICK(0.25f))
			item->Pose.Position.y += CLICK(0.25f);
		else if (item->Pose.Position.y < item->Floor)
			item->Pose.Position.y = item->Floor;

		item->Pose.Orientation.x = 0;
	}

	CreatureSwitchRoom(itemNumber);

	return true;
}

void CreatureSwitchRoom(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	auto roomNumber = GetCollision(item->Pose.Position.x,
		item->Pose.Position.y - CLICK(2),
		item->Pose.Position.z,
		item->RoomNumber).RoomNumber;

	if (roomNumber != item->RoomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	if (!Objects[item->ObjectNumber].waterCreature &&
		TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, &g_Level.Rooms[roomNumber]))
	{
		auto bounds = GetBoundsAccurate(item);
		auto height = item->Pose.Position.y - GetWaterHeight(item);

		if (abs(bounds->Y1 + bounds->Y2) < height)
			DoDamage(item, INT_MAX);
	}
}

void CreatureDie(short itemNumber, bool explode)
{
	auto* item = &g_Level.Items[itemNumber];

	item->HitPoints = NOT_TARGETABLE;
	item->Collidable = false;

	if (explode)
	{
		if (Objects[item->ObjectNumber].hitEffect & HIT_BLOOD)
			ExplodingDeath(itemNumber, BODY_EXPLODE | BODY_GIBS);
		else if (Objects[item->ObjectNumber].hitEffect & HIT_SMOKE)
			ExplodingDeath(itemNumber, BODY_EXPLODE | BODY_NO_BOUNCE);
		else
			ExplodingDeath(itemNumber, BODY_EXPLODE);

		KillItem(itemNumber);
	}
	else
		RemoveActiveItem(itemNumber);

	DisableEntityAI(itemNumber);
	item->Flags |= IFLAG_KILLED | IFLAG_INVISIBLE;
	DropEntityPickups(item);
}

int BadFloor(int x, int y, int z, int boxHeight, int nextHeight, short roomNumber, LOTInfo* LOT)
{
	FloorInfo* floor = GetFloor(x, y, z, &roomNumber);
	if (floor->Box == NO_BOX)
		return true;

	if (LOT->IsJumping)
		return false;

	if (g_Level.Boxes[floor->Box].flags & LOT->BlockMask)
		return true;

	int height = g_Level.Boxes[floor->Box].height;
	if ((boxHeight - height) > LOT->Step || (boxHeight - height) < LOT->Drop)
		return true;

	if (boxHeight - height < -LOT->Step && height > nextHeight)
		return true;

	if (LOT->Fly != NO_FLYING && y > (height + LOT->Fly))
		return true;

	return false;
}

int CreatureCreature(short itemNumber)  
{
	auto* item = &g_Level.Items[itemNumber];
	auto* object = &Objects[item->ObjectNumber];
	int x = item->Pose.Position.x;
	int z = item->Pose.Position.z;
	short radius = object->radius;

	auto* room = &g_Level.Rooms[item->RoomNumber];
	short link = room->itemNumber;
	int distance = 0;
	do
	{
		auto* linked = &g_Level.Items[link];
		
		if (link != itemNumber && linked != LaraItem && linked->Status == ITEM_ACTIVE && linked->HitPoints > 0)
		{
			int xDistance = abs(linked->Pose.Position.x - x);
			int zDistance = abs(linked->Pose.Position.z - z);
			
			if (xDistance > zDistance)
				distance = xDistance + (zDistance >> 1);
			else
				distance = xDistance + (zDistance >> 1);

			if (distance < radius + Objects[linked->ObjectNumber].radius)
				return phd_atan(linked->Pose.Position.z - z, linked->Pose.Position.x - x) - item->Pose.Orientation.y;
		}

		link = linked->NextItem;
	} while (link != NO_ITEM);

	return 0;
}

int ValidBox(ItemInfo* item, short zoneNumber, short boxNumber) 
{
	if (boxNumber == NO_BOX)
		return false;

	auto* creature = GetCreatureInfo(item);
	int* zone = g_Level.Zones[creature->LOT.Zone][FlipStatus].data();
	if (creature->LOT.Fly == NO_FLYING && zone[boxNumber] != zoneNumber)
		return false;

	auto* box = &g_Level.Boxes[boxNumber];
	if (box->flags & creature->LOT.BlockMask)
		return false;

	if (item->Pose.Position.z > (box->left * SECTOR(1)) &&
		item->Pose.Position.z < (box->right * SECTOR(1)) &&
		item->Pose.Position.x > (box->top * SECTOR(1)) &&
		item->Pose.Position.x < (box->bottom * SECTOR(1)))
	{
		return false;
	}

	return true;
}

int EscapeBox(ItemInfo* item, ItemInfo* enemy, int boxNumber) 
{
	if (boxNumber == NO_BOX)
		return false;

	auto* box = &g_Level.Boxes[boxNumber];

	int x = (box->top + box->bottom) * SECTOR(1) / 2 - enemy->Pose.Position.x;
	int z = (box->left + box->right) * SECTOR(1) / 2 - enemy->Pose.Position.z;
	
	if (x > -ESCAPE_DIST && x < ESCAPE_DIST && z > -ESCAPE_DIST && z < ESCAPE_DIST)
		return false;

	if (((x > 0) ^ (item->Pose.Position.x > enemy->Pose.Position.x)) && ((z > 0) ^ (item->Pose.Position.z > enemy->Pose.Position.z)))
		return false;

	return true;
}

void TargetBox(LOTInfo* LOT, int boxNumber)
{
	if (boxNumber == NO_BOX)
		return;

	boxNumber &= NO_BOX;
	auto* box = &g_Level.Boxes[boxNumber];

	// Maximize target precision. DO NOT change bracket precedence!
	LOT->Target.x = (int)((box->top  * SECTOR(1)) + (float)GetRandomControl() * (((float)(box->bottom - box->top) - 1.0f) / 32.0f) + SECTOR(0.5f));
	LOT->Target.z = (int)((box->left * SECTOR(1)) + (float)GetRandomControl() * (((float)(box->right - box->left) - 1.0f) / 32.0f) + SECTOR(0.5f));
	
	LOT->RequiredBox = boxNumber;

	if (LOT->Fly == NO_FLYING)
		LOT->Target.y = box->height;
	else
		LOT->Target.y = box->height - STEPUP_HEIGHT;
}

int UpdateLOT(LOTInfo* LOT, int depth)
{
	//printf("LOT->head: %d, LOT->tail: %d\n", LOT->head, LOT->tail);

	if (LOT->RequiredBox != NO_BOX && LOT->RequiredBox != LOT->TargetBox)
	{
		LOT->TargetBox = LOT->RequiredBox;

		auto* node = &LOT->Node[LOT->TargetBox];
		if (node->nextExpansion == NO_BOX && LOT->Tail != LOT->TargetBox)
		{
			node->nextExpansion = LOT->Head;

			if (LOT->Head == NO_BOX)
				LOT->Tail = LOT->TargetBox;

			LOT->Head = LOT->TargetBox;
		}

		node->searchNumber = ++LOT->SearchNumber;
		node->exitBox = NO_BOX;
	}

	return SearchLOT(LOT, depth);
}

int SearchLOT(LOTInfo* LOT, int depth)
{
	int* zone = g_Level.Zones[LOT->Zone][FlipStatus].data();
	int searchZone = zone[LOT->Head];

	if (depth <= 0)
		return true;

	for (int i = 0; i < depth; i++)
	{
		if (LOT->Head == NO_BOX)
		{
			LOT->Tail = NO_BOX; 
			return false;
		}

		auto* node = &LOT->Node[LOT->Head];
		auto* box = &g_Level.Boxes[LOT->Head];

		int index = box->overlapIndex;
		bool done = false;
		if (index >= 0)
		{
			do
			{
				int boxNumber = g_Level.Overlaps[index].box;
				int flags = g_Level.Overlaps[index++].flags;

				if (flags & BOX_END_BIT)
					done = true;

				if (LOT->Fly == NO_FLYING && searchZone != zone[boxNumber])
					continue;

				int delta = g_Level.Boxes[boxNumber].height - box->height;
				if ((delta > LOT->Step || delta < LOT->Drop) && (!(flags & BOX_MONKEY) || !LOT->CanMonkey))
					continue;

				if ((flags & BOX_JUMP) && !LOT->CanJump)
					continue;

				auto* expand = &LOT->Node[boxNumber];
				if ((node->searchNumber & SEARCH_NUMBER) < (expand->searchNumber & SEARCH_NUMBER))
					continue;

				if (node->searchNumber & BLOCKED_SEARCH)
				{
					if ((node->searchNumber & SEARCH_NUMBER) == (expand->searchNumber & SEARCH_NUMBER))
						continue;

					expand->searchNumber = node->searchNumber;
				}
				else
				{
					if ((node->searchNumber & SEARCH_NUMBER) == (expand->searchNumber & SEARCH_NUMBER) &&
						!(expand->searchNumber & BLOCKED_SEARCH))
					{
						continue;
					}

					if (g_Level.Boxes[boxNumber].flags & LOT->BlockMask)
						expand->searchNumber = node->searchNumber | BLOCKED_SEARCH;
					else
					{
						expand->searchNumber = node->searchNumber;
						expand->exitBox = LOT->Head;
					}
				}

				if (expand->nextExpansion == NO_BOX && boxNumber != LOT->Tail)
				{
					LOT->Node[LOT->Tail].nextExpansion = boxNumber;
					LOT->Tail = boxNumber;
				}
			} while (!done);
		}

		LOT->Head = node->nextExpansion;
		node->nextExpansion = NO_BOX;
	}

	return true;
}


#if CREATURE_AI_PRIORITY_OPTIMIZATION
CreatureAIPriority GetCreatureLOTPriority(ItemInfo* item)
{
	Vector3 itemPos = Vector3(item->Pose.Position.x, item->Pose.Position.y, item->Pose.Position.z);
	Vector3 cameraPos = Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z);
	float distance = Vector3::Distance(itemPos, cameraPos);

	distance /= SECTOR(1);
	if (distance <= HIGH_PRIO_RANGE)
		return CreatureAIPriority::High;

	if (distance <= MEDIUM_PRIO_RANGE)
		return CreatureAIPriority::Medium;

	if (distance <= LOW_PRIO_RANGE)
		return CreatureAIPriority::Low;

	return CreatureAIPriority::None;
}
#endif

int CreatureActive(short itemNumber)
{
	auto* item = &g_Level.Items[itemNumber];

	if (!Objects[item->ObjectNumber].intelligent)
		return false; // Object is not a creature

	if (item->Flags & IFLAG_KILLED)
		return false; // Object is already dead

	if (item->Status == ITEM_INVISIBLE || !item->Data.is<CreatureInfo>())
	{
		if (!EnableEntityAI(itemNumber, 0))
			return false; // AI couldn't be activated

		item->Status = ITEM_ACTIVE;
	}

#ifdef CREATURE_AI_PRIORITY_OPTIMIZATION
	auto* creature = GetCreatureInfo(item);
	creature->Priority = GetCreatureLOTPriority(item);
#endif // CREATURE_AI_PRIORITY_OPTIMIZATION

	return true;
}

void InitialiseCreature(short itemNumber) 
{
	ClearItem(itemNumber);
}

int StalkBox(ItemInfo* item, ItemInfo* enemy, int boxNumber)
{
	if (boxNumber == NO_BOX)
		return false;

	if (enemy == NULL)
		return false;

	auto* box = &g_Level.Boxes[boxNumber];

	int xRange = STALK_DIST + ((box->bottom - box->top) * SECTOR(1));
	int zRange = STALK_DIST + ((box->right - box->left) * SECTOR(1));
	int x = (box->top + box->bottom) * SECTOR(1) / 2 - enemy->Pose.Position.x;
	int z = (box->left + box->right) * SECTOR(1) / 2 - enemy->Pose.Position.z;
	
	if (x > xRange || x < -xRange || z > zRange || z < -zRange)
		return false;

	int enemyQuad = (enemy->Pose.Orientation.y / ANGLE(90.0f)) + 2;
	
	int boxQuad;
	if (z > 0)
		boxQuad = (x > 0) ? 2 : 1;
	else
		boxQuad = (x > 0) ? 3 : 0;

	if (enemyQuad == boxQuad)
		return false;

	int baddyQuad = 0;
	if (item->Pose.Position.z > enemy->Pose.Position.z)
		baddyQuad = (item->Pose.Position.x > enemy->Pose.Position.x) ? 2 : 1;
	else
		baddyQuad = (item->Pose.Position.x > enemy->Pose.Position.x) ? 3 : 0;

	if (enemyQuad == baddyQuad && abs(enemyQuad - boxQuad) == 2)
		return false;

	return true;
}

int CreatureVault(short itemNumber, short angle, int vault, int shift)
{
	auto* item = &g_Level.Items[itemNumber];
	auto* creature = GetCreatureInfo(item);

	int xBlock = item->Pose.Position.x / SECTOR(1);
	int zBlock = item->Pose.Position.z / SECTOR(1);
	int y = item->Pose.Position.y;
	short roomNumber = item->RoomNumber;

	CreatureAnimation(itemNumber, angle, 0);

	if (item->Floor > y + CHECK_CLICK(9))
		vault = 0;
	else if (item->Floor > y + CHECK_CLICK(7))
		vault = -4;
	// FIXME: edit assets adding climb down animations for Von Croy and baddys?
	else if (item->Floor > y + CHECK_CLICK(5) &&
		item->ObjectNumber != ID_VON_CROY &&
		item->ObjectNumber != ID_BADDY1 &&
		item->ObjectNumber != ID_BADDY2)
	{
		vault = -3;
	}
	else if (item->Floor > y + CHECK_CLICK(3) &&
		item->ObjectNumber != ID_VON_CROY &&
		item->ObjectNumber != ID_BADDY1 &&
		item->ObjectNumber != ID_BADDY2)
	{
		vault = -2;
	}
	else if (item->Pose.Position.y > y - CHECK_CLICK(3))
		return 0;
	else if (item->Pose.Position.y > y - CHECK_CLICK(5))
		vault = 2;
	else if (item->Pose.Position.y > y - CHECK_CLICK(7))
		vault = 3;
	else
		vault = 4;

	// Jump
	int newXblock = item->Pose.Position.x / SECTOR(1);
	int newZblock = item->Pose.Position.z / SECTOR(1);

	if (zBlock == newZblock)
	{
		if (xBlock == newXblock)
			return 0;

		if (xBlock < newXblock)
		{
			item->Pose.Position.x = (newXblock * SECTOR(1)) - shift;
			item->Pose.Orientation.y = ANGLE(90.0f);
		}
		else
		{
			item->Pose.Position.x = (xBlock * SECTOR(1)) + shift;
			item->Pose.Orientation.y = -ANGLE(90.0f);
		}
	}
	else if (xBlock == newXblock)
	{
		if (zBlock < newZblock)
		{
			item->Pose.Position.z = (newZblock * SECTOR(1)) - shift;
			item->Pose.Orientation.y = 0;
		}
		else
		{
			item->Pose.Position.z = (zBlock * SECTOR(1)) + shift;
			item->Pose.Orientation.y = -ANGLE(180.0f);
		}
	}

	item->Pose.Position.y = y;
	item->Floor = y;

	if (roomNumber != item->RoomNumber)
		ItemNewRoom(itemNumber, roomNumber);

	return vault;
}

void GetAITarget(CreatureInfo* creature)
{
	auto* enemy = creature->Enemy;

	short enemyObjectNumber;
	if (enemy)
		enemyObjectNumber = enemy->ObjectNumber;
	else
		enemyObjectNumber = NO_ITEM;

	auto* item = &g_Level.Items[creature->ItemNumber];

	if (item->AIBits & GUARD)
	{
		creature->Enemy = LaraItem;
		if (creature->Alerted)
		{
			item->AIBits = ~GUARD;
			if (item->AIBits & AMBUSH)
				item->AIBits |= MODIFY;
		}
	}
	else if (item->AIBits & PATROL1)
	{
		if (creature->Alerted || creature->HurtByLara)
		{
			item->AIBits &= ~PATROL1;
			if (item->AIBits & AMBUSH)
			{
				item->AIBits |= MODIFY;
				// NOTE: added in TR5
				//item->itemFlags[3] = (creature->Tosspad & 0xFF);
			}
		}
		else if (!creature->Patrol)
		{
			if (enemyObjectNumber != ID_AI_PATROL1)
				FindAITargetObject(creature, ID_AI_PATROL1);
		}
		else if (enemyObjectNumber != ID_AI_PATROL2)
			FindAITargetObject(creature, ID_AI_PATROL2);
		else if (abs(enemy->Pose.Position.x - item->Pose.Position.x) < REACHED_GOAL_RADIUS &&
			abs(enemy->Pose.Position.y - item->Pose.Position.y) < REACHED_GOAL_RADIUS &&
			abs(enemy->Pose.Position.z - item->Pose.Position.z) < REACHED_GOAL_RADIUS ||
			Objects[item->ObjectNumber].waterCreature)
		{
			TestTriggers(enemy, true);
			ProcessSectorFlags(enemy);
			creature->Patrol = !creature->Patrol;
		}
	}
	else if (item->AIBits & AMBUSH)
	{
		// First if was removed probably after TR3 and was it used by monkeys?
		/*if (!(item->aiBits & MODIFY) && !creature->hurtByLara)
			creature->enemy = LaraItem;
		else*/ if (enemyObjectNumber != ID_AI_AMBUSH)
			FindAITargetObject(creature, ID_AI_AMBUSH);
		/*else if (item->objectNumber == ID_MONKEY)
			return;*/
		else if (abs(enemy->Pose.Position.x - item->Pose.Position.x) < REACHED_GOAL_RADIUS &&
			abs(enemy->Pose.Position.y - item->Pose.Position.y) < REACHED_GOAL_RADIUS &&
			abs(enemy->Pose.Position.z - item->Pose.Position.z) < REACHED_GOAL_RADIUS)
		{
			TestTriggers(enemy, true);
			ProcessSectorFlags(enemy);

			creature->ReachedGoal = true;
			creature->Enemy = LaraItem;
			item->AIBits &= ~(AMBUSH /* | MODIFY*/);
			if (item->AIBits != MODIFY)
			{
				item->AIBits |= GUARD;
				creature->Alerted = false;
			}
		}
	}
	else if (item->AIBits & FOLLOW)
	{
		if (creature->HurtByLara)
		{
			creature->Enemy = LaraItem;
			creature->Alerted = true;
			//item->aiBits &= ~FOLLOW;
		}
		else if (item->HitStatus)
			item->AIBits &= ~FOLLOW;
		else if (enemyObjectNumber != ID_AI_FOLLOW)
			FindAITargetObject(creature, ID_AI_FOLLOW);
		else if (abs(enemy->Pose.Position.x - item->Pose.Position.x) < REACHED_GOAL_RADIUS &&
			abs(enemy->Pose.Position.y - item->Pose.Position.y) < REACHED_GOAL_RADIUS &&
			abs(enemy->Pose.Position.z - item->Pose.Position.z) < REACHED_GOAL_RADIUS)
		{
			creature->ReachedGoal = true;
			item->AIBits &= ~FOLLOW;
		}
	}
	/*else if (item->objectNumber == ID_MONKEY && item->carriedItem == NO_ITEM)
	{
		if (item->aiBits != MODIFY)
		{
			if (enemyObjectNumber != ID_SMALLMEDI_ITEM)
				FindAITargetObject(creature, ID_SMALLMEDI_ITEM);
		}
		else
		{
			if (enemyObjectNumber != ID_KEY_ITEM4)
				FindAITargetObject(creature, ID_KEY_ITEM4);
		}
	}*/
}

// TR3 old way..
void FindAITarget(CreatureInfo* creature, short objectNumber)
{
	auto* item = &g_Level.Items[creature->ItemNumber];
	ItemInfo* targetItem;

	int i;
	for (i = 0, targetItem = &g_Level.Items[0]; i < g_Level.NumItems; i++, targetItem++)
	{
		if (targetItem->ObjectNumber == objectNumber && targetItem->RoomNumber != NO_ROOM)
		{
			if (SameZone(creature, targetItem) && targetItem->Pose.Orientation.y == item->ItemFlags[3])
			{
				creature->Enemy = targetItem;
				break;
			}
		}
	}
}

void FindAITargetObject(CreatureInfo* creature, short objectNumber)
{
	auto* item = &g_Level.Items[creature->ItemNumber];

	if (g_Level.AIObjects.size() > 0)
	{
		AI_OBJECT* foundObject = NULL;

		for (int i = 0; i < g_Level.AIObjects.size(); i++)
		{
			auto* aiObject = &g_Level.AIObjects[i];

			if (aiObject->objectNumber == objectNumber && aiObject->triggerFlags == item->ItemFlags[3] && aiObject->roomNumber != NO_ROOM)
			{
				int* zone = g_Level.Zones[creature->LOT.Zone][FlipStatus].data();

				auto* room = &g_Level.Rooms[item->RoomNumber];
				item->BoxNumber = GetSector(room, item->Pose.Position.x - room->x, item->Pose.Position.z - room->z)->Box;

				room = &g_Level.Rooms[aiObject->roomNumber];
				aiObject->boxNumber = GetSector(room, aiObject->x - room->x, aiObject->z - room->z)->Box;

				if (item->BoxNumber == NO_BOX || aiObject->boxNumber == NO_BOX)
					return;

				if (zone[item->BoxNumber] == zone[aiObject->boxNumber])
				{
					foundObject = aiObject;
					break;
				}
			}
		}

		if (foundObject != NULL)
		{
			auto* aiItem = creature->AITarget;

			creature->Enemy = aiItem;

			aiItem->ObjectNumber = foundObject->objectNumber;
			aiItem->RoomNumber = foundObject->roomNumber;
			aiItem->Pose.Position.x = foundObject->x;
			aiItem->Pose.Position.y = foundObject->y;
			aiItem->Pose.Position.z = foundObject->z;
			aiItem->Pose.Orientation.y = foundObject->yRot;
			aiItem->Flags = foundObject->flags;
			aiItem->TriggerFlags = foundObject->triggerFlags;
			aiItem->BoxNumber = foundObject->boxNumber;

			if (!(creature->AITarget->Flags & 32))
			{
				creature->AITarget->Pose.Position.x += phd_sin(creature->AITarget->Pose.Orientation.y) * 256;
				creature->AITarget->Pose.Position.z += phd_cos(creature->AITarget->Pose.Orientation.y) * 256;
			}
		}
	}
}

void CreatureAIInfo(ItemInfo* item, AI_INFO* AI)
{
	if (!item->Data)
		return;

	auto* creature = GetCreatureInfo(item);
	auto* object = &Objects[item->ObjectNumber];

	// TODO: Deal with LaraItem global.
	auto* enemy = creature->Enemy;
	if (enemy == nullptr)
	{
		enemy = LaraItem;
		creature->Enemy = LaraItem;
	}

	int* zone = g_Level.Zones[creature->LOT.Zone][FlipStatus].data();

	auto* room = &g_Level.Rooms[item->RoomNumber];
	item->BoxNumber = NO_BOX;
	FloorInfo* floor = GetSector(room, item->Pose.Position.x - room->x, item->Pose.Position.z - room->z);
	if(floor)
		item->BoxNumber = floor->Box;

	if (item->BoxNumber != NO_BOX)
		AI->zoneNumber = zone[item->BoxNumber];
	else
		AI->zoneNumber = NO_ZONE;

	enemy->BoxNumber = NO_BOX;

	room = &g_Level.Rooms[enemy->RoomNumber];
	floor = GetSector(room, enemy->Pose.Position.x - room->x, enemy->Pose.Position.z - room->z);

	// NEW: Only update enemy box number if will be actually reachable by enemy.
	// This prevents enemies from running to Lara and attacking nothing when she is hanging or shimmying. -- Lwmte, 27.06.22

	bool reachable = false;
	if (object->zoneType == ZoneType::ZONE_FLYER ||
	   (object->zoneType == ZoneType::ZONE_WATER && TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, item->RoomNumber)))
	{
		reachable = true; // If NPC is flying or swimming in water, always reach Lara
	}
	else
	{
		auto probe = GetCollision(floor, enemy->Pose.Position.x, enemy->Pose.Position.y, enemy->Pose.Position.z);
		auto bounds = GetBoundsAccurate(item);

		reachable = abs(enemy->Pose.Position.y - probe.Position.Floor) < bounds->Height();
	}

	if (floor && reachable)
		enemy->BoxNumber = floor->Box;

	if (enemy->BoxNumber != NO_BOX)
		AI->enemyZone = zone[enemy->BoxNumber];
	else
		AI->enemyZone = NO_ZONE;

	if (!object->nonLot)
	{
		if (enemy->BoxNumber != NO_BOX && g_Level.Boxes[enemy->BoxNumber].flags & creature->LOT.BlockMask)
			AI->enemyZone |= BLOCKED;
		else if (item->BoxNumber != NO_BOX && creature->LOT.Node[item->BoxNumber].searchNumber == (creature->LOT.SearchNumber | BLOCKED_SEARCH))
			AI->enemyZone |= BLOCKED;
	}

	Vector3Int vector;

	if (enemy == LaraItem)
	{
		vector.x = enemy->Pose.Position.x + enemy->Animation.Velocity.z * PREDICTIVE_SCALE_FACTOR * phd_sin(Lara.Control.MoveAngle) - item->Pose.Position.x - object->pivotLength * phd_sin(item->Pose.Orientation.y);
		vector.z = enemy->Pose.Position.z + enemy->Animation.Velocity.z * PREDICTIVE_SCALE_FACTOR * phd_cos(Lara.Control.MoveAngle) - item->Pose.Position.z - object->pivotLength * phd_cos(item->Pose.Orientation.y);
	}
	else
	{
		vector.x = enemy->Pose.Position.x + enemy->Animation.Velocity.z * PREDICTIVE_SCALE_FACTOR * phd_sin(enemy->Pose.Orientation.y) - item->Pose.Position.x - object->pivotLength * phd_sin(item->Pose.Orientation.y);
		vector.z = enemy->Pose.Position.z + enemy->Animation.Velocity.z * PREDICTIVE_SCALE_FACTOR * phd_cos(enemy->Pose.Orientation.y) - item->Pose.Position.z - object->pivotLength * phd_cos(item->Pose.Orientation.y);
	}

	vector.y = item->Pose.Position.y - enemy->Pose.Position.y;
	short angle = phd_atan(vector.z, vector.x);

	if (vector.x > SECTOR(31.25f) || vector.x < -SECTOR(31.25f) || vector.z > SECTOR(31.25f) || vector.z < -SECTOR(31.25f))
		AI->distance = AI->verticalDistance = INT_MAX;
	else
	{
		if (creature->Enemy)
		{
			// TODO: distance is squared, verticalDistance is not. Desquare distance later. -- Lwmte, 27.06.22

			AI->distance = pow(vector.x, 2) + pow(vector.z, 2);
			AI->verticalDistance = abs(vector.y);
		}
		else
			AI->distance = AI->verticalDistance = INT_MAX;
	}

	AI->angle = angle - item->Pose.Orientation.y;
	AI->enemyFacing = ANGLE(180.0f) + angle - enemy->Pose.Orientation.y;

	vector.x = abs(vector.x);
	vector.z = abs(vector.z);

	// Makes Lara smaller.
	if (enemy->IsLara())
	{
		if (GetLaraInfo(enemy)->Control.IsLow)
			vector.y -= STEPUP_HEIGHT;
	}

	if (vector.x > vector.z)
		AI->xAngle = phd_atan(vector.x + (vector.z >> 1), vector.y);
	else
		AI->xAngle = phd_atan(vector.z + (vector.x >> 1), vector.y);

	AI->ahead = (AI->angle > -FRONT_ARC && AI->angle < FRONT_ARC);
	AI->bite = (AI->ahead && enemy->HitPoints > 0 && abs(enemy->Pose.Position.y - item->Pose.Position.y) <= CLICK(2));
}

void CreatureMood(ItemInfo* item, AI_INFO* AI, bool isViolent)
{
	if (!item->Data)
		return;

	auto* creature = GetCreatureInfo(item);
	auto* enemy = creature->Enemy;
	auto* LOT = &creature->LOT;

	int boxNumber;

	if (enemy != nullptr)
	{
		switch (creature->Mood)
		{
		case MoodType::Bored:
			boxNumber = LOT->Node[GetRandomControl() * LOT->ZoneCount >> 15].boxNumber;
			if (ValidBox(item, AI->zoneNumber, boxNumber))
			{
				if (StalkBox(item, enemy, boxNumber) && enemy->HitPoints > 0 && creature->Enemy)
				{
					TargetBox(LOT, boxNumber);
					creature->Mood = MoodType::Bored;
				}
				else if (LOT->RequiredBox == NO_BOX)
					TargetBox(LOT, boxNumber);
			}

			break;

		case MoodType::Attack:
			LOT->Target = enemy->Pose.Position;
			LOT->RequiredBox = enemy->BoxNumber;

			if (LOT->Fly != NO_FLYING && Lara.Control.WaterStatus == WaterStatus::Dry)
			{
				auto* bounds = (BOUNDING_BOX*)GetBestFrame(enemy);
				LOT->Target.y += bounds->Y1;
			}

			break;

		case MoodType::Escape:
			boxNumber = LOT->Node[GetRandomControl() * LOT->ZoneCount >> 15].boxNumber;
			if (ValidBox(item, AI->zoneNumber, boxNumber))
			{
				if (EscapeBox(item, enemy, boxNumber))
					TargetBox(LOT, boxNumber);
				else if (AI->zoneNumber == AI->enemyZone && StalkBox(item, enemy, boxNumber) && !isViolent)
				{
					TargetBox(LOT, boxNumber);
					creature->Mood = MoodType::Stalk;
				}
			}

			break;

		case MoodType::Stalk:
			if (LOT->RequiredBox == NO_BOX || !StalkBox(item, enemy, LOT->RequiredBox))
			{
				boxNumber = LOT->Node[GetRandomControl() * LOT->ZoneCount >> 15].boxNumber;
				if (ValidBox(item, AI->zoneNumber, boxNumber))
				{
					if (StalkBox(item, enemy, boxNumber))
						TargetBox(LOT, boxNumber);
					else if (LOT->RequiredBox == NO_BOX)
					{
						TargetBox(LOT, boxNumber);
						if (AI->zoneNumber != AI->enemyZone)
							creature->Mood = MoodType::Bored;
					}
				}
			}

			break;
		}
	}

	if (LOT->TargetBox == NO_BOX)
		TargetBox(LOT, item->BoxNumber);

#ifdef CREATURE_AI_PRIORITY_OPTIMIZATION
	bool shouldUpdateTarget = false;

	switch(creature->Priority)
	{
		case CreatureAIPriority::High:
			shouldUpdateTarget = true;
			break;

		case CreatureAIPriority::Medium:
			if (creature->FramesSinceLOTUpdate > std::pow(FRAME_PRIO_BASE, FRAME_PRIO_EXP))
				shouldUpdateTarget = true;

			break;

		case CreatureAIPriority::Low:
			if (creature->FramesSinceLOTUpdate > std::pow(FRAME_PRIO_BASE, FRAME_PRIO_EXP * 2))
				shouldUpdateTarget = true;

			break;

		default:
			break;
	}

	if (shouldUpdateTarget)
	{
		CalculateTarget(&creature->Target, item, &creature->LOT);
		creature->FramesSinceLOTUpdate = 0;
	}
	else
		creature->FramesSinceLOTUpdate++;
#else
	CalculateTarget(&creature->Target, item, &creature->LOT);
#endif // CREATURE_AI_PRIORITY_OPTIMIZATION

	creature->JumpAhead = false;
	creature->MonkeySwingAhead = false;

	if (item->BoxNumber != NO_BOX)
	{
		int startBox = LOT->Node[item->BoxNumber].exitBox;
		if (startBox != NO_BOX)
		{
			int overlapIndex = g_Level.Boxes[item->BoxNumber].overlapIndex;
			int nextBox = 0;
			int flags = 0;

			if (overlapIndex >= 0)
			{
				do
				{
					nextBox = g_Level.Overlaps[overlapIndex].box;
					flags = g_Level.Overlaps[overlapIndex++].flags;
				} while (nextBox != NO_BOX && ((flags & BOX_END_BIT) == false) && (nextBox != startBox));
			}

			if (nextBox == startBox)
			{
				if (flags & BOX_JUMP)
					creature->JumpAhead = true;

				if (flags & BOX_MONKEY)
					creature->MonkeySwingAhead = true;
			}
		}
	}
}

void GetCreatureMood(ItemInfo* item, AI_INFO* AI, bool isViolent)
{
	if (!item->Data)
		return;

	auto* creature = GetCreatureInfo(item);
	auto* enemy = creature->Enemy;
	auto* LOT = &creature->LOT;

	if (item->BoxNumber == NO_BOX || creature->LOT.Node[item->BoxNumber].searchNumber == (creature->LOT.SearchNumber | BLOCKED_SEARCH))
		creature->LOT.RequiredBox = NO_BOX;

	if (creature->Mood != MoodType::Attack &&
		creature->LOT.RequiredBox != NO_BOX)
	{
		if (!ValidBox(item, AI->zoneNumber, creature->LOT.TargetBox))
		{
			if (AI->zoneNumber == AI->enemyZone)
				creature->Mood = MoodType::Bored;

			creature->LOT.RequiredBox = NO_BOX;
		}
	}

	auto mood = creature->Mood;
	if (!enemy)
	{
		creature->Mood = MoodType::Bored;
		enemy = LaraItem;
	}
	else if (enemy->HitPoints <= 0 && enemy == LaraItem)
		creature->Mood = MoodType::Bored;
	else if (isViolent)
	{
		switch (creature->Mood)
		{
			case MoodType::Bored:
			case MoodType::Stalk:
				if (AI->zoneNumber == AI->enemyZone)
					creature->Mood = MoodType::Attack;
				else if (item->HitStatus)
					creature->Mood = MoodType::Escape;

				break;

			case MoodType::Attack:
				if (AI->zoneNumber != AI->enemyZone || enemy->BoxNumber == NO_BOX)
					creature->Mood = MoodType::Bored;

				break;

			case MoodType::Escape:
				if (AI->zoneNumber == AI->enemyZone)
					creature->Mood = MoodType::Attack;

				break;
		}
	}
	else
	{
		switch (creature->Mood)
		{
			case MoodType::Bored:
			case MoodType::Stalk:
				if (creature->Alerted &&
					AI->zoneNumber != AI->enemyZone)
				{
					if (AI->distance > SECTOR(3))
						creature->Mood = MoodType::Stalk;
					else
						creature->Mood = MoodType::Bored;
				}
				else if (AI->zoneNumber == AI->enemyZone)
				{
					if (AI->distance < ATTACK_RANGE ||
						(creature->Mood == MoodType::Stalk && LOT->RequiredBox == NO_BOX))
						creature->Mood = MoodType::Attack;
					else
						creature->Mood = MoodType::Stalk;
				}
				break;

			case MoodType::Attack:
				if (item->HitStatus &&
					(GetRandomControl() < ESCAPE_CHANCE ||
						AI->zoneNumber != AI->enemyZone))
					creature->Mood = MoodType::Stalk;
				else if (AI->zoneNumber != AI->enemyZone && AI->distance > SECTOR(6))
					creature->Mood = MoodType::Bored;

				break;

			case MoodType::Escape:
				if (AI->zoneNumber == AI->enemyZone &&
					GetRandomControl() < RECOVER_CHANCE)
					creature->Mood = MoodType::Stalk;

				break;
		}
	}

	if (mood != creature->Mood)
	{
		if (mood == MoodType::Attack)
			TargetBox(LOT, LOT->TargetBox);

		LOT->RequiredBox = NO_BOX;
	}
}

TARGET_TYPE CalculateTarget(Vector3Int* target, ItemInfo* item, LOTInfo* LOT)
{
	UpdateLOT(LOT, 5);

	target->x = item->Pose.Position.x;
	target->y = item->Pose.Position.y;
	target->z = item->Pose.Position.z;

	int boxNumber = item->BoxNumber;
	if (boxNumber == NO_BOX)
		return TARGET_TYPE::NO_TARGET;

	auto* box = &g_Level.Boxes[boxNumber];
	int boxLeft = ((int)box->left * SECTOR(1));
	int boxRight = ((int)box->right * SECTOR(1)) - 1;
	int boxTop = ((int)box->top * SECTOR(1));
	int boxBottom = ((int)box->bottom * SECTOR(1)) - 1;
	int left = boxLeft;
	int right = boxRight;
	int top = boxTop;
	int bottom = boxBottom;
	int direction = ALL_CLIP;

	do
	{
		box = &g_Level.Boxes[boxNumber];

		if (LOT->Fly == NO_FLYING)
		{
			if (target->y > box->height)
				target->y = box->height;
		}
		else
		{
			if (target->y > box->height - SECTOR(1))
				target->y = box->height - SECTOR(1);
		}

		boxLeft = ((int)box->left * SECTOR(1));
		boxRight = ((int)box->right * SECTOR(1)) - 1;
		boxTop = ((int)box->top * SECTOR(1));
		boxBottom = ((int)box->bottom * SECTOR(1)) - 1;

		if (item->Pose.Position.z >= boxLeft &&
			item->Pose.Position.z <= boxRight &&
			item->Pose.Position.x >= boxTop &&
			item->Pose.Position.x <= boxBottom)
		{
			left = boxLeft;
			right = boxRight;
			top = boxTop;
			bottom = boxBottom;
		}
		else
		{
			if (item->Pose.Position.z < boxLeft)
			{
				if ((direction & CLIP_LEFT) &&
					item->Pose.Position.x >= boxTop &&
					item->Pose.Position.x <= boxBottom)
				{
					if (target->z < (boxLeft + CLICK(2)))
						target->z = boxLeft + CLICK(2);

					if (direction & SECONDARY_CLIP)
						return TARGET_TYPE::SECONDARY_TARGET;

					if (boxTop > top)
						top = boxTop;

					if (boxBottom < bottom)
						bottom = boxBottom;

					direction = CLIP_LEFT;
				}
				else if (direction != CLIP_LEFT)
				{
					target->z = (right - CLICK(2));
					if (direction != ALL_CLIP)
						return TARGET_TYPE::SECONDARY_TARGET;

					direction |= SECONDARY_CLIP;
				}
			}
			else if (item->Pose.Position.z > boxRight)
			{
				if ((direction & CLIP_RIGHT) &&
					item->Pose.Position.x >= boxTop &&
					item->Pose.Position.x <= boxBottom)
				{
					if (target->z > boxRight - CLICK(2))
						target->z = boxRight - CLICK(2);

					if (direction & SECONDARY_CLIP)
						return TARGET_TYPE::SECONDARY_TARGET;

					if (boxTop > top)
						top = boxTop;

					if (boxBottom < bottom)
						bottom = boxBottom;

					direction = CLIP_RIGHT;
				}
				else if (direction != CLIP_RIGHT)
				{
					target->z = left + CLICK(2);
					if (direction != ALL_CLIP)
						return TARGET_TYPE::SECONDARY_TARGET;

					direction |= SECONDARY_CLIP;
				}
			}

			if (item->Pose.Position.x < boxTop)
			{
				if ((direction & CLIP_TOP) &&
					item->Pose.Position.z >= boxLeft &&
					item->Pose.Position.z <= boxRight)
				{
					if (target->x < boxTop + CLICK(2))
						target->x = boxTop + CLICK(2);

					if (direction & SECONDARY_CLIP)
						return TARGET_TYPE::SECONDARY_TARGET;

					if (boxLeft > left)
						left = boxLeft;

					if (boxRight < right)
						right = boxRight;

					direction = CLIP_TOP;
				}
				else if (direction != CLIP_TOP)
				{
					target->x = bottom - CLICK(2);
					if (direction != ALL_CLIP)
						return TARGET_TYPE::SECONDARY_TARGET;

					direction |= SECONDARY_CLIP;
				}
			}
			else if (item->Pose.Position.x > boxBottom)
			{
				if ((direction & CLIP_BOTTOM) &&
					item->Pose.Position.z >= boxLeft &&
					item->Pose.Position.z <= boxRight)
				{
					if (target->x > (boxBottom - CLICK(2)))
						target->x = boxBottom - CLICK(2);

					if (direction & SECONDARY_CLIP)
						return TARGET_TYPE::SECONDARY_TARGET;

					if (boxLeft > left)
						left = boxLeft;

					if (boxRight < right)
						right = boxRight;

					direction = CLIP_BOTTOM;
				}
				else if (direction != CLIP_BOTTOM)
				{
					target->x = top + CLICK(2);
					if (direction != ALL_CLIP)
						return TARGET_TYPE::SECONDARY_TARGET;

					direction |= SECONDARY_CLIP;
				}
			}
		}

		if (boxNumber == LOT->TargetBox)
		{
			if (direction & (CLIP_LEFT | CLIP_RIGHT))
				target->z = LOT->Target.z;
			else if (!(direction & SECONDARY_CLIP))
			{
				if (target->z < (boxLeft + CLICK(2)))
					target->z = boxLeft + CLICK(2);
				else if (target->z > (boxRight - CLICK(2)))
					target->z = boxRight - CLICK(2);
			}

			if (direction & (CLIP_TOP | CLIP_BOTTOM))
				target->x = LOT->Target.x;
			else if (!(direction & SECONDARY_CLIP))
			{
				if (target->x < (boxTop + CLICK(2)))
					target->x = boxTop + CLICK(2);
				else if (target->x > (boxBottom - CLICK(2)))
					target->x = boxBottom - CLICK(2);
			}

			target->y = LOT->Target.y;
			return TARGET_TYPE::PRIME_TARGET;
		}

		boxNumber = LOT->Node[boxNumber].exitBox;
		if (boxNumber != NO_BOX && (g_Level.Boxes[boxNumber].flags & LOT->BlockMask))
			break;
	} while (boxNumber != NO_BOX);

	if (direction & (CLIP_LEFT | CLIP_RIGHT))
		target->z = boxLeft + SECTOR(0.5f) + (GetRandomControl() * (boxRight - boxLeft - SECTOR(1)) >> 15);
	else if (!(direction & SECONDARY_CLIP))
	{
		if (target->z < (boxLeft + CLICK(2)))
			target->z = boxLeft + CLICK(2);
		else if (target->z > (boxRight - CLICK(2)))
			target->z = boxRight - CLICK(2);
	}

	if (direction & (CLIP_TOP | CLIP_BOTTOM))
		target->x = boxTop + SECTOR(0.5f) + (GetRandomControl() * (boxBottom - boxTop - SECTOR(1)) >> 15);
	else if (!(direction & SECONDARY_CLIP))
	{
		if (target->x < (boxTop + CLICK(2)))
			target->x = boxTop + CLICK(2);
		else if (target->x > (boxBottom - CLICK(2)))
			target->x = boxBottom - CLICK(2);
	}

	if (LOT->Fly == NO_FLYING)
		target->y = box->height;
	else
		target->y = box->height - STEPUP_HEIGHT;

	return TARGET_TYPE::NO_TARGET;
}

void AdjustStopperFlag(ItemInfo* item, int direction, bool set)
{
	int x = item->Pose.Position.x;
	int z = item->Pose.Position.z;

	auto* room = &g_Level.Rooms[item->RoomNumber];

	FloorInfo* floor = GetSector(room, x - room->x, z - room->z);
	floor->Stopper = set;

	x = item->Pose.Position.x + SECTOR(1) * phd_sin(direction);
	z = item->Pose.Position.z + SECTOR(1) * phd_cos(direction);
	room = &g_Level.Rooms[GetCollision(x, item->Pose.Position.y, z, item->RoomNumber).RoomNumber];

	floor = GetSector(room, x - room->x, z - room->z);
	floor->Stopper = set;
}

void InitialiseItemBoxData()
{
	for (int i = 0; i < g_Level.Items.size(); i++)
	{
		auto* currentItem = &g_Level.Items[i];

		if (currentItem->Active && currentItem->Data.is<PushableInfo>())
			ClearMovableBlockSplitters(currentItem->Pose.Position.x, currentItem->Pose.Position.y, currentItem->Pose.Position.z, currentItem->RoomNumber);
	}

	for (auto& room : g_Level.Rooms)
	{
		for (const auto& mesh : room.mesh)
		{
			long index = ((mesh.pos.Position.z - room.z) / SECTOR(1)) + room.zSize * ((mesh.pos.Position.x - room.x) / SECTOR(1));

			if (index > room.floor.size())
				continue;

			FloorInfo* floor = &room.floor[index];

			if (floor->Box == NO_BOX)
				continue;

			if (!(g_Level.Boxes[floor->Box].flags & BLOCKED))
			{
				int floorHeight = floor->FloorHeight(mesh.pos.Position.x, mesh.pos.Position.z);
				auto* staticInfo = &StaticObjects[mesh.staticNumber];

				if (floorHeight <= mesh.pos.Position.y - staticInfo->collisionBox.Y2 + CLICK(2) &&
					floorHeight < mesh.pos.Position.y - staticInfo->collisionBox.Y1)
				{
					if (staticInfo->collisionBox.X1 == 0 || staticInfo->collisionBox.X2 == 0 ||
						staticInfo->collisionBox.Z1 == 0 || staticInfo->collisionBox.Z2 == 0 ||
						((staticInfo->collisionBox.X1 < 0) ^ (staticInfo->collisionBox.X2 < 0)) &&
						((staticInfo->collisionBox.Z1 < 0) ^ (staticInfo->collisionBox.Z2 < 0)))
					{
						floor->Stopper = true;
					}
				}
			}
		}
	}
}
