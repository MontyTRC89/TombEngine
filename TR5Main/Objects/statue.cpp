#include "framework.h"
#include "newobjects.h"

// TODO: finish the statue render (drawanimatingitem not work anymore with the new render)
// TODO: crash with the explosion death !

// for TR2 and TR3 statue (compatible by any statue entity)
// the statue object need to be after the normal one:
// ex: ID_SWORD_GUARDIAN: 256, ID_SWORD_GUARDIAN_STATUE: 257
void DrawStatue(ITEM_INFO* item)
{
	/*
	ObjectInfo* obj;
	CREATURE_INFO* creature;
	int* bones;
	int clip, i, poppush, frac, rate, bit;
	short* frames[2];
	short* extra_rotation;
	short* rotation1, *rotation2;
	short** normal, **statue;

	creature = (CREATURE_INFO*)item->data;
	frac = GetFrame_D2(item, frames, &rate);

	if (item->hitPoints <= 0 && item->status != ITEM_ACTIVE && item->meshBits != 0)
		item->meshBits = item->meshBits >> 1;

	obj = &Objects[item->objectNumber];
	//if (obj->shadowSize)
	//	S_PrintShadow(obj->shadowSize, frames[0], item, NULL);

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.xPos, item->pos.yPos, item->pos.zPos);
	phd_RotYXZ(item->pos.yRot, item->pos.xRot, item->pos.zRot);

	if (phd_ClipBoundingBox(frames[0]))
	{
		CalculateObjectLighting(item, frames[0]);

		if (item->data == NULL)
			extra_rotation = NullRotations;
		else
			extra_rotation = (short*)item->data;

		// all entity have the statue slot after it, (ex: ID_SWORD_GUARDIAN: 256, ID_SWORD_GUARDIAN_STATUE: 257)
		normal = &Meshes[Objects[obj->objectNumber].meshIndex];
		statue = &Meshes[Objects[obj->objectNumber + 1].meshIndex];
		bones = &Bones[obj->boneIndex];
		bit = 1;

		if (!frac)
		{
			phd_TranslateRel((int)*(frames[0] + 6), (int)*(frames[0] + 7), (int)*(frames[0] + 8)); // can be [0][6] etc.. ?
			rotation1 = (short*)(frames[0] + 9);
			gar_RotYXZsuperpack(&rotation1, 0);
			
			if (item->meshBits & bit)
				phd_PutPolygons(*normal);
			else
				phd_PutPolygons(*statue);
			normal++;
			statue++;

			for (i = (obj->nmeshes - 1); i > 0; i--, bones += 4, normal++, statue++)
			{
				poppush = *bones;

				if (poppush & 1)
				{
					phd_PopMatrix();
					phd_PopDxMatrix();
				}

				if (poppush & 2)
					phd_PushMatrix();

				phd_TranslateRel(*(bones + 1), *(bones + 2), *(bones + 3));
				gar_RotYXZsuperpack(&rotation1, 0);

				if (extra_rotation && (poppush & (ROT_X|ROT_Y|ROT_Z)))
				{
					if (poppush & ROT_Y)
						phd_RotY(*(extra_rotation++));
					if (poppush & ROT_X)
						phd_RotX(*(extra_rotation++));
					if (poppush & ROT_Z)
						phd_RotZ(*(extra_rotation++));
				}

				bit <<= 1;
				if (item->meshBits & bit)
					phd_PutPolygons(*normal);
				else
					phd_PutPolygons(*statue);
			}
		}
		else
		{
			InitInterpolate(frac, rate);
			phd_TranslateRel_ID((int)*(frames[0] + 6), (int)*(frames[0] + 7), (int)*(frames[0] + 8),
				                (int)*(frames[1] + 6), (int)*(frames[1] + 7), (int)*(frames[1] + 8));
			rotation1 = (short*)(frames[0] + 9);
			rotation2 = (short*)(frames[1] + 9);
			gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

			if (item->meshBits & bit)
				phd_PutPolygons_I(*normal);
			else
				phd_PutPolygons_I(*statue);
			normal++;
			statue++;

			for (i = (obj->nmeshes - 1); i > 0; i--, bones += 4, normal++, statue++)
			{
				poppush = *bones;
				if (poppush & 1)
					phd_PopMatrix_I();

				if (poppush & 2)
					phd_PushMatrix_I();

				phd_TranslateRel_I(*(bones + 1), *(bones + 2), *(bones + 3));
				gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

				if (extra_rotation && (poppush & (ROT_X|ROT_Y|ROT_Z)))
				{
					if (poppush & ROT_Y)
						phd_RotY_I(*(extra_rotation++));
					if (poppush & ROT_X)
						phd_RotX_I(*(extra_rotation++));
					if (poppush & ROT_Z)
						phd_RotZ_I(*(extra_rotation++));
				}

				bit <<= 1;
				if (item->meshBits & bit)
					phd_PutPolygons_I(*normal);
				else
					phd_PutPolygons_I(*statue);
			}
		}
	}

	//PhdRight = PhdWidth;
	//PhdLeft = 0;
	//PhdTop = 0;
	//PhdBottom = PhdHeight;
	phd_PopMatrix();
	phd_PopDxMatrix();
	*/
}