#include "rope.h"
#include "..\Global\global.h"

void InitialiseRope(short itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	__int16 roomNumber = item->roomNumber;

	FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
	int ceiling = GetCeiling(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);
	
	PHD_VECTOR pos;
	pos.x = 0;
	pos.y = 0;
	pos.z = 16384;
	
	ROPE_STRUCT* rope = &Ropes[NumRopes];
	
	PrepareRope(rope, (PHD_VECTOR*)&item->pos, &pos, 128, item);
	
	item->triggerFlags = NumRopes;
	NumRopes++;
}

void PrepareRope(ROPE_STRUCT* rope, PHD_VECTOR* pos1, PHD_VECTOR* pos2, int length, ITEM_INFO* item)
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

	int il = 0x300000;
	int l = 0;
	int sum = 0;
	PHD_VECTOR* pos = &rope->segment[0];
	PHD_VECTOR* velocity = &rope->velocity[0];
	do
	{
		pos->x = pos2->x * sum >> 16;
		pos->y = pos2->y * sum >> 16;
		pos->z = pos2->z * sum >> 16;

		velocity->x = 0;
		velocity->y = 0;
		velocity->z = 0;

		if (item->triggerFlags == -1)
		{
			pos->x = l;
			pos->y >>= 4;

			velocity->x = 16384;
			velocity->y = il;
			velocity->z = 16384;
		}

		pos++;
		velocity++;
		l += 1024;
		sum += rope->segmentLength;
		il -= 0x20000;
	} while (il > 0);

	rope->active = 0;
}

void NormaliseRopeVector(PHD_VECTOR* vec)
{
	int x = vec->x >> 16;
	int y = vec->y >> 16;
	int z = vec->z >> 16;

	if (!x && !y && !z)
		return;

	int length = SQUARE(x) + SQUARE(y) + SQUARE(z);
	if (length < 0)
		length = -length;

	length = 0x1000000 / (SQRT_ASM(length) << 16 >> 8) << 8 >> 8;
	
	vec->x = vec->x * length >> 16;
	vec->y = vec->y * length >> 16;
	vec->z = vec->z * length >> 16;
}