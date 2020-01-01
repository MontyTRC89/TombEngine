#include "sphere.h"
#include "draw.h"

int NumLaraSpheres;
bool GotLaraSpheres;
SPHERE LaraSpheres[34];
SPHERE BaddieSpheres[34];

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

	short* objPtr = *(meshPtr++);

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

int TestCollision(ITEM_INFO* item, ITEM_INFO* l)
{
	int flags = 0;

	int num1 = GetSpheres(item, SphereList, 1);
	int num2 = 0;

	if (l == LaraItem)
	{
		if (GotLaraSpheres)
		{
			num2 = NumLaraSpheres;
		}
		else
		{
			num2 = GetSpheres(l, LaraSpheres, 1);
			NumLaraSpheres = num2;
			if (l == LaraItem)
				GotLaraSpheres = true;
		}
	}
	else
	{
		GotLaraSpheres = false;

		num2 = GetSpheres(l, LaraSpheres, 1);
		NumLaraSpheres = num2;
		if (l == LaraItem)
			GotLaraSpheres = true;
	}

	l->touchBits = 0;

	if (num1 <= 0)
	{
		item->touchBits = 0;
		return 0;
	}
	else
	{
		for (int i = 0; i < num1; i++)
		{
			SPHERE* ptr1 = &SphereList[i];
			
			int x1 = ptr1->x;
			int y1 = ptr1->y;
			int z1 = ptr1->z;
			int r1 = ptr1->r;

			if (r1 > 0)
			{
				for (int j = 0; j < num2; j++)
				{
					SPHERE* ptr2 = &LaraSpheres[j];

					int x2 = ptr2->x;
					int y2 = ptr2->y;
					int z2 = ptr2->z;
					int r2 = ptr2->r;

					if (r2 > 0)
					{
						int dx = x1 - x2;
						int dy = y1 - y2;
						int dz = z1 - z2;
						int r = r1 + r2;


						if (SQUARE(dx) + SQUARE(dy) + SQUARE(dz) < SQUARE(r))
						{
							l->touchBits |= (1 << j);
							flags |= (1 << i);
							break;
						}
					}
				}
			}
		}

		item->touchBits = flags;
		return flags;
	}
}

void Inject_Sphere()
{
	INJECT(0x00479380, GetSpheres);
	INJECT(0x00479170, TestCollision);
}