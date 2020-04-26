#include "sphere.h"
#include "draw.h"
#include "lara.h"
#include "../Specific/level.h"
#include "../Specific/setup.h"

int NumLaraSpheres;
bool GotLaraSpheres;
SPHERE LaraSpheres[34];
SPHERE SpheresList[34];

int GetSpheres(ITEM_INFO* item, SPHERE* ptr, char worldSpace, Matrix local)
{
	if (!item)
		return 0;

	BoundingSphere spheres[34];
	short itemNumber = (item - Items); // / sizeof(ITEM_INFO);

	int num = g_Renderer->GetSpheres(itemNumber, spheres, worldSpace, local);

	for (int i = 0; i < 34; i++)
	{
		ptr[i].x = spheres[i].Center.x;
		ptr[i].y = spheres[i].Center.y;
		ptr[i].z = spheres[i].Center.z;
		ptr[i].r = spheres[i].Radius;
	}

	return num;
}

int TestCollision(ITEM_INFO* item, ITEM_INFO* l)
{
	int flags = 0;

	int num1 = GetSpheres(item, SpheresList, 1, Matrix::Identity);
	int num2 = 0;

	if (l == LaraItem)
	{
		if (GotLaraSpheres)
		{
			num2 = NumLaraSpheres;
		}
		else
		{
			num2 = GetSpheres(l, LaraSpheres, 1, Matrix::Identity);
			NumLaraSpheres = num2;
			if (l == LaraItem)
				GotLaraSpheres = true;
		}
	}
	else
	{
		GotLaraSpheres = false;

		num2 = GetSpheres(l, LaraSpheres, 1, Matrix::Identity);
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
			SPHERE* ptr1 = &SpheresList[i];
			
			int x1 = item->pos.xPos + ptr1->x;
			int y1 = item->pos.yPos + ptr1->y;
			int z1 = item->pos.zPos + ptr1->z;
			int r1 = ptr1->r;

			if (r1 > 0)
			{
				for (int j = 0; j < num2; j++)
				{
					SPHERE* ptr2 = &LaraSpheres[j];

					int x2 = item->pos.xPos + ptr2->x;
					int y2 = item->pos.yPos + ptr2->y;
					int z2 = item->pos.zPos + ptr2->z;
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

void GetMatrixFromTrAngle(Matrix* matrix, short* frameptr, int index)
{
	short* ptr = &frameptr[0];

	ptr += 9;
	for (int i = 0; i < index; i++)
	{
		ptr += ((*ptr & 0xc000) == 0 ? 2 : 1);
	}

	int rot0 = *ptr++;
	int frameMode = (rot0 & 0xc000);

	int rot1;
	int rotX;
	int rotY;
	int rotZ;

	switch (frameMode)
	{
	case 0:
		rot1 = *ptr++;
		rotX = ((rot0 & 0x3ff0) >> 4);
		rotY = (((rot1 & 0xfc00) >> 10) | ((rot0 & 0xf) << 6) & 0x3ff);
		rotZ = ((rot1) & 0x3ff);

		*matrix = Matrix::CreateFromYawPitchRoll(rotY * (360.0f / 1024.0f) * RADIAN,
			rotX * (360.0f / 1024.0f) * RADIAN,
			rotZ * (360.0f / 1024.0f) * RADIAN);
		break;

	case 0x4000:
		*matrix = Matrix::CreateRotationX((rot0 & 0xfff) * (360.0f / 4096.0f) * RADIAN);
		break;

	case 0x8000:
		*matrix = Matrix::CreateRotationY((rot0 & 0xfff) * (360.0f / 4096.0f) * RADIAN);
		break;

	case 0xc000:
		*matrix = Matrix::CreateRotationZ((rot0 & 0xfff) * (360.0f / 4096.0f) * RADIAN);
		break;
	}
}

void GetJointAbsPosition(ITEM_INFO* item, PHD_VECTOR* vec, int joint)
{
	// Get the real item number
	short itemNumber = ((item - Items) / sizeof(ITEM_INFO));

	// Use matrices done in the renderer and transform the input vector
	Vector3 p = Vector3(vec->x, vec->y, vec->z);
	g_Renderer->GetItemAbsBonePosition(itemNumber, &p, joint);

	// Store the result
	vec->x = p.x;
	vec->y = p.y;
	vec->z = p.z;
}

