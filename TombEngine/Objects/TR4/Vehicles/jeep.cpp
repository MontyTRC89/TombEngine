#include "framework.h"
#include "Objects/TR4/Vehicles/jeep.h"
#include "Game/animation.h"
#include "Game/Lara/lara.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/gui.h"
#include "Game/effects/effects.h"
#include "Game/collision/collide_item.h"
#include "Game/Lara/lara_one_gun.h"
#include "Game/items.h"
#include "Game/camera.h"
#include "Game/effects/tomb4fx.h"
#include "Game/Lara/lara_flare.h"
#include "Game/effects/simple_particle.h"
#include "Specific/input.h"
#include "Specific/setup.h"
#include "Specific/level.h"
#include "Sound/sound.h"
#include "Objects/TR4/Vehicles/jeep_info.h"
#include "Renderer/Renderer11Enums.h"
#include "Specific/prng.h"

using namespace TEN::Input;
using std::vector;

namespace TEN::Entities::Vehicles
{
	//bool QuadHandbrakeStarting;
	//bool QuadCanHandbrakeStart;
	char JeepSmokeStart;
	bool JeepNoGetOff;
	short Unk_0080DE1A;
	int Unk_0080DDE8;
	short Unk_0080DE24;

	const vector<int> JeepMeshJoints = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 16 };

	#define JEEP_DISMOUNT_DISTANCE		512
	#define JEEP_UNDO_TURN				91
	#define	JEEP_FRONT					550
	#define JEEP_SIDE					256
	#define JEEP_SLIP					100
	#define JEEP_SLIP_SIDE				128
	#define JEEP_MAX_SPEED				0x8000
	#define JEEP_MAX_BACK				0x4000

	#define JEEP_IN_ACCELERATE	IN_ACTION
	#define JEEP_IN_BRAKE		IN_JUMP

	enum JeepState
	{
		JS_STOP = 0,
		JS_DRIVE_FORWARD = 1,
		//2 3 4 5 are the collide with walls states
		JS_BRAKE = 6,//?
		JS_FWD_LEFT = 7,//FWD = forwards
		JS_FWD_RIGHT = 8,
		JS_GETIN = 9,
		JS_GETOFF = 10,
		JS_JUMP = 11,
		JS_LAND = 12,
		JS_BACK = 13,//when she's turning back, and when idle back.. hmm
		JS_BACK_LEFT = 14,
		JS_BACK_RIGHT = 15,
		JS_DEATH = 16,
		JS_DRIVE_BACK = 17
	};

	enum JeepAnim
	{
		JA_DEATH = 0,
		JA_BRAKE = 1,
		JA_DRIVE_FORWARD = 2,
		JA_FWD_LEFT_START = 3,//FWD = forwards
		JA_FWD_LEFT = 4,
		JA_FWD_LEFT_END = 5,
		JA_FWD_JUMP_START = 6,
		JA_JUMP = 7,
		JA_FWD_JUMP_LAND = 8,
		JA_GETIN_RIGHT = 9,
		//10 11 12 13 are the collide with walls anims
		JA_IDLE = 14,
		JA_FWD_RIGHT_START = 15,
		JA_FWD_RIGHT = 16,
		JA_FWD_RIGHT_END = 17,
		JA_GETIN_LEFT = 18,
		JA_GETOFF = 19,
		JA_BACK_JUMP_START = 20,
		JA_BACK_JUMP = 21,
		JA_BACK_JUMP_LAND = 22,
		JA_REVERSE_START = 23,
		JA_REVERSE = 24,
		JA_REVERSE_END = 25, //aka turn head back forwards
		JA_BACK_RIGHT_START = 26,
		JA_BACK_RIGHT = 27,
		JA_BACK_RIGHT_END = 28,
		JA_BACK_LEFT_START = 29,
		JA_BACK_LEFT = 30,
		JA_BACK_LEFT_END = 31,
		JA_IDLE_RIGHT_START = 32,//turn steering whel right while idle
		JA_IDLE_LEFT_START = 33,// blah blah left while idle
		JA_IDLE_RIGHT_END = 34,//turn steering wheel straight from right while idle
		JA_IDLE_LEFT_END = 35,//blah blah straight from left while idle
		JA_IDLE_RIGHT_BACK_START = 36,//same as 32 but in reverse
		JA_IDLE_LEFT_BACK_START = 37,//same as 33 but in reverse
		JA_IDLE_BACK_RIGHT_END = 38,//same as 34 but in reverse
		JA_IDLE_BACK_LEFT_END = 39,//same as 35 but in reverse
		JA_IDLE_REVERSE_RIGHT = 40,//"change to reverse gear with the wheels still turned left"
		JA_IDLE_REVERSE_LEFT = 41,//"change to reverse gear with the wheels still turned right"
		JA_BACK_IDLE = 42,
		JA_IDLE_FWD_LEFT = 43,//aka the opposite of 41. "change to forward gear with the wheels still turned left"
		JA_IDLE_FWD_RIGHT = 44,//aka the opposite of 42. "change to forward gear with the wheels still turned right"
	};

	enum JeepFlags
	{
		JEEP_FLAG_FALLING = (1 << 6),
		JEEP_FLAG_DEAD = (1 << 7)
	};

	static JeepInfo* GetJeepInfo(ItemInfo* jeepItem)
	{
		return (JeepInfo*)jeepItem->Data;
	}

	void InitialiseJeep(short itemNumber)
	{
		ItemInfo* jeepItem = &g_Level.Items[itemNumber];
		jeepItem->Data = JeepInfo();
		auto* jeep = GetJeepInfo(jeepItem);

		jeepItem->SetBits(JointBitType::Mesh, JeepMeshJoints);
		jeep->velocity = 0;
		jeep->revs = 0;
		jeep->jeepTurn = 0;
		jeep->fallSpeed = 0;
		jeep->extraRotation = 0;
		jeep->momentumAngle = jeepItem->Pose.Position.y;
		jeep->pitch = 0;
		jeep->flags = 0;
		jeep->unknown2 = 0;
		jeep->rot1 = 0;
		jeep->rot2 = 0;
		jeep->rot3 = 0;
		jeep->rot4 = 0;
	}

	static int TestJeepHeight(ItemInfo* jeepItem, int dz, int dx, Vector3Int* pos)
	{
		float sinX = phd_sin(jeepItem->Pose.Orientation.x);
		float sinY = phd_sin(jeepItem->Pose.Orientation.y);
		float cosY = phd_cos(jeepItem->Pose.Orientation.y);
		float sinZ = phd_sin(jeepItem->Pose.Orientation.z);

		pos->x = jeepItem->Pose.Position.x + (dz * sinY) + (dx * cosY);
		pos->y = jeepItem->Pose.Position.y - (dz * sinX) + (dx * sinZ);
		pos->z = jeepItem->Pose.Position.z + (dz * cosY) - (dx * sinY);

		auto probe = GetCollision(pos->x, pos->y, pos->z, jeepItem->RoomNumber);

		if (pos->y < probe.Position.Ceiling || probe.Position.Ceiling == NO_HEIGHT)
			return NO_HEIGHT;

		return probe.Position.Floor;
	}

	static int DoJeepShift(ItemInfo* jeepItem, Vector3Int* pos, Vector3Int* old)
	{
		int x = pos->x / SECTOR(1);
		int z = pos->z / SECTOR(1);
		int  oldX = old->x / SECTOR(1);
		int oldZ = old->z / SECTOR(1);
		int shiftX = pos->x & (WALL_SIZE - 1);
		int shiftZ = pos->z & (WALL_SIZE - 1);

		if (x == oldX)
		{
			if (z == oldZ)
			{
				jeepItem->Pose.Position.z += (old->z - pos->z);
				jeepItem->Pose.Position.x += (old->x - pos->x);
			}
			else if (z > oldZ)
			{
				jeepItem->Pose.Position.z -= shiftZ + 1;
				return (pos->x - jeepItem->Pose.Position.x);
			}
			else
			{
				jeepItem->Pose.Position.z += WALL_SIZE - shiftZ;
				return (jeepItem->Pose.Position.x - pos->x);
			}
		}
		else if (z == oldZ)
		{
			if (x > oldX)
			{
				jeepItem->Pose.Position.x -= shiftX + 1;
				return (jeepItem->Pose.Position.z - pos->z);
			}
			else
			{
				jeepItem->Pose.Position.x += WALL_SIZE - shiftX;
				return (pos->z - jeepItem->Pose.Position.z);
			}
		}
		else
		{
			x = 0;
			z = 0;

			short roomNumber = jeepItem->RoomNumber;
			FloorInfo* floor = GetFloor(old->x, pos->y, pos->z, &roomNumber);
			int height = GetFloorHeight(floor, old->x, pos->y, pos->z);
			if (height < old->y - STEP_SIZE)
			{
				if (pos->z > old->z)
					z = -shiftZ - 1;
				else
					z = WALL_SIZE - shiftZ;
			}

			roomNumber = jeepItem->RoomNumber;
			floor = GetFloor(pos->x, pos->y, old->z, &roomNumber);
			height = GetFloorHeight(floor, pos->x, pos->y, old->z);
			if (height < old->y - STEP_SIZE)
			{
				if (pos->x > old->x)
					x = -shiftX - 1;
				else
					x = WALL_SIZE - shiftX;
			}

			if (x && z)
			{
				jeepItem->Pose.Position.z += z;
				jeepItem->Pose.Position.x += x;
			}
			else if (z)
			{
				jeepItem->Pose.Position.z += z;
				if (z > 0)
					return (jeepItem->Pose.Position.x - pos->x);
				else
					return (pos->x - jeepItem->Pose.Position.x);
			}
			else if (x)
			{
				jeepItem->Pose.Position.x += x;
				if (x > 0)
					return (pos->z - jeepItem->Pose.Position.z);
				else
					return (jeepItem->Pose.Position.z - pos->z);
			}
			else
			{
				jeepItem->Pose.Position.z += (old->z - pos->z);
				jeepItem->Pose.Position.x += (old->x - pos->x);
			}
		}

		return 0;
	}

	static int DoJeepDynamics(ItemInfo* laraItem, int height, int speed, int* y, int flags)
	{
		int result = 0;

		// Grounded.
		if (height <= *y)
		{
			if (flags)
				result = speed;
			else
			{
				int temp = height - *y;

				if (temp < -80)
					temp = -80;

				result = ((temp - speed) / 16) + speed;

				if (*y > height)
					*y = height;
			}
		}
		// Airborne.
		else
		{
			*y += speed;
			if (*y <= height - 32)
			{
				if (flags)
					result = flags + (flags / 2) + speed;
				else
					result = speed + GRAVITY;
			}
			else
			{
				*y = height;

				if (speed > 150)
					laraItem->HitPoints += 150 - speed;

				result = 0;
			}
		}

		return result;
	}

	static bool JeepCanGetOff(ItemInfo* jeepItem, ItemInfo* laraItem)
	{
		auto* lara = GetLaraInfo(laraItem);

		short angle = jeepItem->Pose.Orientation.y + ANGLE(90.0f);

		int x = jeepItem->Pose.Position.x - JEEP_DISMOUNT_DISTANCE * phd_sin(angle);
		int y = jeepItem->Pose.Position.y;
		int z = jeepItem->Pose.Position.z - JEEP_DISMOUNT_DISTANCE * phd_cos(angle);

		auto probe = GetCollision(x, y, z, jeepItem->RoomNumber);

		if (probe.Position.FloorSlope || probe.Position.Floor == NO_HEIGHT)
			return false;

		if (abs(probe.Position.Floor - jeepItem->Pose.Position.y) > WALL_SIZE / 2)
			return false;

		if ((probe.Position.Ceiling - jeepItem->Pose.Position.y) > -LARA_HEIGHT ||
			(probe.Position.Floor - probe.Position.Ceiling) < LARA_HEIGHT)
		{
			return false;
		}

		return true;
	}

	static void TriggerJeepExhaustSmoke(int x, int y, int z, short angle, short speed, int moving)
	{
		auto* spark = GetFreeParticle();

		spark->dR = 16;
		spark->dG = 16;
		spark->on = 1;
		spark->sR = 0;
		spark->sG = 0;
		spark->sB = 0;
		spark->dB = 32;

		if (moving)
		{
			spark->dR = (spark->dR * speed) / 32;
			spark->dG = (spark->dG * speed) / 32;
			spark->dB = (spark->dB * speed) / 32;
		}

		spark->colFadeSpeed = 4;
		spark->fadeToBlack = 4;
		spark->life = spark->sLife = (GetRandomControl() & 3) - (speed / 4096) + 20;;

		if (spark->life < 9)
		{
			spark->life = 9;
			spark->sLife = 9;
		}

		spark->blendMode = BLEND_MODES::BLENDMODE_ADDITIVE;
		spark->x = (GetRandomControl() & 0xF) + x - 8;
		spark->y = (GetRandomControl() & 0xF) + y - 8;
		spark->z = (GetRandomControl() & 0xF) + z - 8;
		spark->xVel = speed * phd_sin(angle) / 4;
		spark->yVel = -8 - (GetRandomControl() & 7);
		spark->zVel = speed * phd_cos(angle) / 4;
		spark->friction = 4;

		if (GetRandomControl() & 1)
		{
			spark->flags = 538;
			spark->rotAng = GetRandomControl() & 0xFFF;

			if (GetRandomControl() & 1)
				spark->rotAdd = -24 - (GetRandomControl() & 7);
			else
				spark->rotAdd = (GetRandomControl() & 7) + 24;
		}
		else
			spark->flags = 522;

		spark->scalar = 1;
		spark->gravity = -4 - (GetRandomControl() & 3);
		spark->maxYvel = -8 - (GetRandomControl() & 7);
		spark->dSize = (GetRandomControl() & 7) + (speed / 128) + 32;
		spark->sSize = spark->dSize / 2;
		spark->size = spark->dSize / 2;
	}

	static int JeepCheckGetOff(ItemInfo* laraItem)
	{
		auto* lara = GetLaraInfo(laraItem);

		if (laraItem->Animation.ActiveState == JS_GETOFF)
		{
			if (laraItem->Animation.FrameNumber == g_Level.Anims[laraItem->Animation.AnimNumber].frameEnd)
			{
				laraItem->Pose.Orientation.y += ANGLE(90.0f);
				TranslateItem(laraItem, laraItem->Pose.Orientation.y, -JEEP_DISMOUNT_DISTANCE);
				SetAnimation(laraItem, LA_STAND_SOLID);
				laraItem->Pose.Orientation.x = 0;
				laraItem->Pose.Orientation.z = 0;
				lara->Vehicle = NO_ITEM;
				lara->Control.HandStatus = HandStatus::Free;
				return false;
			}
		}

		return true;
	}

	static bool GetOnJeep(int itemNumber, ItemInfo* laraItem)
	{
		auto* jeepItem = &g_Level.Items[itemNumber];
		auto* lara = GetLaraInfo(laraItem);

		if (!(TrInput & IN_ACTION) && g_Gui.GetInventoryItemChosen() != ID_PUZZLE_ITEM1)
			return false;

		if (jeepItem->Flags & 0x100)
			return false;

		if (lara->Control.HandStatus != HandStatus::Free)
			return false;

		if (laraItem->Animation.ActiveState != LS_IDLE)
			return false;

		if (laraItem->Animation.AnimNumber != LA_STAND_IDLE)
			return false;

		if (laraItem->Animation.Airborne)
			return false;

		if (abs(jeepItem->Pose.Position.y - laraItem->Pose.Position.y) >= STEP_SIZE)
			return false;

		if (!TestBoundsCollide(jeepItem, laraItem, 100))
			return false;

		int floorHeight = GetCollision(jeepItem).Position.Floor;
		if (floorHeight < -32000)
			return false;

		short angle = phd_atan(jeepItem->Pose.Position.z - laraItem->Pose.Position.z, jeepItem->Pose.Position.x - laraItem->Pose.Position.x);
		angle -= jeepItem->Pose.Orientation.y;

		if ((angle > -ANGLE(45)) && (angle < ANGLE(135)))
		{
			int tempAngle = laraItem->Pose.Orientation.y - jeepItem->Pose.Orientation.y;
			if (tempAngle > ANGLE(45) && tempAngle < ANGLE(135))
			{
				if (g_Gui.GetInventoryItemChosen() == ID_PUZZLE_ITEM1)
				{
					g_Gui.SetInventoryItemChosen(NO_ITEM);
					return true;
				}
				else
				{
					if (g_Gui.IsObjectInInventory(ID_PUZZLE_ITEM1))
						g_Gui.SetEnterInventory(ID_PUZZLE_ITEM1);

					return false;
				}
			}
			else
				return false;
		}
		else
		{
			int tempAngle = laraItem->Pose.Orientation.y - jeepItem->Pose.Orientation.y;
			if (tempAngle > ANGLE(225) && tempAngle < ANGLE(315))
			{
				if (g_Gui.GetInventoryItemChosen() == ID_PUZZLE_ITEM1)
				{
					g_Gui.SetInventoryItemChosen(NO_ITEM);
					return true;
				}
				else
				{
					if (g_Gui.IsObjectInInventory(ID_PUZZLE_ITEM1))
						g_Gui.SetEnterInventory(ID_PUZZLE_ITEM1);

					return false;
				}
			}
			else
				return false;
		}

		return false;
	}

	static int GetJeepCollisionAnim(ItemInfo* jeepItem, Vector3Int* p)
	{
		auto* jeep = GetJeepInfo(jeepItem);

		if (jeep->unknown2 != 0)
			return 0;

		p->x = jeepItem->Pose.Position.x - p->x;
		p->z = jeepItem->Pose.Position.z - p->z;

		if (p->x || p->z)
		{
			float sinY = phd_sin(jeepItem->Pose.Orientation.y);
			float cosY = phd_cos(jeepItem->Pose.Orientation.y);

			int front = (p->z * cosY) + (p->x * sinY);
			int side = (p->z * -sinY) + (p->x * cosY);

			if (abs(front) > abs(side))
				return ((front > 0) + 13);
			else
				return ((side <= 0) + 11);
		}

		return 0;
	}

	int JeepDynamics(ItemInfo* jeepItem, ItemInfo* laraItem)
	{
		auto* jeep = GetJeepInfo(jeepItem);
		auto* lara = GetLaraInfo(laraItem);

		Vector3Int f_old, b_old, mm_old, mt_old, mb_old;

		int hf_old = TestJeepHeight(jeepItem, JEEP_FRONT, -JEEP_SIDE, &f_old);
		int hb_old = TestJeepHeight(jeepItem, JEEP_FRONT, JEEP_SIDE, &b_old);
		int hmm_old = TestJeepHeight(jeepItem, -(JEEP_FRONT + 50), -JEEP_SIDE, &mm_old);
		int hmt_old = TestJeepHeight(jeepItem, -(JEEP_FRONT + 50), JEEP_SIDE, &mt_old);
		int hmb_old = TestJeepHeight(jeepItem, -(JEEP_FRONT + 50), 0, (Vector3Int*)&mb_old);

		Vector3Int oldPos;
		oldPos.x = jeepItem->Pose.Position.x;
		oldPos.y = jeepItem->Pose.Position.y;
		oldPos.z = jeepItem->Pose.Position.z;

		if (mm_old.y > hmm_old)
			mm_old.y = hmm_old;
		if (mt_old.y > hmt_old)
			mt_old.y = hmt_old;
		if (f_old.y > hf_old)
			f_old.y = hf_old;
		if (b_old.y > hb_old)
			b_old.y = hb_old;
		if (mb_old.y > hmb_old)
			mb_old.y = hmb_old;

		short rot = 0;

		if (oldPos.y <= jeepItem->Floor - 8 )
		{
			if (jeep->jeepTurn < -JEEP_UNDO_TURN)
				jeep->jeepTurn += JEEP_UNDO_TURN;
			else if (jeep->jeepTurn > JEEP_UNDO_TURN)
				jeep->jeepTurn -= JEEP_UNDO_TURN;
			else
				jeep->jeepTurn = 0;

			jeepItem->Pose.Orientation.y += jeep->jeepTurn + jeep->extraRotation;
			jeep->momentumAngle += ((jeepItem->Pose.Orientation.y - jeep->momentumAngle) / 32);
		}
		else
		{
			short rot2 = 0;
			short momentum = 0;

			if (jeep->jeepTurn < -ANGLE(1))
				jeep->jeepTurn += ANGLE(1);
			else if (jeep->jeepTurn > ANGLE(1))
				jeep->jeepTurn -= ANGLE(1);
			else
				jeep->jeepTurn = 0;

			jeepItem->Pose.Orientation.y += jeep->jeepTurn + jeep->extraRotation;

			rot = jeepItem->Pose.Orientation.y - jeep->momentumAngle;
			momentum = 728 - ((3 * jeep->velocity) / 2048);

			if (!(TrInput & IN_ACTION) && jeep->velocity > 0)
				momentum -= (momentum / 4);

			if (rot >= -273)
			{
				if (rot <= 273)
					jeep->momentumAngle = jeepItem->Pose.Orientation.y;
				else
				{
					if (rot > 13650)
					{
						jeepItem->Pose.Position.y -= 41;
						jeepItem->Animation.VerticalVelocity = -6 - (GetRandomControl() & 3);
						jeep->jeepTurn = 0;
						jeep->velocity -= (jeep->velocity / 8);
					}

					if (rot <= 16380)
						jeep->momentumAngle += momentum;
					else
						jeep->momentumAngle = jeepItem->Pose.Orientation.y - 16380;
				}
			}
			else
			{
				if (rot < -13650)
				{
					jeepItem->Pose.Position.y -= 41;
					jeepItem->Animation.VerticalVelocity = -6 - (GetRandomControl() & 3);
					jeep->jeepTurn = 0;
					jeep->velocity -= (jeep->velocity / 8);
				}

				if (rot >= -16380)
					jeep->momentumAngle -= momentum;
				else
					jeep->momentumAngle = jeepItem->Pose.Orientation.y + 16380;
			}
		}

		short roomNumber = jeepItem->RoomNumber;
		FloorInfo* floor = GetFloor(jeepItem->Pose.Position.x, jeepItem->Pose.Position.y, jeepItem->Pose.Position.z, &roomNumber);
		int height = GetFloorHeight(floor, jeepItem->Pose.Position.x, jeepItem->Pose.Position.y, jeepItem->Pose.Position.z);

		short speed;
		if (jeepItem->Pose.Position.y < height)
			speed = jeepItem->Animation.Velocity;
		else
			speed = jeepItem->Animation.Velocity * phd_cos(jeepItem->Pose.Orientation.x);

		jeepItem->Pose.Position.x += speed * phd_sin(jeep->momentumAngle);
		jeepItem->Pose.Position.z += speed * phd_cos(jeep->momentumAngle);
	
		int slip = 0;
		if (jeepItem->Pose.Position.y >= height)
		{
			slip = JEEP_SLIP * phd_sin(jeepItem->Pose.Orientation.x);

			if (abs(slip) > 16)
			{
				JeepNoGetOff = 1;
				if (slip >= 0)
					slip = jeep->velocity - (SQUARE(slip - 16) / 2);
				else
					slip = (SQUARE(slip + 16) / 2) + jeep->velocity;
				jeep->velocity = slip;
			}
			else
				JeepNoGetOff = 0;

			slip = JEEP_SLIP_SIDE * phd_sin(jeepItem->Pose.Orientation.z);
			if (abs(slip) > JEEP_SLIP_SIDE / 4)
			{
				JeepNoGetOff = 1;

				if (slip >= 0)
				{
					jeepItem->Pose.Position.x += (slip - 24) * phd_sin(jeepItem->Pose.Orientation.y + ANGLE(90));
					jeepItem->Pose.Position.z += (slip - 24) * phd_cos(jeepItem->Pose.Orientation.y + ANGLE(90));
				}
				else
				{
					jeepItem->Pose.Position.x += (slip - 24) * phd_sin(jeepItem->Pose.Orientation.y - ANGLE(90));
					jeepItem->Pose.Position.z += (slip - 24) * phd_cos(jeepItem->Pose.Orientation.y - ANGLE(90));
				}
			}
			else
				JeepNoGetOff = 0;
		}

		if (jeep->velocity > JEEP_MAX_SPEED)
			jeep->velocity = JEEP_MAX_SPEED;
		else if (jeep->velocity < -JEEP_MAX_BACK)
			jeep->velocity = -JEEP_MAX_BACK;

		Vector3Int movedPos;
		movedPos.x = jeepItem->Pose.Position.x;
		movedPos.z = jeepItem->Pose.Position.z;

		if (!(jeepItem->Flags & ONESHOT))
			DoVehicleCollision(jeepItem, JEEP_FRONT);

		Vector3Int f, b, mm, mt, mb;
	
		int rot1 = 0;
		int rot2 = 0;

		int hf = TestJeepHeight(jeepItem, JEEP_FRONT, -JEEP_SIDE, (Vector3Int*)&f);
		if (hf < f_old.y - STEP_SIZE)
			rot1 = abs(4 * DoJeepShift(jeepItem, &f, &f_old));

		int hmm = TestJeepHeight(jeepItem, -(JEEP_FRONT + 50), -JEEP_SIDE, (Vector3Int*)&mm);
		if (hmm < mm_old.y - STEP_SIZE)
		{
			if (rot)
				rot1 += abs(4 * DoJeepShift(jeepItem, &mm, &mm_old));
			else
				rot1 = -abs(4 * DoJeepShift(jeepItem, &mm, &mm_old));
		}

		int hb = TestJeepHeight(jeepItem, JEEP_FRONT, JEEP_SIDE, (Vector3Int*)&b);
		if (hb < b_old.y - STEP_SIZE)
			rot2 = -abs(4 * DoJeepShift(jeepItem, &b, &b_old));

		int hmb = TestJeepHeight(jeepItem, -(JEEP_FRONT + 50), 0, (Vector3Int*)&mb);
		if (hmb < mb_old.y - STEP_SIZE)
			DoJeepShift(jeepItem, &mb, &mb_old);
	
		int hmt = TestJeepHeight(jeepItem, -(JEEP_FRONT + 50), JEEP_SIDE, (Vector3Int*)&mt);
		if (hmt < mt_old.y - STEP_SIZE)
		{
			if (rot2)
				rot2 -= abs(4 * DoJeepShift(jeepItem, &mt, &mt_old));
			else
				rot2 = abs(4 * DoJeepShift(jeepItem, &mt, &mt_old));
		}

		if (!rot1)
			rot1 = rot2;
	   
		roomNumber = jeepItem->RoomNumber;
		floor = GetFloor(jeepItem->Pose.Position.x, jeepItem->Pose.Position.y, jeepItem->Pose.Position.z, &roomNumber);
		if (GetFloorHeight(floor, jeepItem->Pose.Position.x, jeepItem->Pose.Position.y, jeepItem->Pose.Position.z) < jeepItem->Pose.Position.y - STEP_SIZE)
			DoJeepShift(jeepItem, (Vector3Int*)&jeepItem->Pose, &oldPos);

		if (!jeep->velocity)
			rot1 = 0;

		jeep->extraRotation = rot1;
	
		/*jeep->unknown0 += rot1 / 2;

		if (abs(jeep->unknown0) < 2)
			jeep->unknown0 = 0;

		if (abs(jeep->unknown0 - jeep->extraRotation) >= 4)
			jeep->extraRotation += ((jeep->unknown0 - jeep->extraRotation) / 4);
		else
			jeep->extraRotation = jeep->unknown0;
			*/ // just incase this code is ever uncommented.
		int newspeed = 0;
		int collide = GetJeepCollisionAnim(jeepItem, &movedPos);
	
		if (collide)
		{
			newspeed = (jeepItem->Pose.Position.z - oldPos.z) * phd_cos(jeep->momentumAngle) + (jeepItem->Pose.Position.x - oldPos.x) * phd_sin(jeep->momentumAngle);
			newspeed *= 256;

			if ((&g_Level.Items[lara->Vehicle] == jeepItem) && (jeep->velocity == JEEP_MAX_SPEED) && (newspeed < (JEEP_MAX_SPEED - 10)))
			{
				DoDamage(laraItem, (JEEP_MAX_SPEED - newspeed) / 128);
			}

			if (jeep->velocity > 0 && newspeed < jeep->velocity)
				jeep->velocity = (newspeed > 0) ? 0 : newspeed;

			else if (jeep->velocity < 0 && newspeed > jeep->velocity)
				jeep->velocity = (newspeed < 0) ? 0 : newspeed;

			if (jeep->velocity < JEEP_MAX_BACK)
				jeep->velocity = JEEP_MAX_BACK;
		}

		return collide;
	}

	static int JeepUserControl(ItemInfo* jeepItem, ItemInfo* laraItem, int height, int* pitch)
	{
		auto* jeep = GetJeepInfo(jeepItem);

		if (laraItem->Animation.ActiveState == JS_GETOFF || laraItem->Animation.TargetState == JS_GETOFF)
			TrInput = 0;
	
		if (jeep->revs <= 16)
			jeep->revs = 0;
		else
		{
			jeep->velocity += (jeep->revs / 16);
			jeep->revs -= (jeep->revs / 8);
		}

		int rot1 = 0;
		int rot2 = 0;

		if (jeepItem->Pose.Position.y >= height - STEP_SIZE)
		{
			if (!jeep->velocity)
			{
				if (TrInput & IN_LOOK)
					LookUpDown(laraItem);
			}

			if (abs(jeep->velocity) <= JEEP_MAX_SPEED / 2)
			{
				rot1 = ANGLE(5) * abs(jeep->velocity) / 16384;
				rot2 = 60 * abs(jeep->velocity) / 16384 + ANGLE(1);
			}
			else
			{
				rot1 = ANGLE(5);
				rot2 = 242;
			}

			if (jeep->velocity < 0)
			{
				if (TrInput & (IN_RIGHT | IN_RSTEP))
				{
					jeep->jeepTurn -= rot2;
					if (jeep->jeepTurn < -rot1)
						jeep->jeepTurn = -rot1;
				}
				else if (TrInput & (IN_LEFT | IN_LSTEP))
				{
					jeep->jeepTurn += rot2;
					if (jeep->jeepTurn > rot1)
						jeep->jeepTurn = rot1;
				}
			}
			else if (jeep->velocity > 0)
			{
				if (TrInput & (IN_RIGHT | IN_RSTEP))
				{
					jeep->jeepTurn += rot2;
					if (jeep->jeepTurn > rot1)
						jeep->jeepTurn = rot1;
				}
				else if (TrInput & (IN_LEFT | IN_LSTEP))
				{
					jeep->jeepTurn -= rot2;
					if (jeep->jeepTurn < -rot1)
						jeep->jeepTurn = -rot1;
				}
			}

			// Break/reverse
			if (TrInput & JEEP_IN_BRAKE)
			{
				if (jeep->velocity < 0)
				{
					jeep->velocity += 768;
					if (jeep->velocity > 0)
						jeep->velocity = 0;
				}
				else if (jeep->velocity > 0)
				{
					jeep->velocity -= 768;
					if (jeep->velocity < 0)
						jeep->velocity = 0;
				}
			}
			// Accelerate
			else if (TrInput & JEEP_IN_ACCELERATE)
			{
				if (jeep->unknown2)
				{
					if (jeep->unknown2 == 1 && jeep->velocity > -JEEP_MAX_BACK)
						jeep->velocity -= (abs(-JEEP_MAX_BACK - jeep->velocity) / 8) - 2;
					else if (jeep->unknown2 == 1 && jeep->velocity < -JEEP_MAX_BACK)
						jeep->velocity = -JEEP_MAX_BACK;
				}
				else
				{
					if (jeep->velocity < JEEP_MAX_SPEED)
					{
						if (jeep->velocity < 0x4000)
							jeep->velocity += 8 + ((0x4000 + 0x800 - jeep->velocity) / 8);
						else if (jeep->velocity < 0x7000)
							jeep->velocity += 4 + ((0x7000 + 0x800 - jeep->velocity) / 16);
						else if (jeep->velocity < JEEP_MAX_SPEED)
							jeep->velocity += 2 + ((JEEP_MAX_SPEED - jeep->velocity) / 8);
					}
					else
						jeep->velocity = JEEP_MAX_SPEED;
				}

				jeep->velocity -= (abs(jeepItem->Pose.Orientation.y - jeep->momentumAngle) / 64);
			}
			else if (jeep->velocity > 256)
				jeep->velocity -= 256;
			else if (jeep->velocity < -256)
				jeep->velocity += 256;
			else
				jeep->velocity = 0;

			jeepItem->Animation.Velocity = jeep->velocity / 256;
			if (jeep->engineRevs > 0xC000)
				jeep->engineRevs = (GetRandomControl() & 0x1FF) + 48896;

			int revs = jeep->velocity;
			if (jeep->velocity < 0)
				revs /= 2;

			jeep->engineRevs += ((abs(revs) - jeep->engineRevs) / 8);
		}

		if (TrInput & JEEP_IN_BRAKE)
		{
			Vector3Int pos;
			pos.x = 0;
			pos.y = -144;
			pos.z = -1024;
			GetJointAbsPosition(jeepItem, &pos, 11);
			TriggerDynamicLight(pos.x, pos.y, pos.z, 10, 64, 0, 0);
			jeepItem->MeshBits = 163839;
		}
		else
			jeepItem->MeshBits = 114687;
	
		*pitch = jeep->engineRevs;

		return 1;
	}

	static void AnimateJeep(ItemInfo* jeepItem, ItemInfo* laraItem, int collide, int dead)
	{
		auto* jeep = GetJeepInfo(jeepItem);

		bool dismount;
		if (jeepItem->Pose.Position.y != jeepItem->Floor && 
			laraItem->Animation.ActiveState != JS_JUMP && 
			laraItem->Animation.ActiveState != JS_LAND && 
			!dead)
		{
			if (jeep->unknown2 == 1)
				laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_BACK_JUMP_START;
			else
				laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_FWD_JUMP_START;

			laraItem->Animation.ActiveState = JS_JUMP;
			laraItem->Animation.TargetState = JS_JUMP;
			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
		}
		else if  ( (collide) && 
			(laraItem->Animation.ActiveState != 4) && 
			(laraItem->Animation.ActiveState != 5) && 
			(laraItem->Animation.ActiveState != 2) &&
			(laraItem->Animation.ActiveState != 3) &&
			(laraItem->Animation.ActiveState != JS_JUMP) &&
			(0x2AAA < (short)jeep->velocity) &&
			(!dead) )
		{
			short state;
			switch (collide)
			{
			case 13:
				state = 4;
				laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 11;
				break;

			case 14:
				state = 5;
				laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 10;
				break;

			case 11:
				state = 2;
				laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 12;
				break;

			case 12:
				state = 3;
				laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + 13;
				break;
			}

			laraItem->Animation.ActiveState = state;
			laraItem->Animation.TargetState = state;
			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
		}
		else
		{
			switch (laraItem->Animation.ActiveState)
			{
			case JS_STOP:
				if (dead)
					laraItem->Animation.TargetState = JS_DEATH;
				else
				{
					dismount = ((TrInput & JEEP_IN_BRAKE) && (TrInput & IN_LEFT)) ? true : false;
					if (dismount &&
						!jeep->velocity &&
						!JeepNoGetOff)
					{
						if (JeepCanGetOff(jeepItem, laraItem))
							laraItem->Animation.TargetState = JS_GETOFF;
						break;
					}

					if (DbInput & IN_WALK)
					{
						if (jeep->unknown2)
							jeep->unknown2--;
						break;
					}
					else if (DbInput & IN_SPRINT)
					{
						if (jeep->unknown2 < 1)
						{
							jeep->unknown2++;
							if (jeep->unknown2 == 1)
								laraItem->Animation.TargetState = JS_DRIVE_BACK;
							break;
						}
					}
					else
					{
						if ((TrInput & JEEP_IN_ACCELERATE) && !(TrInput & JEEP_IN_BRAKE))
						{
							laraItem->Animation.TargetState = JS_DRIVE_FORWARD;
							break;
						}
						else if (TrInput & (IN_LEFT | IN_LSTEP))
							laraItem->Animation.TargetState = JS_FWD_LEFT;
						else if (TrInput & (IN_RIGHT | IN_RSTEP))
							laraItem->Animation.TargetState = JS_FWD_RIGHT;
					}

	/*				if (!(DbInput & IN_WALK))
					{
						if (!(DbInput & IN_SPRINT))
						{
							if ((TrInput & JEEP_IN_ACCELERATE) && !(TrInput & JEEP_IN_BRAKE))
							{
								laraItem->TargetState = JS_DRIVE_FORWARD;
								break;
							}
							else if (TrInput & (IN_LEFT | IN_LSTEP))
								laraItem->TargetState = JS_FWD_LEFT;
							else if (TrInput & (IN_RIGHT | IN_RSTEP))
								laraItem->TargetState = JS_FWD_RIGHT;
						}
						else if (jeep->unknown2 < 1)
						{
							jeep->unknown2++;
							if (jeep->unknown2 == 1)
								laraItem->TargetState = JS_DRIVE_BACK;

						}
					}
					else
					{
						if (jeep->unknown2)
							jeep->unknown2--;
					}*/
				}

				break;

			case JS_DRIVE_FORWARD:
				if (dead)
					laraItem->Animation.TargetState = JS_STOP;
				else
				{
					if (jeep->velocity & 0xFFFFFF00 ||
						TrInput & (JEEP_IN_ACCELERATE | JEEP_IN_BRAKE))
					{
						if (TrInput & JEEP_IN_BRAKE)
						{
							if (jeep->velocity <= 21844)
								laraItem->Animation.TargetState = JS_STOP;
							else
								laraItem->Animation.TargetState = JS_BRAKE;
						}
						else
						{
							if (TrInput & (IN_LEFT | IN_LSTEP))
								laraItem->Animation.TargetState = JS_FWD_LEFT;
							else if (TrInput & (IN_RIGHT | IN_RSTEP))
								laraItem->Animation.TargetState = JS_FWD_RIGHT;
						}
					}
					else
						laraItem->Animation.TargetState = JS_STOP;
				}

				break;

			case 2:
			case 3:
			case 4:
			case 5:
				if (dead)
					laraItem->Animation.TargetState = JS_STOP;
				else if (TrInput & (JEEP_IN_ACCELERATE | JEEP_IN_BRAKE))
					laraItem->Animation.TargetState = JS_DRIVE_FORWARD;
				break;

			case JS_BRAKE:
				if (dead)
					laraItem->Animation.TargetState = JS_STOP;
				else if (jeep->velocity & 0xFFFFFF00)
				{
					if (TrInput & (IN_LEFT | IN_LSTEP))
						laraItem->Animation.TargetState = JS_FWD_LEFT;
					else if (TrInput & (IN_RIGHT | IN_RSTEP))
						laraItem->Animation.TargetState = JS_FWD_RIGHT;
				}
				else
					laraItem->Animation.TargetState = JS_STOP;
				break;

			case JS_FWD_LEFT:
				if (dead)
					laraItem->Animation.TargetState = JS_STOP;
				else if (!(DbInput & IN_WALK))
				{
					if (DbInput & IN_SPRINT)
					{
						if (jeep->unknown2 < 1)
						{
							jeep->unknown2++;
							if (jeep->unknown2 == 1)
							{
								laraItem->Animation.TargetState = JS_BACK_RIGHT;
								laraItem->Animation.ActiveState = JS_BACK_RIGHT;
								laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_REVERSE_RIGHT;
								laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
								break;
							}
						}
					}
					else if (TrInput & (IN_RIGHT | IN_RSTEP))
						laraItem->Animation.TargetState = JS_DRIVE_FORWARD;
					else if (TrInput & (IN_LEFT | IN_LSTEP))
						laraItem->Animation.TargetState = JS_FWD_LEFT;
					else if (jeep->velocity)
						laraItem->Animation.TargetState = JS_DRIVE_FORWARD;
					else
						laraItem->Animation.TargetState = JS_STOP;
				}
				else
				{
					if (jeep->unknown2)
						jeep->unknown2--;
				}

				if (laraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_FWD_LEFT &&
					!jeep->velocity)
				{
					laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_RIGHT_START;
					laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase + JA_IDLE;
				}
				if (laraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_RIGHT_START)
				{
					if (jeep->velocity)
					{
						laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_FWD_LEFT;
						laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
					}
				}

				break;

			case JS_FWD_RIGHT:
				if (dead)
					laraItem->Animation.TargetState = JS_STOP;
				if (!(DbInput & IN_WALK))
				{
					if (DbInput & IN_SPRINT)
					{
						if (jeep->unknown2 < 1)
						{
							jeep->unknown2++;
							if (jeep->unknown2 == 1)
							{
								laraItem->Animation.TargetState = JS_BACK_LEFT;
								laraItem->Animation.ActiveState = JS_BACK_LEFT;
								laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_REVERSE_LEFT;
								laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
								break;
							}
						}
					}
					else if (TrInput & (IN_LEFT | IN_LSTEP))
						laraItem->Animation.TargetState = JS_DRIVE_FORWARD;
					else if (TrInput & (IN_RIGHT | IN_RSTEP))
						laraItem->Animation.TargetState = JS_FWD_RIGHT;
					else if (jeep->velocity)
						laraItem->Animation.TargetState = JS_DRIVE_FORWARD;
					else
						laraItem->Animation.TargetState = JS_STOP;
				}
				else
				{
					if (jeep->unknown2)
						jeep->unknown2--;
				}

				if (laraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_FWD_RIGHT && !jeep->velocity)
				{
					laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_LEFT_START;
					laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase + 14;//hmm
				}
				if (laraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_LEFT_START)
				{
					if (jeep->velocity)
					{
						laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_FWD_RIGHT;
						laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
					}
				}

				break;

			case JS_JUMP:
				if (jeepItem->Pose.Position.y == jeepItem->Floor)
					laraItem->Animation.TargetState = JS_LAND;
				else if (jeepItem->Animation.VerticalVelocity > 300)
					jeep->flags |= JEEP_FLAG_FALLING;
				break;

			case JS_BACK:
				if (dead)
					laraItem->Animation.TargetState = JS_DRIVE_BACK;
				else if (abs(jeep->velocity) & 0xFFFFFF00)
				{
					if (TrInput & (IN_LEFT | IN_LSTEP))
						laraItem->Animation.TargetState = JS_BACK_RIGHT;
					else if (TrInput & (IN_RIGHT | IN_RSTEP))
						laraItem->Animation.TargetState = JS_BACK_LEFT;
				}
				else
					laraItem->Animation.TargetState = JS_DRIVE_BACK;

				break;

			case JS_BACK_LEFT:
				if (dead)
					laraItem->Animation.TargetState = JS_DRIVE_BACK;
				else if (!(DbInput & IN_WALK))
				{
					if (DbInput & IN_SPRINT)
					{
						if (jeep->unknown2 < 1)
							jeep->unknown2++;
					}
					else if (TrInput & (IN_RIGHT | IN_RSTEP))
						laraItem->Animation.TargetState = JS_BACK_LEFT;
					else
						laraItem->Animation.TargetState = JS_BACK;
				}
				else
				{
					if (jeep->unknown2)
					{
						jeep->unknown2--;
						if (!jeep->unknown2)
						{
							laraItem->Animation.TargetState = JS_FWD_RIGHT;
							laraItem->Animation.ActiveState = JS_FWD_RIGHT;
							laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_FWD_RIGHT;
							laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
							break;
						}
					}
				}

				if (laraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_BACK_LEFT && !jeep->velocity)
				{
					laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_LEFT_BACK_START;
					laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase + 14;
				}
				if (laraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_LEFT_BACK_START)
				{
					if (jeep->velocity)
					{
						laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_BACK_LEFT;
						laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
					}
				}

				break;

			case JS_BACK_RIGHT:
				if (dead)
				{
					laraItem->Animation.TargetState = JS_DRIVE_BACK;
				}
				else if (!(DbInput & IN_WALK))
				{
					if (DbInput & IN_SPRINT)
					{
						if (jeep->unknown2 < 1)
							jeep->unknown2++;
					}
					else if (TrInput & (IN_LEFT | IN_LSTEP))
						laraItem->Animation.TargetState = JS_BACK_RIGHT;
					else
						laraItem->Animation.TargetState = JS_BACK;

					if (laraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_BACK_RIGHT && !jeep->velocity)
					{
						laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_RIGHT_BACK_START;
						laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase + 14;
					}
					if (laraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_RIGHT_BACK_START)
					{
						if (jeep->velocity)
						{
							laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_BACK_RIGHT;
							laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
						}
					}
					break;
				}
				else if (!jeep->unknown2 || (--jeep->unknown2 != 0))
				{
					if (laraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_BACK_RIGHT && !jeep->velocity)
					{
						laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_RIGHT_BACK_START;
						laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase + 14;
					}
					if (laraItem->Animation.AnimNumber == Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_RIGHT_BACK_START)
					{
						if (jeep->velocity)
						{
							laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_BACK_RIGHT;
							laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
						}
					}
					break;
				}

				laraItem->Animation.TargetState = JS_FWD_LEFT;
				laraItem->Animation.ActiveState = JS_FWD_LEFT;
				laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_IDLE_FWD_RIGHT;
				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;

				break;

			case JS_DRIVE_BACK:
				if (dead)
					laraItem->Animation.TargetState = JS_STOP;
				else
	//			if (jeep->velocity || JeepNoGetOff)
				{
					if (!(DbInput & IN_WALK))
					{
						if (DbInput & IN_SPRINT)
						{
							if (jeep->unknown2 < 1)
								jeep->unknown2++;
						}
						else if (!(TrInput & JEEP_IN_ACCELERATE) || TrInput & JEEP_IN_BRAKE)
						{
							if (TrInput & (IN_LEFT | IN_LSTEP))
								laraItem->Animation.TargetState = JS_BACK_RIGHT;
							else if (TrInput & (IN_LEFT | IN_LSTEP))
								laraItem->Animation.TargetState = JS_BACK_LEFT;
						}
						else
							laraItem->Animation.TargetState = JS_BACK;
					}
					else
					{
						if (jeep->unknown2)
						{
							jeep->unknown2--;
							if (!jeep->unknown2)
								laraItem->Animation.TargetState = JS_STOP;
						}
					}
				}

				break;

			default:
				break;
			}
		}
	}

	void JeepCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* item = &g_Level.Items[itemNumber];
		auto* lara = GetLaraInfo(laraItem);

		if (laraItem->HitPoints <= 0 && lara->Vehicle != NO_ITEM)
			return;
	
		if (GetOnJeep(itemNumber, laraItem))
		{
			lara->Vehicle = itemNumber;

			if (lara->Control.Weapon.GunType == LaraWeaponType::Flare)
			{
				CreateFlare(laraItem, ID_FLARE_ITEM, 0);
				UndrawFlareMeshes(laraItem);
				lara->Flare.ControlLeft = 0;
				lara->Control.Weapon.RequestGunType = LaraWeaponType::None;
				lara->Control.Weapon.GunType = LaraWeaponType::None;
			}

			lara->Control.HandStatus = HandStatus::Busy;

			/*v4 = *(_WORD*)(Rooms + 148 * (signed short)v3->roomNumber + 72);
			// Enable ENEMY_JEEP
			if (v4 != -1)
			{
				while (1)
				{
					v5 = Items + 5622 * v4;
					if (*(_WORD*)(Items + 5622 * v4 + 12) == 34)
					{
						break;
					}
					v4 = *(_WORD*)(v5 + 26);
					if (v4 == -1)
					{
						goto LABEL_11;
					}
				}
				EnableBaddyAI(v4, 1);
				*(_DWORD*)(v5 + 5610) = *(_DWORD*)(v5 + 5610) & 0xFFFFFFFB | 2;
				AddActiveItem(v4);
			}*/

			short ang = phd_atan(item->Pose.Position.z - laraItem->Pose.Position.z, item->Pose.Position.x - laraItem->Pose.Position.x);
			ang -= item->Pose.Orientation.y;

			if ((ang > -(ANGLE(45))) && (ang < (ANGLE(135))))
				laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_GETIN_LEFT;
			else
				laraItem->Animation.AnimNumber = Objects[ID_JEEP_LARA_ANIMS].animIndex + JA_GETIN_RIGHT;

			laraItem->Animation.TargetState = JS_GETIN;
			laraItem->Animation.ActiveState = JS_GETIN;
			laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;

			item->HitPoints = 1;
			laraItem->Pose.Position = item->Pose.Position;
			laraItem->Pose.Orientation.y = item->Pose.Orientation.y;

			ResetLaraFlex(laraItem);
			lara->HitDirection = -1;

			AnimateItem(laraItem);

			int anim = laraItem->Animation.AnimNumber;

			JeepInfo* jeep = (JeepInfo*)item->Data;
			jeep->revs = 0;
			jeep->unknown2 = 0;

			item->Flags |= TRIGGERED;
		}
		else
			ObjectCollision(itemNumber, laraItem, coll);
	}

	int JeepControl(ItemInfo* laraItem)
	{
		auto* lara = GetLaraInfo(laraItem);
		auto* jeepItem = &g_Level.Items[lara->Vehicle];
		auto* jeep = GetJeepInfo(jeepItem);

		int drive = -1;
		bool dead = 0;

		int collide = JeepDynamics(jeepItem, laraItem);

		short roomNumber = jeepItem->RoomNumber;
		FloorInfo* floor = GetFloor(jeepItem->Pose.Position.x, jeepItem->Pose.Position.y, jeepItem->Pose.Position.z, &roomNumber);

		int floorHeight = GetFloorHeight(floor, jeepItem->Pose.Position.x, jeepItem->Pose.Position.y, jeepItem->Pose.Position.z);
		int ceiling = GetCeiling(floor, jeepItem->Pose.Position.x, jeepItem->Pose.Position.y, jeepItem->Pose.Position.z);

		Vector3Int fl, fr, bc;
		int hfl = TestJeepHeight(jeepItem, JEEP_FRONT, -JEEP_SIDE, &fl);
		int hfr = TestJeepHeight(jeepItem, JEEP_FRONT, JEEP_SIDE, &fr);
		int hbc = TestJeepHeight(jeepItem, -(JEEP_FRONT + 50), 0, &bc);

		roomNumber = jeepItem->RoomNumber;
		floor = GetFloor(jeepItem->Pose.Position.x, jeepItem->Pose.Position.y, jeepItem->Pose.Position.z, &roomNumber);
		floorHeight = GetFloorHeight(floor, jeepItem->Pose.Position.x, jeepItem->Pose.Position.y, jeepItem->Pose.Position.z);

		TestTriggers(jeepItem, true);
		TestTriggers(jeepItem, false);

		if (laraItem->HitPoints <= 0)
		{
			dead = true;
			TrInput &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
		}

		int pitch = 0;
		if (jeep->flags)
			collide = 0;
		else if (laraItem->Animation.ActiveState == JS_GETIN)
		{
			drive = -1;
			collide = 0;
		}
		else
			drive = JeepUserControl(jeepItem, laraItem, floorHeight, &pitch);

		if (jeep->velocity || jeep->revs)
		{
			jeep->pitch = pitch;
			if (pitch >= -32768)
			{
				if (pitch > 40960)
					jeep->pitch = 40960;
			}
			else
				jeep->pitch = -32768;

			SoundEffect(SFX_TR4_VEHICLE_JEEP_MOVING, &jeepItem->Pose, SoundEnvironment::Land, 0.5f + jeep->pitch / 65535.0f);
		}
		else
		{
			if (drive != -1)
				SoundEffect(SFX_TR4_VEHICLE_JEEP_IDLE, &jeepItem->Pose);
			jeep->pitch = 0;
		}

		jeepItem->Floor = floorHeight;
		short rotAdd = jeep->velocity / 4;
		jeep->rot1 -= rotAdd;
		jeep->rot2 -= rotAdd;
		jeep->rot3 -= rotAdd;
		jeep->rot4 -= rotAdd;

		int oldY = jeepItem->Pose.Position.y;
		jeepItem->Animation.VerticalVelocity = DoJeepDynamics(laraItem, floorHeight, jeepItem->Animation.VerticalVelocity, &jeepItem->Pose.Position.y, 0);
		jeep->velocity = DoVehicleWaterMovement(jeepItem, laraItem, jeep->velocity, JEEP_FRONT, &jeep->jeepTurn);

		floorHeight = (fl.y + fr.y) / 2;
		short xRot;
		short zRot;
		if (bc.y >= hbc)
		{
			if (floorHeight >= (hfl + hfr) / 2)
				xRot = phd_atan(1100, hbc - floorHeight);
			else
				xRot = phd_atan(JEEP_FRONT, hbc - jeepItem->Pose.Position.y);
		}
		else
		{
			if (floorHeight >= (hfl + hfr) / 2)
				xRot = phd_atan(JEEP_FRONT, jeepItem->Pose.Position.y - floorHeight);
			else
			{
				xRot = -phd_atan(137, oldY - jeepItem->Pose.Position.y);
				if (jeep->velocity < 0)
					xRot = -xRot;
			}
		}

		jeepItem->Pose.Orientation.x += (xRot - jeepItem->Pose.Orientation.x) / 4;
		jeepItem->Pose.Orientation.z += (phd_atan(256, floorHeight - fl.y) - jeepItem->Pose.Orientation.z) / 4;
		if (jeep->velocity == 0)
		{
			jeepItem->Pose.Orientation.x = 0;
			jeepItem->Pose.Orientation.z = 0;
		}

		if (!(jeep->flags & JEEP_FLAG_DEAD))
		{
			if (roomNumber != jeepItem->RoomNumber)
			{
				ItemNewRoom(lara->Vehicle, roomNumber);
				ItemNewRoom(lara->ItemNumber, roomNumber);
			}

			laraItem->Pose = jeepItem->Pose;

			int jeepAnim = Objects[ID_JEEP].animIndex;
			int laraAnim = laraItem->Animation.AnimNumber;
			int extraAnim = Objects[ID_JEEP_LARA_ANIMS].animIndex;

			AnimateJeep(jeepItem, laraItem, collide, dead);
			AnimateItem(laraItem);

			jeepItem->Animation.AnimNumber = Objects[ID_JEEP].animIndex + laraItem->Animation.AnimNumber - Objects[ID_JEEP_LARA_ANIMS].animIndex;
			jeepItem->Animation.FrameNumber = g_Level.Anims[jeepItem->Animation.AnimNumber].frameBase + (laraItem->Animation.FrameNumber - g_Level.Anims[laraItem->Animation.AnimNumber].frameBase);

			jeepAnim = Objects[ID_JEEP].animIndex;
			laraAnim = laraItem->Animation.AnimNumber;
			extraAnim = Objects[ID_JEEP_LARA_ANIMS].animIndex;

			Camera.targetElevation = -ANGLE(30.0f);
			Camera.targetDistance = SECTOR(2);

			if (jeep->unknown2)
			{
				if (jeep->unknown2 == 1)
					jeep->fallSpeed += ((32578 - jeep->fallSpeed) / 8);
			}
			else
				jeep->fallSpeed -= (jeep->fallSpeed / 8);

			Camera.targetAngle = jeep->fallSpeed;

			if (jeep->flags & JEEP_FLAG_FALLING && jeepItem->Pose.Position.y == jeepItem->Floor)
			{
				laraItem->MeshBits = 0;
				ExplodeVehicle(laraItem, jeepItem);
				return 0;
			}
		}

		if (laraItem->Animation.ActiveState == JS_GETIN ||
			laraItem->Animation.ActiveState == JS_GETOFF)
		{
			JeepSmokeStart = 0;
		}
		else
		{
			short speed = 0;
			short angle = 0;

			auto pos = Vector3Int(90, 0, -500);
			GetJointAbsPosition(jeepItem, &pos, 11);

			if (jeepItem->Animation.Velocity <= 32)
			{
				if (JeepSmokeStart >= 16)
				{
					if (GetRandomControl() & 3)
						speed = 0;
					else
						speed = ((GetRandomControl() & 0xF) + GetRandomControl() & 0x10) * 64;
				}
				else
				{
					speed = ((GetRandomControl() & 7) + GetRandomControl() & 0x10 + 2 * JeepSmokeStart) * 64;
					JeepSmokeStart++;
				}

				TriggerJeepExhaustSmoke(pos.x, pos.y, pos.z, jeepItem->Pose.Orientation.y + -32768, speed, 0);
			}
			else if (jeepItem->Animation.Velocity < 64)
				TriggerJeepExhaustSmoke(pos.x, pos.y, pos.z, jeepItem->Pose.Orientation.y - 32768, 64 - jeepItem->Animation.Velocity, 1);
		}

		return JeepCheckGetOff(laraItem);
	}
}
