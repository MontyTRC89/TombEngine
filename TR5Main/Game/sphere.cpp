#include "sphere.h"
#include "draw.h"

int GetSpheres(ITEM_INFO* item, SPHERE* ptr, char worldSpace)
{
	int x, y, z;

	if (!item)
		return 0;

	if (worldSpace & 1)
	{
		x = item->pos.xPos;
		y = item->pos.yPos;
		z = item->pos.zPos;

		phd_PushUnitMatrix();

		MatrixPtr[M03] = 0;
		MatrixPtr[M13] = 0;
		MatrixPtr[M23] = 0;
	}
	else
	{
		z = 0;
		y = 0;
		x = 0;

		phd_PushMatrix();
		phd_TranslateAbs(item->pos.xPos, item->pos.yPos, item->pos.zPos);
	}

	phd_RotYXZ(item->pos.yRot, item->pos.xRot, item->pos.zRot);

	short* frame = GetBestFrame(item);
	phd_TranslateRel(frame[6], frame[7], frame[8]);
	short* rotation = &frame[9];
	gar_RotYXZsuperpack(&rotation, 0);

	OBJECT_INFO* obj = &Objects[item->objectNumber];
	short** meshPtr = &Meshes[obj->meshIndex];
	int* bone = &Bones[obj->boneIndex];

	phd_PushMatrix();

	short* objPtr = *(meshPtr++);							// Get Sphere stuff Now...

	if (!(worldSpace & 2))
		phd_TranslateRel(objPtr[0], objPtr[1], objPtr[2]);

	ptr->x = x + (MatrixPtr[M03] >> W2V_SHIFT);
	ptr->y = y + (MatrixPtr[M13] >> W2V_SHIFT);
	ptr->z = z + (MatrixPtr[M23] >> W2V_SHIFT);
	ptr->r = (int)objPtr[3];

	phd_PopMatrix();

	short* extraRotation = (short*)item->data;

	for (int i = obj->nmeshes - 1; i > 0; i--, bone += 3)
	{
		int poppush = *(bone++);
		if (poppush & 1)
			phd_PopMatrix();
		if (poppush & 2)
			phd_PushMatrix();

		phd_TranslateRel(*(bone), *(bone + 1), *(bone + 2));
		gar_RotYXZsuperpack(&rotation, 0);

		if ((poppush & (ROT_X | ROT_Y | ROT_Z)) && extraRotation)
		{
			if (poppush & ROT_Y)
				phd_RotY(*(extraRotation++));
			if (poppush & ROT_X)
				phd_RotX(*(extraRotation++));
			if (poppush & ROT_Z)
				phd_RotZ(*(extraRotation++));
		}

		objPtr = *(meshPtr++);

		phd_PushMatrix();
		if (!(worldSpace & 2))
			phd_TranslateRel(objPtr[0], objPtr[1], objPtr[2]);

		ptr->x = x + (MatrixPtr[M03] >> W2V_SHIFT);
		ptr->y = y + (MatrixPtr[M13] >> W2V_SHIFT);
		ptr->z = z + (MatrixPtr[M23] >> W2V_SHIFT);
		ptr->r = (int)objPtr[3];
		ptr++;
		phd_PopMatrix();
	}

	phd_PopMatrix();
	return obj->nmeshes;
}

void Inject_Sphere()
{
	INJECT(0x00479380, GetSpheres);
}