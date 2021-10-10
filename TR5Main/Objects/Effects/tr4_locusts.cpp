#include "framework.h"
#include "tr4_locusts.h"
#include "Sound/sound.h"
#include "Specific/trmath.h"
#include "sphere.h"
#include "level.h"
#include "lara.h"
#include "effects/tomb4fx.h"
#include "collide.h"
#include "animation.h"
#include "items.h"

namespace TEN::Entities::TR4 
{
    LOCUST_INFO Locusts[MAX_LOCUSTS];

    int CreateLocust(void)
    {
        LOCUST_INFO* locust;

        for (int i = 0; i < MAX_LOCUSTS; i++)
        {
            locust = &Locusts[i];
            if (!locust->on)
                return i;
        }

        return NO_ITEM;
    }

    void SpawnLocust(ITEM_INFO* item)
    {
        LOCUST_INFO* locust;
        ITEM_INFO* target;
        PHD_VECTOR start, end;
        short angles[2];
        short locustNumber = NO_ITEM;

        locustNumber = CreateLocust();
        if (locustNumber != NO_ITEM)
        {
            locust = &Locusts[locustNumber];
            // emitter
            if (item->objectNumber == ID_LOCUSTS_EMITTER)
            {
                end.x = item->pos.xPos;
                end.y = item->pos.yPos;
                end.z = item->pos.zPos;
                angles[0] = item->pos.yRot - ANGLE(180.0f);
                angles[1] = 0;
            }
            // mutant
            else
            {
                start.x = 0;
                start.y = -96;
                start.z = 144;
                GetJointAbsPosition(item, &start, 9);
                end.x = 0;
                end.y = -128;
                end.z = 288;
                GetJointAbsPosition(item, &end, 9);
                phd_GetVectorAngles(end.x - start.x, end.y - start.y, end.z - start.z, angles);
            }

            // NOTE: this is not present in original TR4 code
            //target = GetCreatureInfo(item)->enemy;

            locust->on = true;
            //locust->target = target != nullptr ? target : nullptr;
            locust->pos.xPos = end.x;
            locust->pos.yPos = end.y;
            locust->pos.zPos = end.z;
            locust->pos.yRot = (GetRandomControl() & 0x7FF) + angles[0] - 0x400;
            locust->pos.xRot = (GetRandomControl() & 0x3FF) + angles[1] - 0x200;
            locust->roomNumber = item->roomNumber;
            locust->randomRotation = (GetRandomControl() & 0x1F) + 0x10;
            locust->escapeYrot = (GetRandomControl() & 0x1FF);
            locust->escapeXrot = ((GetRandomControl() & 0x7) + 0xF) * 20;
            locust->counter = 0;
        }
    }

    void InitialiseLocust(short itemNumber)
    {
        ITEM_INFO* item = &g_Level.Items[itemNumber];

        if (item->pos.yRot > 0)
        {
            if (item->pos.yRot == ANGLE(90))
                item->pos.xPos += CLICK(2);
        }
        else if (item->pos.yRot < 0)
        {
            if (item->pos.yRot == -ANGLE(180))
                item->pos.zPos -= CLICK(2);
            else if (item->pos.yRot == -ANGLE(90))
                item->pos.xPos -= CLICK(2);
        }
        else
        {
            item->pos.zPos += CLICK(2);
        }
    }

    void LocustControl(short itemNumber)
    {
        ITEM_INFO* item = &g_Level.Items[itemNumber];

        if (TriggerActive(item))
        {
            if (item->triggerFlags)
            {
                SpawnLocust(item);
                item->triggerFlags--;
            }
            else
            {
                KillItem(itemNumber);
            }
        }
    }

    void UpdateLocusts(void)
    {
        LOCUST_INFO* locust;
        ITEM_INFO* target;
        PHD_VECTOR posAngle;
        int distance;
        int square;
        short angles[2];

        for (int i = 0; i < MAX_LOCUSTS; i++)
        {
            locust = &Locusts[i];
            if (locust->on)
            {
                // NOTE: not present in original TR4 code
                //if (LaraItem == nullptr)
                //    LaraItem = LaraItem;

                if ((Lara.keepDucked || LaraItem->hitPoints <= 0)
                    && locust->counter >= 90
                    && !(GetRandomControl() & 7))
                    locust->counter = 90;

                locust->counter--;
                if (locust->counter == 0)
                {
                    locust->on = false;
                    break;
                }

                if (!(GetRandomControl() & 7))
                {
                    locust->escapeYrot = GetRandomControl() % 640 + 128;
                    locust->escapeXrot = (GetRandomControl() & 0x7F) - 64;
                    locust->escapeZrot = (GetRandomControl() & 0x7F) - 64;
                }

                phd_GetVectorAngles(
                    LaraItem->pos.xPos + 8 * locust->escapeXrot - locust->pos.xPos,
                    LaraItem->pos.yPos - locust->escapeYrot - locust->pos.yPos,
                    LaraItem->pos.zPos + 8 * locust->escapeZrot - locust->pos.zPos,
                    angles);

                distance = SQUARE(LaraItem->pos.zPos - locust->pos.zPos) + SQUARE(LaraItem->pos.xPos - locust->pos.xPos);
                square = int(sqrt(distance)) / 8;
                if (square <= 128)
                {
                    if (square < 48)
                        square = 48;
                }
                else
                {
                    square = 128;
                }

                if (locust->randomRotation < square)
                    locust->randomRotation += 1;
                else if (locust->randomRotation > square)
                    locust->randomRotation -= 1;

                if (locust->counter > 90)
                {
                    short resultYrot, resultXrot;
                    int shiftYrot, shiftXrot;
                    int random = locust->randomRotation * 128;
                    resultYrot = angles[0] - locust->pos.yRot;
                    if (abs(resultYrot) > 0x8000)
                        resultYrot = locust->pos.yRot - angles[0];
                    resultXrot = angles[1] - locust->pos.xRot;
                    if (abs(resultXrot) > 0x8000)
                        resultXrot = locust->pos.xRot - angles[0];
                    shiftYrot = resultYrot / 8;
                    shiftXrot = resultXrot / 8;
                    if (shiftYrot > random || shiftYrot < -random)
                        shiftYrot = -random;
                    if (shiftXrot > random || shiftXrot < -random)
                        shiftXrot = -random;
                    locust->pos.yRot += shiftYrot;
                    locust->pos.xRot += shiftXrot;
                }

                locust->pos.xPos += locust->randomRotation * phd_cos(locust->pos.xRot) * phd_sin(locust->pos.yRot);
                locust->pos.yPos += locust->randomRotation * phd_sin(-locust->pos.xRot);
                locust->pos.zPos += locust->randomRotation * phd_cos(locust->pos.xRot) * phd_cos(locust->pos.yRot);
                if (ItemNearTarget(&locust->pos, LaraItem, CLICK(1) / 2))
                {
                    TriggerBlood(locust->pos.xPos, locust->pos.yPos, locust->pos.zPos, 2 * GetRandomControl(), 2);
                    if (LaraItem->hitPoints > 0)
                        LaraItem->hitPoints -= LOCUST_LARA_DAMAGE;
                    // NOTE: not present in original TR4 code
                    //else
                    //    LaraItem->hitPoints -= LOCUST_ENTITY_DAMAGE;
                }

                if (locust->counter > 0)
                    SoundEffect(SFX_TR4_LOCUSTS_LOOP, &locust->pos, NULL);
            }
        }
    }

    void DrawLocust(void)
    {
        // TODO: no render for the locusts !
    }
}