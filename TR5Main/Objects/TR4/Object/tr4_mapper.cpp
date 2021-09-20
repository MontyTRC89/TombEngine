#include "framework.h"
#include "tr4_mapper.h"
#include "level.h"
#include "control/control.h"
#include "Sound\sound.h"
#include "animation.h"
#include <lara.h>
#include <Game\sphere.h>
#include <effects\effects.h>

namespace TEN::Entities::TR4
{
    void InitialiseMapper(short itemNumber)
    {
        g_Level.Items[itemNumber].meshBits = -3;
    }

    void MapperControl(short itemNumber)
    {
        ITEM_INFO* item = &g_Level.Items[itemNumber];

        if (!TriggerActive(item))
            return;

        if (item->frameNumber - g_Level.Anims[item->animNumber].frameBase >= 200)
        {
            SoundEffect(SFX_TR4_MAPPER_LAZER, &item->pos, 0);

            item->meshBits |= 2;

            PHD_VECTOR pos;
            pos.x = 0;
            pos.y = 0;
            pos.z = 0;
            GetJointAbsPosition(item, &pos, SPHERES_SPACE_WORLD);

            byte color = (GetRandomControl() & 0x1F) + 192;
            TriggerDynamicLight(pos.x, pos.y, pos.z, (GetRandomControl() & 3) + 16, color, color, 0);

            short roomNumber = item->roomNumber;
            FLOOR_INFO* floor = GetFloor(item->pos.xPos, item->pos.yPos, item->pos.zPos, &roomNumber);
            int height = GetFloorHeight(floor, item->pos.xPos, item->pos.yPos, item->pos.zPos);

            for (int i = 0; i < 2; i++)
            {
                SPARKS* spark = &Sparks[GetFreeSpark()];

                spark->on = 1;
                spark->sR = (GetRandomControl() & 0x7F) + 64;
                spark->sG = spark->sR - (GetRandomControl() & 0x1F);
                spark->sB = 0;
                spark->dR = (GetRandomControl() & 0x7F) + 64;
                spark->dG = spark->dR - (GetRandomControl() & 0x1F);
                spark->dB = 0;
                spark->colFadeSpeed = 4;
                spark->fadeToBlack = 4;
                spark->transType = TransTypeEnum::COLADD;
                spark->life = 12;
                spark->sLife = 12;
                spark->x = pos.x;
                spark->y = height;
                spark->z = pos.z;
                spark->xVel = (GetRandomControl() & 0x3FF) - 512;
                spark->yVel = -256 - (GetRandomControl() & 0x7F);
                spark->zVel = (GetRandomControl() & 0x3FF) - 512;
                spark->friction = 4;
                spark->scalar = 2;
                spark->sSize = spark->size = (GetRandomControl() & 0xF) + 16;
                spark->dSize = (GetRandomControl() & 1) + 3;
                spark->maxYvel = 0;
                spark->gravity = (GetRandomControl() & 0x1F) + 32;
                spark->flags = SP_SCALE | SP_DEF;
            }
        }

        AnimateItem(item);
    }
}