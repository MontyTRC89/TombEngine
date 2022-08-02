#include "framework.h"
#include "Specific/level.h"
#include "Game/control/control.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/control/lot.h"
#include "Specific/input.h"
#include "Game/Lara/lara_helpers.h"
#include "Game/Lara/lara_struct.h"
#include "Game/Lara/lara.h"
#include "Specific/trmath.h"
#include "Game/collision/collide_room.h"
#include "Game/collision/sphere.h"
#include "Objects/Generic/Object/rope.h"
#include "Sound/sound.h"
#include "Game/camera.h"

using namespace TEN::Input;

namespace TEN::Entities::Generic
{
	PENDULUM CurrentPendulum;
	PENDULUM AlternatePendulum;
	std::vector<ROPE_STRUCT> Ropes;
	int RopeSwing = 0;

	void InitialiseRope(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		short roomNumber = item->RoomNumber;

		auto itemPos = item->Pose.Position;

		itemPos.y = GetCollision(item).Position.Ceiling;

		auto pos = Vector3Int(0, 16384, 0);
		ROPE_STRUCT rope;
		PrepareRope(&rope, &itemPos, &pos, CLICK(0.5f), item);

		item->TriggerFlags = short(Ropes.size());

		Ropes.push_back(rope);
	}

	void PrepareRope(ROPE_STRUCT* rope, Vector3Int* pos1, Vector3Int* pos2, int length, ItemInfo* item)
	{
		rope->room = item->RoomNumber;
		rope->position = *pos1;
		rope->segmentLength = length << 16;

		pos2->x <<= 16;
		pos2->y <<= 16;
		pos2->z <<= 16;

		NormaliseRopeVector(pos2);

		if (item->TriggerFlags == -1)
			rope->coiled = 30;
		else
			rope->coiled = 0;

		int l = 0;
		int sum = 0;
		int il = 3145728;

		for (int i = 0; i < ROPE_SEGMENTS; ++i)
		{
			rope->segment[i].x = (int64_t)sum * pos2->x >> FP_SHIFT;
			rope->segment[i].y = (int64_t)sum * pos2->x >> FP_SHIFT;
			rope->segment[i].z = (int64_t)sum * pos2->z >> FP_SHIFT;

			rope->velocity[i].x = 0;
			rope->velocity[i].y = 0;
			rope->velocity[i].z = 0;

			if (item->TriggerFlags == -1)
			{
				rope->segment[i].x = l;
				rope->segment[i].y >>= 4;

				rope->velocity[i].x = 16384;
				rope->velocity[i].y = il;
				rope->velocity[i].z = 16384;
			}

			l += 1024;
			sum += rope->segmentLength;
			il -= 131072;
		}

		rope->active = 0;
	}

	Vector3Int* NormaliseRopeVector(Vector3Int* vec)
	{
		int x = vec->x >> FP_SHIFT;
		int y = vec->y >> FP_SHIFT;
		int z = vec->z >> FP_SHIFT;

		if (!x && !y && !z)
			return vec;

		int length = pow(x, 2) + pow(y, 2) + pow(z, 2);
		if (length < 0)
			length = -length;

		length = 65536 / sqrt(length);

		vec->x = ((int64_t)length * vec->x) >> FP_SHIFT;
		vec->y = ((int64_t)length * vec->y) >> FP_SHIFT;
		vec->z = ((int64_t)length * vec->z) >> FP_SHIFT;
		return vec;
	}

	void GetRopePos(ROPE_STRUCT* rope, int segmentFrame, int* x, int* y, int* z)
	{
		int segment;
		short frame;

		segment = segmentFrame / 128;
		frame = segmentFrame & 0x7F;

		*x = (rope->normalisedSegment[segment].x * frame >> FP_SHIFT) + (rope->meshSegment[segment].x >> FP_SHIFT) + rope->position.x;
		*y = (rope->normalisedSegment[segment].y * frame >> FP_SHIFT) + (rope->meshSegment[segment].y >> FP_SHIFT) + rope->position.y;
		*z = (rope->normalisedSegment[segment].z * frame >> FP_SHIFT) + (rope->meshSegment[segment].z >> FP_SHIFT) + rope->position.z;
	}

	int DotProduct(Vector3Int* u, Vector3Int* v)
	{
		return (u->x * v->x + u->y * v->y + u->z * v->z) >> W2V_SHIFT;
	}

	void ScaleVector(Vector3Int* u, int c, Vector3Int* destination)
	{
		destination->x = c * u->x >> W2V_SHIFT;
		destination->y = c * u->y >> W2V_SHIFT;
		destination->z = c * u->z >> W2V_SHIFT;
	}

	void CrossProduct(Vector3Int* u, Vector3Int* v, Vector3Int* destination)
	{
		destination->x = (u->y * v->z - u->z * v->y) >> W2V_SHIFT;
		destination->y = (u->z * v->x - u->x * v->z) >> W2V_SHIFT;
		destination->z = (u->x * v->y - u->y * v->x) >> W2V_SHIFT;
	}

	void phd_GetMatrixAngles(int* matrix, float* angle)
	{
		angle[0] = atan2(sqrt(pow(matrix[M22], 2) + pow(matrix[M02], 2)), matrix[M12]);
		if (matrix[M12] >= 0 && angle[0] > 0 || matrix[M12] < 0 && angle[0] < 0)
			angle[0] = -angle[0];

		angle[1] = atan2(matrix[M22], matrix[M02]);
		angle[2] = atan2(matrix[M00] * cos(angle[1]) - matrix[M20] * sin(angle[1]), matrix[M21] * sin(angle[1]) - matrix[M01] * cos(angle[1]));
	}

	void RopeControl(short itemNumber)
	{
		auto* item = &g_Level.Items[itemNumber];
		auto* rope = &Ropes[item->TriggerFlags];

		if (TriggerActive(item))
		{
			rope->active = 1;
			RopeDynamics(rope);
		}
		else
			rope->active = 0;
	}

	void RopeCollision(short itemNumber, ItemInfo* laraItem, CollisionInfo* coll)
	{
		auto* laraInfo = GetLaraInfo(laraItem);
		auto* ropeItem = &g_Level.Items[itemNumber];
		auto* rope = &Ropes[ropeItem->TriggerFlags];
		
		if (TrInput & IN_ACTION &&
			laraInfo->Control.HandStatus == HandStatus::Free &&
			(laraItem->Animation.ActiveState == LS_REACH || laraItem->Animation.ActiveState == LS_JUMP_UP) &&
			laraItem->Animation.IsAirborne &&
			laraItem->Animation.VerticalVelocity > 0&&
			rope->active)
		{
			auto* frame = GetBoundsAccurate(laraItem);

			int segment = RopeNodeCollision(
				rope,
				laraItem->Pose.Position.x,
				laraItem->Pose.Position.y + frame->Y1 + CLICK(2),
				laraItem->Pose.Position.z + frame->Z2 * cos(laraItem->Pose.Orientation.y),
				laraItem->Animation.ActiveState == LS_REACH ? 128 : 320);

			if (segment >= 0)
			{
				if (laraItem->Animation.ActiveState == LS_REACH)
				{
					laraItem->Animation.AnimNumber = LA_REACH_TO_ROPE_SWING;
					laraItem->Animation.ActiveState = LS_ROPE_SWING;
					laraInfo->Control.Rope.Frame = g_Level.Anims[LA_ROPE_SWING].frameBase + 32 << 8;
					laraInfo->Control.Rope.DFrame = g_Level.Anims[LA_ROPE_SWING].frameBase + 60 << 8;
				}
				else
				{
					laraItem->Animation.AnimNumber = LA_JUMP_UP_TO_ROPE_START;
					laraItem->Animation.ActiveState = LS_ROPE_IDLE;
				}

				laraItem->Animation.FrameNumber = g_Level.Anims[laraItem->Animation.AnimNumber].frameBase;
				laraItem->Animation.VerticalVelocity = 0;
				laraItem->Animation.IsAirborne = false;

				laraInfo->Control.HandStatus = HandStatus::Busy;
				laraInfo->Control.Rope.Ptr = ropeItem->TriggerFlags;
				laraInfo->Control.Rope.Segment = segment;
				laraInfo->Control.Rope.Y = laraItem->Pose.Orientation.y;

				DelAlignLaraToRope(laraItem);

				CurrentPendulum.velocity = Vector3Int();

				ApplyVelocityToRope(segment, laraItem->Pose.Orientation.y, 16 * laraItem->Animation.Velocity);
			}
		}
	}

	void RopeDynamics(ROPE_STRUCT* rope)
	{
		PENDULUM* pendulumPointer;
		Vector3Int vec, vec2;

		int flag = 0;
		if (rope->coiled)
		{
			--rope->coiled;
			if (!rope->coiled)
			{
				for (int i = 0; i < ROPE_SEGMENTS; ++i)
					rope->velocity[i].y = 0;
			}
		}

		if (Lara.Control.Rope.Ptr != -1 && rope == &Ropes[Lara.Control.Rope.Ptr])
		{
			pendulumPointer = &CurrentPendulum;
			if (CurrentPendulum.node != Lara.Control.Rope.Segment + 1)
			{
				SetPendulumPoint(rope, Lara.Control.Rope.Segment + 1);
				flag = 1;
			}
		}
		else
		{
			pendulumPointer = &AlternatePendulum;
			if (Lara.Control.Rope.Ptr == -1 && CurrentPendulum.rope)
			{
				for (int i = 0; i < CurrentPendulum.node; i++)
					CurrentPendulum.rope->velocity[i] = CurrentPendulum.rope->velocity[CurrentPendulum.node];

				CurrentPendulum.position = Vector3Int();
				CurrentPendulum.velocity = Vector3Int();

				CurrentPendulum.node = -1;
				CurrentPendulum.rope = NULL;
			}
		}

		if (Lara.Control.Rope.Ptr != -1)
		{
			vec = pendulumPointer->position - rope->segment[0];
			NormaliseRopeVector(&vec);

			for (int i = pendulumPointer->node; i >= 0; --i)
			{
				rope->segment[i].x = rope->meshSegment[i - 1].x + ((int64_t)rope->segmentLength * vec.x >> FP_SHIFT);
				rope->segment[i].y = rope->meshSegment[i - 1].y + ((int64_t)rope->segmentLength * vec.y >> FP_SHIFT);
				rope->segment[i].z = rope->meshSegment[i - 1].z + ((int64_t)rope->segmentLength * vec.z >> FP_SHIFT);

				rope->velocity[i] = Vector3Int();
			}

			if (flag)
			{
				vec2 = pendulumPointer->position - rope->segment[pendulumPointer->node];
				rope->segment[pendulumPointer->node] = pendulumPointer->position;
				
				for (int i = pendulumPointer->node; i < ROPE_SEGMENTS; ++i)
				{
					rope->segment[i] -= vec2;
					rope->velocity[i] = Vector3Int();
				}
			}

			ModelRigidRope(
				rope, 
				pendulumPointer,
				&rope->velocity[0], 
				&pendulumPointer->velocity,
				rope->segmentLength * pendulumPointer->node);
		
			pendulumPointer->velocity.y += 6 << FP_SHIFT;

			pendulumPointer->position += pendulumPointer->velocity;

			pendulumPointer->velocity.x -= pendulumPointer->velocity.x >> 8;
			pendulumPointer->velocity.z -= pendulumPointer->velocity.z >> 8;
		}

		for (int i = pendulumPointer->node; i < ROPE_SEGMENTS - 1; ++i)
			ModelRigid(
				&rope->segment[i],
				&rope->segment[i + 1],
				&rope->velocity[i],
				&rope->velocity[i + 1], 
				rope->segmentLength);

		for (int i = 0; i < ROPE_SEGMENTS; ++i)
			rope->segment[i] += rope->velocity[i];

		for (int i = pendulumPointer->node; i < ROPE_SEGMENTS; ++i)
		{
			rope->velocity[i].y += 3 << FP_SHIFT;

			if (pendulumPointer->rope)
			{
				rope->velocity[i].x -= rope->velocity[i].x >> 4;
				rope->velocity[i].z -= rope->velocity[i].z >> 4;
			}
			else
			{
				rope->velocity[i].x -= rope->velocity[i].x >> 4;
				rope->velocity[i].z -= rope->velocity[i].z >> 4;
			}
		}

		rope->segment[0] = Vector3Int();
		rope->velocity[0] = Vector3Int();

		for (INT i = 0; i < ROPE_SEGMENTS - 1; ++i)
		{
			rope->normalisedSegment[i] = rope->segment[i + 1] - rope->segment[i];
			NormaliseRopeVector(&rope->normalisedSegment[i]);
		}

		if (Lara.Control.Rope.Ptr != -1 && rope != &Ropes[Lara.Control.Rope.Ptr])
		{
			rope->meshSegment[0] = rope->segment[0];

			rope->meshSegment[1].x = rope->segment[0].x + ((int64_t)rope->segmentLength * rope->normalisedSegment[0].x >> FP_SHIFT);
			rope->meshSegment[1].y = rope->segment[0].y + ((int64_t)rope->segmentLength * rope->normalisedSegment[0].y >> FP_SHIFT);
			rope->meshSegment[1].z = rope->segment[0].z + ((int64_t)rope->segmentLength * rope->normalisedSegment[0].z >> FP_SHIFT);

			for (int i = 2; i < ROPE_SEGMENTS; i++)
			{
				rope->meshSegment[i].x = rope->meshSegment[i - 1].x + ((int64_t)rope->segmentLength * rope->normalisedSegment[i - 1].x >> FP_SHIFT);
				rope->meshSegment[i].y = rope->meshSegment[i - 1].y + ((int64_t)rope->segmentLength * rope->normalisedSegment[i - 1].y >> FP_SHIFT);
				rope->meshSegment[i].z = rope->meshSegment[i - 1].z + ((int64_t)rope->segmentLength * rope->normalisedSegment[i - 1].z >> FP_SHIFT);
			}
		}
		else
		{
			rope->meshSegment[pendulumPointer->node] = rope->segment[pendulumPointer->node];

			rope->meshSegment[pendulumPointer->node + 1].x = rope->segment[pendulumPointer->node].x + ((int64_t)rope->segmentLength * rope->normalisedSegment[pendulumPointer->node].x >> FP_SHIFT);
			rope->meshSegment[pendulumPointer->node + 1].y = rope->segment[pendulumPointer->node].y + ((int64_t)rope->segmentLength * rope->normalisedSegment[pendulumPointer->node].y >> FP_SHIFT);
			rope->meshSegment[pendulumPointer->node + 1].z = rope->segment[pendulumPointer->node].z + ((int64_t)rope->segmentLength * rope->normalisedSegment[pendulumPointer->node].z >> FP_SHIFT);

			for (int i = pendulumPointer->node + 1; i < ROPE_SEGMENTS - 1; ++i)
			{
				rope->meshSegment[i + 1].x = rope->meshSegment[i].x + ((int64_t)rope->segmentLength * rope->normalisedSegment[i].x >> FP_SHIFT);
				rope->meshSegment[i + 1].y = rope->meshSegment[i].y + ((int64_t)rope->segmentLength * rope->normalisedSegment[i].y >> FP_SHIFT);
				rope->meshSegment[i + 1].z = rope->meshSegment[i].z + ((int64_t)rope->segmentLength * rope->normalisedSegment[i].z >> FP_SHIFT);
			}

			for (int i = 0; i < pendulumPointer->node; i++)
				rope->meshSegment[i] = rope->segment[i];
		}
	}

	int RopeNodeCollision(ROPE_STRUCT* rope, int x, int y, int z, int radius)
	{
		for (int i = 0; i < ROPE_SEGMENTS - 2; ++i)
		{
			if (y > rope->position.y + (rope->meshSegment[i].y >> FP_SHIFT) &&
				y < rope->position.y + (rope->meshSegment[i + 1].y >> FP_SHIFT))
			{
				int dx = x - ((rope->meshSegment[i + 1].x + rope->meshSegment[i].x) >> (FP_SHIFT + 1)) - rope->position.x;
				int dy = y - ((rope->meshSegment[i + 1].y + rope->meshSegment[i].y) >> (FP_SHIFT + 1)) - rope->position.y;
				int dz = z - ((rope->meshSegment[i + 1].z + rope->meshSegment[i].z) >> (FP_SHIFT + 1)) - rope->position.z;

				if (pow(dx, 2) + pow(dy, 2) + pow(dz, 2) < pow(radius + 64, 2))
					return i;
			}
		}

		return -1;
	}

	void ApplyVelocityToRope(int node, short angle, short n)
	{
		SetPendulumVelocity(n * sin(angle) * SECTOR(4), 0, n * cos(angle) * SECTOR(4));
	}

	void SetPendulumVelocity(int x, int y, int z)
	{
		int node = 2 * (CurrentPendulum.node >> 1);
		if (node < ROPE_SEGMENTS)
		{
			int val = 4096 / (ROPE_SEGMENTS - node) * 256;

			x = (int64_t)val * x >> FP_SHIFT;
			y = (int64_t)val * y >> FP_SHIFT;
			z = (int64_t)val * z >> FP_SHIFT;
		}

		CurrentPendulum.velocity.x += x;
		CurrentPendulum.velocity.y += y;
		CurrentPendulum.velocity.z += z;
	}

	void SetPendulumPoint(ROPE_STRUCT* rope, int node)
	{
		CurrentPendulum.position = rope->segment[node];

		if (CurrentPendulum.node == -1)
			CurrentPendulum.velocity += rope->velocity[node];

		CurrentPendulum.node = node;
		CurrentPendulum.rope = rope;
	}

	void ModelRigidRope(ROPE_STRUCT* rope, PENDULUM* pendulumPointer, Vector3Int* ropeVelocity, Vector3Int* pendulumVelocity, int value)
	{
		auto vec = pendulumPointer->position + *pendulumVelocity - rope->segment[0];

		int result = 65536 * sqrt(abs(pow(vec.x >> FP_SHIFT, 2) + pow(vec.y >> FP_SHIFT, 2) + pow(vec.z >> FP_SHIFT, 2))) - value;
		NormaliseRopeVector(&vec);

		pendulumVelocity->x -= (int64_t)result * vec.x >> FP_SHIFT;
		pendulumVelocity->y -= (int64_t)result * vec.y >> FP_SHIFT;
		pendulumVelocity->z -= (int64_t)result * vec.z >> FP_SHIFT;
	}

	void ModelRigid(Vector3Int* segment, Vector3Int* nextSegment, Vector3Int* velocity, Vector3Int* nextVelocity, int length)
	{
		auto vec = *nextSegment + *nextVelocity - *segment - *velocity;

		int result = (65536 * sqrt(abs(pow(vec.x >> FP_SHIFT, 2) + pow(vec.y >> FP_SHIFT, 2) + pow(vec.z >> FP_SHIFT, 2))) - length) / 2;
		NormaliseRopeVector(&vec);

		vec.x = (int64_t)result * vec.x >> FP_SHIFT;
		vec.y = (int64_t)result * vec.y >> FP_SHIFT;
		vec.z = (int64_t)result * vec.z >> FP_SHIFT;

		*velocity += vec;
		*nextVelocity -= vec;
	}

	void UpdateRopeSwing(ItemInfo* item)
	{
		if (Lara.Control.Rope.MaxXForward > 9000)
			Lara.Control.Rope.MaxXForward = 9000;

		if (Lara.Control.Rope.MaxXBackward > 9000)
			Lara.Control.Rope.MaxXBackward = 9000;

		if (Lara.Control.Rope.Direction)
		{
			if (item->Pose.Orientation.x > 0.0f && item->Pose.Orientation.x - Lara.Control.Rope.LastX < Angle::DegToRad(-0.55))
			{
				Lara.Control.Rope.ArcFront = Lara.Control.Rope.LastX;
				Lara.Control.Rope.Direction = 0;
				Lara.Control.Rope.MaxXBackward = 0;
				int frame = 15 * Angle::RadToShrt(Lara.Control.Rope.MaxXForward) / 18000 + g_Level.Anims[LA_ROPE_SWING].frameBase + 47 << 8;

				if (frame > Lara.Control.Rope.DFrame)
				{
					Lara.Control.Rope.DFrame = frame;
					RopeSwing = 1;
				}
				else
					RopeSwing = 0;

				SoundEffect(SFX_TR4_LARA_ROPE_CREAK, &item->Pose);
			}
			else if (Lara.Control.Rope.LastX < 0 && Lara.Control.Rope.Frame == Lara.Control.Rope.DFrame)
			{
				RopeSwing = 0;
				Lara.Control.Rope.DFrame = 15 * Angle::RadToShrt(Lara.Control.Rope.MaxXBackward) / 18000 + g_Level.Anims[LA_ROPE_SWING].frameBase + 47 << 8;
				Lara.Control.Rope.FrameRate = 15 * Lara.Control.Rope.MaxXBackward / 9000 + 1;
			}
			else if (Lara.Control.Rope.FrameRate < 512)
			{
				int num = RopeSwing ? 31 : 7;
				Lara.Control.Rope.FrameRate += num * Lara.Control.Rope.MaxXBackward / 9000 + 1;
			}
		}
		else
		{
			if (item->Pose.Orientation.x < 0.0f && item->Pose.Orientation.x - Lara.Control.Rope.LastX < Angle::DegToRad(0.55))
			{
				Lara.Control.Rope.ArcBack = Lara.Control.Rope.LastX;
				Lara.Control.Rope.Direction = 1;
				Lara.Control.Rope.MaxXForward = 0;

				int frame = g_Level.Anims[LA_ROPE_SWING].frameBase - 15 * Angle::RadToShrt(Lara.Control.Rope.MaxXBackward) / 18000 + 17 << 8; // TODO
				if (frame < Lara.Control.Rope.DFrame)
				{
					Lara.Control.Rope.DFrame = frame;
					RopeSwing = 1;
				}
				else
					RopeSwing = 0;

				SoundEffect(SFX_TR4_LARA_ROPE_CREAK, &item->Pose);
			}
			else if (Lara.Control.Rope.LastX > 0 && Lara.Control.Rope.Frame == Lara.Control.Rope.DFrame)
			{
				RopeSwing = 0;
				Lara.Control.Rope.DFrame = g_Level.Anims[LA_ROPE_SWING].frameBase - 15 * Angle::RadToShrt(Lara.Control.Rope.MaxXForward) / 18000 + 17 << 8;
				Lara.Control.Rope.FrameRate = 15 * Lara.Control.Rope.MaxXForward / 9000 + 1;
			}
			else if (Lara.Control.Rope.FrameRate < 512)
			{
				int num = RopeSwing ? 31 : 7;
				Lara.Control.Rope.FrameRate += num * Lara.Control.Rope.MaxXForward / 9000 + 1;
			}
		}

		Lara.Control.Rope.LastX = item->Pose.Orientation.x;
		if (Lara.Control.Rope.Direction)
		{
			if (item->Pose.Orientation.x > Lara.Control.Rope.MaxXForward)
				Lara.Control.Rope.MaxXForward = item->Pose.Orientation.x;
		}
		else
		{
			if (item->Pose.Orientation.x < -Lara.Control.Rope.MaxXBackward)
				Lara.Control.Rope.MaxXBackward = abs(item->Pose.Orientation.x);
		}
	}

	bool RopeSwingCollision(ItemInfo* item, CollisionInfo* coll)
	{
		auto* lara = GetLaraInfo(item);

		coll->Setup.Mode = CollisionProbeMode::FreeForward;
		coll->Setup.ForwardAngle = lara->Control.Rope.Direction ? item->Pose.Orientation.y : -item->Pose.Orientation.y;
		GetCollisionInfo(coll, item);

		bool stumble = (coll->CollisionType != CollisionType::CT_NONE || coll->HitStatic);

		if (stumble || 
			TestEnvironment(RoomEnvFlags::ENV_FLAG_WATER, item->RoomNumber) ||
			TestEnvironment(RoomEnvFlags::ENV_FLAG_SWAMP, item->RoomNumber))
		{
			item->Pose.Position = coll->Setup.OldPosition;
			FallFromRope(item, stumble);
			return true;
		}

		return false;
	}

	void JumpOffRope(ItemInfo* item)
	{
		if (Lara.Control.Rope.Ptr != -1)
		{
			if (item->Pose.Orientation.x >= 0.0f)
			{
				item->Animation.VerticalVelocity = -112;
				item->Animation.Velocity = item->Pose.Orientation.x / 128;
			}
			else
			{
				item->Animation.Velocity = 0;
				item->Animation.VerticalVelocity = -20;
			}

			item->Pose.Orientation.x = 0.0f;
			item->Animation.IsAirborne = true;

			Lara.Control.HandStatus = HandStatus::Free;

			if (item->Animation.FrameNumber - g_Level.Anims[LA_ROPE_SWING].frameBase > 42)
				item->Animation.AnimNumber = LA_ROPE_SWING_TO_REACH_1;
			else if (item->Animation.FrameNumber - g_Level.Anims[LA_ROPE_SWING].frameBase > 21)
				item->Animation.AnimNumber = LA_ROPE_SWING_TO_REACH_2;
			else
				item->Animation.AnimNumber = LA_ROPE_SWING_TO_REACH_3;

			item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
			item->Animation.ActiveState = LS_REACH;
			item->Animation.TargetState = LS_REACH;
			Lara.Control.Rope.Ptr = -1;
		}
	}

	void FallFromRope(ItemInfo* item, bool stumble)
	{
		item->Animation.Velocity = abs(CurrentPendulum.velocity.x >> FP_SHIFT) + abs(CurrentPendulum.velocity.z >> FP_SHIFT) >> 1;
		item->Pose.Orientation.x = 0.0f;
		item->Pose.Position.y += 320;

		SetAnimation(item, stumble ? LA_JUMP_WALL_SMASH_START : LA_FALL_START);

		item->Animation.IsAirborne = true;
		item->Animation.VerticalVelocity = 0;

		auto* lara = GetLaraInfo(item);
		lara->Control.HandStatus = HandStatus::Free;
		lara->Control.Rope.Ptr = -1;

		if (stumble)
		{
			item->Animation.Velocity = -item->Animation.Velocity;
			DoDamage(item, 0);
		}
	}

	void LaraClimbRope(ItemInfo* item, CollisionInfo* coll)
	{
		if (!(TrInput & IN_ACTION))
			FallFromRope(item);
		else
		{
			Camera.targetAngle = Angle::DegToRad(30.0f);

			if (Lara.Control.Rope.Count)
			{
				if (!Lara.Control.Rope.Flag)
				{
					--Lara.Control.Rope.Count;
					Lara.Control.Rope.Offset += Lara.Control.Rope.DownVel;

					if (!Lara.Control.Rope.Count)
						Lara.Control.Rope.Flag = 1;

					return;
				}
			}
			else
			{
				if (!Lara.Control.Rope.Flag)
				{
					auto* rope = &Ropes[Lara.Control.Rope.Ptr];
					Lara.Control.Rope.Offset = 0;
					Lara.Control.Rope.DownVel = (unsigned int)(rope->meshSegment[Lara.Control.Rope.Segment + 1].y - rope->meshSegment[Lara.Control.Rope.Segment].y) >> 17;
					Lara.Control.Rope.Count = 0;
					Lara.Control.Rope.Offset += Lara.Control.Rope.DownVel;
					Lara.Control.Rope.Flag = 1;
					return;
				}
			}

			if (item->Animation.AnimNumber == LA_ROPE_DOWN && item->Animation.FrameNumber == g_Level.Anims[item->Animation.AnimNumber].frameEnd)
			{
				SoundEffect(SFX_TR4_LARA_POLE_SLIDE_LOOP, &LaraItem->Pose);
				item->Animation.FrameNumber = g_Level.Anims[item->Animation.AnimNumber].frameBase;
				Lara.Control.Rope.Flag = 0;
				++Lara.Control.Rope.Segment;
				Lara.Control.Rope.Offset = 0;
			}

			if (!(TrInput & IN_BACK) || Lara.Control.Rope.Segment >= 21)
				item->Animation.TargetState = LS_ROPE_IDLE;
		}
	}

	void DelAlignLaraToRope(ItemInfo* item)
	{
		auto vec = Vector3Int(4096, 0, 0);

		auto* frame = (ANIM_FRAME*)GetBestFrame(item);
		float ropeY = Lara.Control.Rope.Y - Angle::DegToRad(90.0f);
		auto* rope = &Ropes[Lara.Control.Rope.Ptr];

		Vector3Int pos, pos2;
		GetRopePos(rope, (Lara.Control.Rope.Segment - 1 << 7) + frame->offsetY, &pos.x, &pos.y, &pos.z);
		GetRopePos(rope, (Lara.Control.Rope.Segment - 1 << 7) + frame->offsetY - 192, &pos2.x, &pos2.y, &pos2.z);

		Vector3Int diff;
		diff.x = pos.x - pos2.x << 16;
		diff.y = pos.y - pos2.y << 16;
		diff.z = pos.z - pos2.z << 16;
		NormaliseRopeVector(&diff);
		diff.x >>= 2;
		diff.y >>= 2;
		diff.z >>= 2;

		Vector3Int vec2;
		ScaleVector(&diff, DotProduct(&vec, &diff), &vec2);
		vec2 = vec - vec2;

		auto vec3 = vec2;
		auto vec4 = vec2;

		auto diff2 = diff;

		ScaleVector(&vec3, 16384 * cos(ropeY), &vec3);
		ScaleVector(&diff2, DotProduct(&diff2, &vec2), &diff2);
		ScaleVector(&diff2, 4096 - 16384 * cos(ropeY), &diff2);

		CrossProduct(&diff, &vec2, &vec4);
		ScaleVector(&vec4, 16384 * sin(ropeY), &vec4);
		diff2.x += vec3.x;
		diff2.y += vec3.y;
		diff2.z += vec3.z;

		vec2.x = diff2.x + vec4.x << 16;
		vec2.y = diff2.y + vec4.y << 16;
		vec2.z = diff2.z + vec4.z << 16;
		NormaliseRopeVector(&vec2);
		vec2.x >>= 2;
		vec2.y >>= 2;
		vec2.z >>= 2;

		Vector3Int vec5;
		CrossProduct(&diff, &vec2, &vec5);

		vec5.x <<= 16;
		vec5.y <<= 16;
		vec5.z <<= 16;
		NormaliseRopeVector(&vec5);
		vec5.x >>= 2;
		vec5.y >>= 2;
		vec5.z >>= 2;

		int matrix[12];
		matrix[M00] = vec5.x;
		matrix[M01] = diff.x;
		matrix[M02] = vec2.x;
		matrix[M10] = vec5.y;
		matrix[M11] = diff.y;
		matrix[M12] = vec2.y;
		matrix[M20] = vec5.z;
		matrix[M21] = diff.z;
		matrix[M22] = vec2.z;

		float angle[3];
		phd_GetMatrixAngles(matrix, angle);

		item->Pose.Position.x = rope->position.x + (rope->meshSegment[Lara.Control.Rope.Segment].x >> FP_SHIFT);
		item->Pose.Position.y = rope->position.y + (rope->meshSegment[Lara.Control.Rope.Segment].y >> FP_SHIFT) + Lara.Control.Rope.Offset;
		item->Pose.Position.z = rope->position.z + (rope->meshSegment[Lara.Control.Rope.Segment].z >> FP_SHIFT);

		Matrix rotMatrix = Matrix::CreateFromYawPitchRoll(
			angle[1],
			angle[0],
			angle[2]
		);

		rotMatrix = rotMatrix.Transpose();

		item->Pose.Position.x += -112 * rotMatrix.m[0][2];
		item->Pose.Position.y += -112 * rotMatrix.m[1][2];
		item->Pose.Position.z += -112 * rotMatrix.m[2][2];

		item->Pose.Orientation = EulerAngles(angle[0], angle[1], angle[2]);
	}
}
