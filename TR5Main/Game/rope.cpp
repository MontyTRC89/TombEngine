#include "rope.h"
#include "..\Global\global.h"
#include "draw.h"
#include "laramisc.h"
#include "lara.h"
#include "..\Specific\level.h"
#include "../Specific/input.h"
#include "control.h"

PENDULUM CurrentPendulum;
PENDULUM AlternatePendulum;
ROPE_STRUCT Ropes[12];
int NumRopes;

void InitialiseRope(short itemNumber) // (F) (D)
{
	PHD_VECTOR itemPos;

	ITEM_INFO* item = &Items[itemNumber];
	short roomNumber = item->roomNumber;

	itemPos.x = item->pos.xPos;
	itemPos.y = item->pos.yPos;
	itemPos.z = item->pos.zPos;

	FLOOR_INFO* floor = GetFloor(itemPos.x, itemPos.y, itemPos.z, &roomNumber);
	itemPos.y = GetCeiling(floor, itemPos.x, itemPos.y, itemPos.z);
	
	PHD_VECTOR pos;
	pos.x = 0;
	pos.y = 16384;
	pos.z = 0;
	
	ROPE_STRUCT* rope = &Ropes[NumRopes];
	
	PrepareRope(rope, &itemPos, &pos, 128, item);
	
	item->triggerFlags = NumRopes;
	NumRopes++;
}

void PrepareRope(ROPE_STRUCT* rope, PHD_VECTOR* pos1, PHD_VECTOR* pos2, int length, ITEM_INFO* item) // (F) (D)
{
	rope->position = *pos1;
	rope->segmentLength = length << 16;

	pos2->x <<= 16;
	pos2->y <<= 16;
	pos2->z <<= 16;

	NormaliseRopeVector(pos2);

	if (item->triggerFlags == -1)
		rope->coiled = 30;
	else
		rope->coiled = 0;

	int l = 0;
	int sum = 0;
	int il = 3145728;

	for (int i = 0; i < 24; ++i)
	{
		rope->segment[i].x = (int64_t) sum * pos2->x >> 16;
		rope->segment[i].y = (int64_t) sum * pos2->y >> 16;
		rope->segment[i].z = (int64_t) sum * pos2->z >> 16;

		rope->velocity[i].x = 0;
		rope->velocity[i].y = 0;
		rope->velocity[i].z = 0;

		if (item->triggerFlags == -1)
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

PHD_VECTOR* NormaliseRopeVector(PHD_VECTOR* vec) // (F) (D)
{
	int x = vec->x >> 16;
	int y = vec->y >> 16;
	int z = vec->z >> 16;

	if (!x && !y && !z)
		return vec;

	int length = SQUARE(x) + SQUARE(y) + SQUARE(z);
	if (length < 0)
		length = -length;

	length = 65536 / sqrt(length);
	
	vec->x = (int64_t) length * vec->x >> 16;
	vec->y = (int64_t) length * vec->y >> 16;
	vec->z = (int64_t) length * vec->z >> 16;
	return vec;
}

void _0x0046D130(ROPE_STRUCT* rope, int segmentFrame, int* x, int* y, int* z) // (F) (D)
{
	int segment;
	short frame;

	segment = segmentFrame >> 7;
	frame = segmentFrame & 0x7F;
	*x = (rope->normalisedSegment[segment].x * frame >> 16) + (rope->meshSegment[segment].x >> 16) + rope->position.x;
	*y = (rope->normalisedSegment[segment].y * frame >> 16) + (rope->meshSegment[segment].y >> 16) + rope->position.y;
	*z = (rope->normalisedSegment[segment].z * frame >> 16) + (rope->meshSegment[segment].z >> 16) + rope->position.z;
}

int DotProduct(PHD_VECTOR* u, PHD_VECTOR* v) // (F) (D)
{
	return u->x * v->x + u->y * v->y + u->z * v->z >> W2V_SHIFT;
}

void ScaleVector(PHD_VECTOR* u, int c, PHD_VECTOR* destination) // (F) (D)
{
	destination->x = c * u->x >> W2V_SHIFT;
	destination->y = c * u->y >> W2V_SHIFT;
	destination->z = c * u->z >> W2V_SHIFT;
}

void CrossProduct(PHD_VECTOR* u, PHD_VECTOR* v, PHD_VECTOR* destination) // (F) (D)
{
	destination->x = u->y * v->z - u->z * v->y >> W2V_SHIFT;
	destination->y = u->z * v->x - u->x * v->z >> W2V_SHIFT;
	destination->z = u->x * v->y - u->y * v->x >> W2V_SHIFT;
}

void _0x0046D420(int* matrix, short* angle) // (F) (D)
{
	angle[0] = phd_atan(sqrt(SQUARE(matrix[M22]) + SQUARE(matrix[M02])), matrix[M12]);
	if (matrix[M12] >= 0 && angle[0] > 0 || matrix[M12] < 0 && angle[0] < 0)
		angle[0] = -angle[0];
	angle[1] = phd_atan(matrix[M22], matrix[M02]);
	angle[2] = phd_atan(matrix[M00] * phd_cos(angle[1]) - matrix[M20] * phd_sin(angle[1]), matrix[M21] * phd_sin(angle[1]) - matrix[M01] * phd_cos(angle[1]));
}

void RopeControl(short itemNumber) // (F) (D)
{
	ITEM_INFO* item;
	ROPE_STRUCT* rope;

	item = &Items[itemNumber];
	rope = &Ropes[item->triggerFlags];
	if (TriggerActive(item))
	{
		rope->active = 1;
		RopeDynamics(rope);
	}
	else
	{
		rope->active = 0;
	}
}

void RopeCollision(short itemNumber, ITEM_INFO* l, COLL_INFO* coll) // (F) (D)
{
	ITEM_INFO* item;
	ROPE_STRUCT* rope;
	ANIM_FRAME* frame;
	int segment;

	item = &Items[itemNumber];
	rope = &Ropes[item->triggerFlags];
	if (TrInput & IN_ACTION && Lara.gunStatus == LG_NO_ARMS && (l->currentAnimState == STATE_LARA_REACH || l->currentAnimState == STATE_LARA_JUMP_UP) && l->gravityStatus && l->fallspeed > 0 && rope->active)
	{
		frame = (ANIM_FRAME*) GetBoundsAccurate(l);
		segment = _0x0046D200(rope, l->pos.xPos, l->pos.yPos + frame->MinY + 512, l->pos.zPos + (frame->MaxZ * phd_cos(l->pos.yRot) >> W2V_SHIFT), l->currentAnimState == STATE_LARA_REACH ? 128 : 320);
		if (segment >= 0)
		{
			if (l->currentAnimState == STATE_LARA_REACH)
			{
				l->animNumber = ANIMATION_LARA_ROPE_JUMP_TO_GRAB;
				l->currentAnimState = STATE_LARA_ROPE_SWING;
				Lara.ropeFrame = Anims[ANIMATION_LARA_ROPE_SWING_FORWARD_SEMIHARD].frameBase + 32 << 8;
				Lara.ropeDFrame = Anims[ANIMATION_LARA_ROPE_SWING_FORWARD_SEMIHARD].frameBase + 60 << 8;
			}
			else
			{
				l->animNumber = ANIMATION_LARA_MONKEY_TO_ROPE_BEGIN;
				l->currentAnimState = STATE_LARA_ROPE_IDLE;
			}
			l->frameNumber = Anims[l->animNumber].frameBase;
			l->gravityStatus = false;
			l->fallspeed = 0;
			Lara.gunStatus = LG_HANDS_BUSY;
			Lara.ropePtr = item->triggerFlags;
			Lara.ropeSegment = segment;
			Lara.ropeY = l->pos.yRot;
			DelAlignLaraToRope(l);
			CurrentPendulum.Velocity.x = 0;
			CurrentPendulum.Velocity.y = 0;
			CurrentPendulum.Velocity.z = 0;
			ApplyVelocityToRope(segment, l->pos.yRot, 16 * LaraItem->speed);
		}
	}
}

void RopeDynamics(ROPE_STRUCT* rope) // (F) (D)
{
	int flag, i;
	PENDULUM* pendulumPointer;
	PHD_VECTOR vec, vec2;

	flag = 0;
	if (rope->coiled)
	{
		--rope->coiled;
		if (!rope->coiled)
		{
			for (i = 0; i < 24; ++i)
				rope->velocity[i].y = 0;
		}
	}
	if (rope == &Ropes[Lara.ropePtr])
	{
		pendulumPointer = &CurrentPendulum;
		if (CurrentPendulum.node != Lara.ropeSegment + 1)
		{
			_0x0046E1C0(rope, Lara.ropeSegment + 1);
			flag = 1;
		}
	}
	else
	{
		pendulumPointer = &AlternatePendulum;
		if (Lara.ropePtr == -1 && CurrentPendulum.Rope)
		{
			for (i = 0; i < CurrentPendulum.node; ++i)
			{
				CurrentPendulum.Rope->velocity[i].x = CurrentPendulum.Rope->velocity[CurrentPendulum.node].x;
				CurrentPendulum.Rope->velocity[i].y = CurrentPendulum.Rope->velocity[CurrentPendulum.node].y;
				CurrentPendulum.Rope->velocity[i].z = CurrentPendulum.Rope->velocity[CurrentPendulum.node].z;
			}
			CurrentPendulum.Position.x = 0;
			CurrentPendulum.Position.y = 0;
			CurrentPendulum.Position.z = 0;
			CurrentPendulum.Velocity.x = 0;
			CurrentPendulum.Velocity.y = 0;
			CurrentPendulum.Velocity.z = 0;
			CurrentPendulum.node = -1;
			CurrentPendulum.Rope = NULL;
		}
	}
	if (Lara.ropePtr != -1)
	{
		vec.x = pendulumPointer->Position.x - rope->segment[0].x;
		vec.y = pendulumPointer->Position.y - rope->segment[0].y;
		vec.z = pendulumPointer->Position.z - rope->segment[0].z;
		NormaliseRopeVector(&vec);
		for (i = pendulumPointer->node; i >= 0; --i)
		{
			rope->segment[i].x = rope->meshSegment[i - 1].x + ((int64_t) rope->segmentLength * vec.x >> 16);
			rope->segment[i].y = rope->meshSegment[i - 1].y + ((int64_t) rope->segmentLength * vec.y >> 16);
			rope->segment[i].z = rope->meshSegment[i - 1].z + ((int64_t) rope->segmentLength * vec.z >> 16);
			rope->velocity[i].x = 0;
			rope->velocity[i].y = 0;
			rope->velocity[i].z = 0;
		}
		if (flag)
		{
			vec2.x = pendulumPointer->Position.x - rope->segment[pendulumPointer->node].x;
			vec2.y = pendulumPointer->Position.y - rope->segment[pendulumPointer->node].y;
			vec2.z = pendulumPointer->Position.z - rope->segment[pendulumPointer->node].z;
			rope->segment[pendulumPointer->node].x = pendulumPointer->Position.x;
			rope->segment[pendulumPointer->node].y = pendulumPointer->Position.y;
			rope->segment[pendulumPointer->node].z = pendulumPointer->Position.z;
			for (i = pendulumPointer->node; i < 24; ++i)
			{
				rope->segment[i].x -= vec2.x;
				rope->segment[i].y -= vec2.y;
				rope->segment[i].z -= vec2.z;
				rope->velocity[i].x = 0;
				rope->velocity[i].y = 0;
				rope->velocity[i].z = 0;
			}
		}
		_0x0046E080(rope, pendulumPointer, &rope->velocity[0], &pendulumPointer->Velocity, rope->segmentLength * pendulumPointer->node);
		pendulumPointer->Velocity.y += 393216;
		pendulumPointer->Position.x += pendulumPointer->Velocity.x;
		pendulumPointer->Position.y += pendulumPointer->Velocity.y;
		pendulumPointer->Position.z += pendulumPointer->Velocity.z;
		pendulumPointer->Velocity.x -= pendulumPointer->Velocity.x >> 8;
		pendulumPointer->Velocity.z -= pendulumPointer->Velocity.z >> 8;
	}
	for (i = pendulumPointer->node; i < 23; ++i)
		_0x0046DF00(&rope->segment[i], &rope->segment[i + 1], &rope->velocity[i], &rope->velocity[i + 1], rope->segmentLength);
	for (i = 0; i < 24; ++i)
	{
		rope->segment[i].x += rope->velocity[i].x;
		rope->segment[i].y += rope->velocity[i].y;
		rope->segment[i].z += rope->velocity[i].z;
	}
	for (i = pendulumPointer->node; i < 24; ++i)
	{
		rope->velocity[i].y += 196608;
		if (pendulumPointer->Rope)
		{
			rope->velocity[i].x -= rope->velocity[i].x >> 4;
			rope->velocity[i].z -= rope->velocity[i].z >> 4;
		}
		else
		{
			rope->velocity[i].x -= rope->velocity[i].x >> 7;
			rope->velocity[i].z -= rope->velocity[i].z >> 7;
		}
	}
	rope->segment[0].x = 0;
	rope->segment[0].y = 0;
	rope->segment[0].z = 0;
	rope->velocity[0].x = 0;
	rope->velocity[0].y = 0;
	rope->velocity[0].z = 0;
	for (i = 0; i < 23; ++i)
	{
		rope->normalisedSegment[i].x = rope->segment[i + 1].x - rope->segment[i].x;
		rope->normalisedSegment[i].y = rope->segment[i + 1].y - rope->segment[i].y;
		rope->normalisedSegment[i].z = rope->segment[i + 1].z - rope->segment[i].z;
		NormaliseRopeVector(&rope->normalisedSegment[i]);
	}
	if (rope != &Ropes[Lara.ropePtr])
	{
		rope->meshSegment[0].x = rope->segment[0].x;
		rope->meshSegment[0].y = rope->segment[0].y;
		rope->meshSegment[0].z = rope->segment[0].z;
		rope->meshSegment[1].x = rope->segment[0].x + ((int64_t) rope->segmentLength * rope->normalisedSegment[0].x >> 16);
		rope->meshSegment[1].y = rope->segment[0].y + ((int64_t) rope->segmentLength * rope->normalisedSegment[0].y >> 16);
		rope->meshSegment[1].z = rope->segment[0].z + ((int64_t) rope->segmentLength * rope->normalisedSegment[0].z >> 16);
		for (i = 2; i < 24; ++i)
		{
			rope->meshSegment[i].x = rope->meshSegment[i - 1].x + ((int64_t) rope->segmentLength * rope->normalisedSegment[i - 1].x >> 16);
			rope->meshSegment[i].y = rope->meshSegment[i - 1].y + ((int64_t) rope->segmentLength * rope->normalisedSegment[i - 1].y >> 16);
			rope->meshSegment[i].z = rope->meshSegment[i - 1].z + ((int64_t) rope->segmentLength * rope->normalisedSegment[i - 1].z >> 16);
		}
	}
	else
	{
		rope->meshSegment[pendulumPointer->node].x = rope->segment[pendulumPointer->node].x;
		rope->meshSegment[pendulumPointer->node].y = rope->segment[pendulumPointer->node].y;
		rope->meshSegment[pendulumPointer->node].z = rope->segment[pendulumPointer->node].z;
		rope->meshSegment[pendulumPointer->node + 1].x = rope->segment[pendulumPointer->node].x + ((int64_t) rope->segmentLength * rope->normalisedSegment[pendulumPointer->node].x >> 16);
		rope->meshSegment[pendulumPointer->node + 1].y = rope->segment[pendulumPointer->node].y + ((int64_t) rope->segmentLength * rope->normalisedSegment[pendulumPointer->node].y >> 16);
		rope->meshSegment[pendulumPointer->node + 1].z = rope->segment[pendulumPointer->node].z + ((int64_t) rope->segmentLength * rope->normalisedSegment[pendulumPointer->node].z >> 16);
		for (i = pendulumPointer->node + 1; i < 23; ++i)
		{
			rope->meshSegment[i + 1].x = rope->meshSegment[i].x + ((int64_t) rope->segmentLength * rope->normalisedSegment[i].x >> 16);
			rope->meshSegment[i + 1].y = rope->meshSegment[i].y + ((int64_t) rope->segmentLength * rope->normalisedSegment[i].y >> 16);
			rope->meshSegment[i + 1].z = rope->meshSegment[i].z + ((int64_t) rope->segmentLength * rope->normalisedSegment[i].z >> 16);
		}
		for (i = 0; i < pendulumPointer->node; ++i)
		{
			rope->meshSegment[i].x = rope->segment[i].x;
			rope->meshSegment[i].y = rope->segment[i].y;
			rope->meshSegment[i].z = rope->segment[i].z;
		}
	}
}

int _0x0046D200(ROPE_STRUCT* rope, int x, int y, int z, int radius) // (F) (D)
{
	int dx, dy, dz;

	for (int i = 0; i < 22; ++i)
	{
		if (y > rope->position.y + (rope->meshSegment[i].y >> 16) && y < rope->position.y + (rope->meshSegment[i + 1].y >> 16))
		{
			dx = x - (rope->meshSegment[i + 1].x + rope->meshSegment[i].x >> 17) - rope->position.x;
			dy = y - (rope->meshSegment[i + 1].y + rope->meshSegment[i].y >> 17) - rope->position.y;
			dz = z - (rope->meshSegment[i + 1].z + rope->meshSegment[i].z >> 17) - rope->position.z;
			if (SQUARE(dx) + SQUARE(dy) + SQUARE(dz) < SQUARE(radius + 64))
				return i;
		}
	}
	return -1;
}

void ApplyVelocityToRope(int node, short angle, short n) // (F) (D)
{
	SetPendulumVelocity(
		(unsigned short) n * phd_sin(angle) >> 2,
		0,
		(unsigned short) n * phd_cos(angle) >> 2); /* @ORIGINAL_BUG: casting n to unsigned short results in the rope glitch */
}

void SetPendulumVelocity(int x, int y, int z) // (F) (D)
{
	int node;

	node = 2 * (CurrentPendulum.node >> 1);
	if (node < 24)
	{
		int val = 4096 / (24 - node) * 256;

		x = (int64_t) val * x >> 16;
		y = (int64_t) val * y >> 16;
		z = (int64_t) val * z >> 16;
	}

	CurrentPendulum.Velocity.x += x;
	CurrentPendulum.Velocity.y += y;
	CurrentPendulum.Velocity.z += z;
}

void _0x0046E1C0(ROPE_STRUCT* rope, int node) // (F) (D)
{
	CurrentPendulum.Position.x = rope->segment[node].x;
	CurrentPendulum.Position.y = rope->segment[node].y;
	CurrentPendulum.Position.z = rope->segment[node].z;
	if (CurrentPendulum.node == -1)
	{
		CurrentPendulum.Velocity.x += rope->velocity[node].x;
		CurrentPendulum.Velocity.y += rope->velocity[node].y;
		CurrentPendulum.Velocity.z += rope->velocity[node].z;
	}
	CurrentPendulum.node = node;
	CurrentPendulum.Rope = rope;
}

void _0x0046E080(ROPE_STRUCT* rope, PENDULUM* pendulumPointer, PHD_VECTOR* ropeVelocity, PHD_VECTOR* pendulumVelocity, int value) // (F) (D)
{
	PHD_VECTOR vec;
	int result;

	vec.x = pendulumPointer->Position.x + pendulumVelocity->x - rope->segment[0].x;
	vec.y = pendulumPointer->Position.y + pendulumVelocity->y - rope->segment[0].y;
	vec.z = pendulumPointer->Position.z + pendulumVelocity->z - rope->segment[0].z;
	result = 65536 * sqrt(abs(SQUARE(vec.x >> 16) + SQUARE(vec.y >> 16) + SQUARE(vec.z >> 16))) - value;
	NormaliseRopeVector(&vec);
	pendulumVelocity->x -= (int64_t) result * vec.x >> 16;
	pendulumVelocity->y -= (int64_t) result * vec.y >> 16;
	pendulumVelocity->z -= (int64_t) result * vec.z >> 16;
}

void _0x0046DF00(PHD_VECTOR* segment, PHD_VECTOR* nextSegment, PHD_VECTOR* velocity, PHD_VECTOR* nextVelocity, int length) // (F) (D)
{
	PHD_VECTOR vec;
	int result;

	vec.x = nextSegment->x + nextVelocity->x - segment->x - velocity->x;
	vec.y = nextSegment->y + nextVelocity->y - segment->y - velocity->y;
	vec.z = nextSegment->z + nextVelocity->z - segment->z - velocity->z;
	result = (65536 * sqrt(abs(SQUARE(vec.x >> 16) + SQUARE(vec.y >> 16) + SQUARE(vec.z >> 16))) - length) / 2;
	NormaliseRopeVector(&vec);
	vec.x = (int64_t) result * vec.x >> 16;
	vec.y = (int64_t) result * vec.y >> 16;
	vec.z = (int64_t) result * vec.z >> 16;
	velocity->x += vec.x;
	velocity->y += vec.y;
	velocity->z += vec.z;
	nextVelocity->x -= vec.x;
	nextVelocity->y -= vec.y;
	nextVelocity->z -= vec.z;
}
