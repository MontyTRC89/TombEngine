#include "../newobjects.h"

// TODO: check dword_51CF98 name
void InitialiseRomanStatue(short itemNum)
{
    /*
    ITEM_INFO* v1; // esi
    __int16 v2; // ax
    unsigned int v3; // edi
    __int16 v4; // cx
    int v5; // eax
    int v6; // eax

    v1 = &Items[itemNum];
    j_ClearItem(itemNum);
    v2 = objects[ID_ROMAN_GOD].anim_index + 16;
    v3 = v1->flags2 & 0xFFFFFFF9;
    v1->anim_number = objects[ID_ROMAN_GOD].anim_index + 16;
    v4 = anims[v2].frame_base;
    v1->goal_anim_state = 13;
    v1->current_anim_state = 13;
    v5 = v1->pos.y_rot;
    v1->frame_number = v4;
    v1->flags2 = v3;
    v6 = ((v5 + 0x4000) >> 3) & 0x1FFE;
    v1->pos.x_pos += 1944 * rcossin_tbl[v6] >> 14;
    v1->pos.z_pos += 1944 * rcossin_tbl[v6 + 1] >> 14;
    memset(&dword_51CF98, 0, 0x2Cu);
    *(&dword_51CF98 + 44) = 0;
    */
}