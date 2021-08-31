#include "framework.h"
#include "sphere.h"
#include "draw.h"
#include "lara.h"
#include "level.h"
#include "setup.h"
#include "Renderer11.h"
#include "trmath.h"

using namespace TEN::Renderer;

bool GotLaraSpheres;
SPHERE LaraSpheres[MAX_SPHERES];
SPHERE CreatureSpheres[MAX_SPHERES];

int GetSpheres(ITEM_INFO* item, SPHERE* ptr, int worldSpace, Matrix local)
{
	if (!item)
		return 0;

	BoundingSphere spheres[MAX_SPHERES];
	short itemNumber = (item - g_Level.Items.data());

	int num = g_Renderer.getSpheres(itemNumber, spheres, worldSpace, local);

	for (int i = 0; i < MAX_SPHERES; i++)
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

	int creatureSphereCount = GetSpheres(item, CreatureSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
	int laraSphereCount = 0;

	if (l == LaraItem)
	{
		if (!GotLaraSpheres)
		{
			laraSphereCount = GetSpheres(l, LaraSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
			if (l == LaraItem)
				GotLaraSpheres = true;
		}
	}
	else
	{
		GotLaraSpheres = false;

		laraSphereCount = GetSpheres(l, LaraSpheres, SPHERES_SPACE_WORLD, Matrix::Identity);
		if (l == LaraItem)
			GotLaraSpheres = true;
	}

	l->touchBits = 0;

	if (creatureSphereCount <= 0)
	{
		item->touchBits = 0;
		return 0;
	}
	else
	{
		for (int i = 0; i < creatureSphereCount; i++)
		{
			SPHERE* ptr1 = &CreatureSpheres[i];
			
			int x1 = item->pos.xPos + ptr1->x;
			int y1 = item->pos.yPos + ptr1->y;
			int z1 = item->pos.zPos + ptr1->z;
			int r1 = ptr1->r;

			if (r1 > 0)
			{
				for (int j = 0; j < laraSphereCount; j++)
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
		rotX = ((rot0 & 0x3ff0) / 16);
		rotY = (((rot1 & 0xfc00) / 1024) | ((rot0 & 0xf) * 64) & 0x3ff);
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
	short itemNumber = item - g_Level.Items.data();

	// Use matrices done in the renderer and transform the input vector
	Vector3 p = Vector3(vec->x, vec->y, vec->z);
	g_Renderer.getItemAbsBonePosition(itemNumber, &p, joint);

	// Store the result
	vec->x = p.x;
	vec->y = p.y;
	vec->z = p.z;
}

