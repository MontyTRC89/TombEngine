#pragma once

extern short SubsuitAir;
extern struct COLL_INFO mycoll;
extern short cheat_hitPoints;

extern void GetLaraDeadlyBounds();
extern void DelAlignLaraToRope(struct ITEM_INFO* item);
extern void InitialiseLaraAnims(struct ITEM_INFO* item);
extern void InitialiseLaraLoad(short item_num);
extern void InitialiseLara(int restore);
extern void LaraControl(short item_number);
extern void LaraCheat(struct ITEM_INFO* item, struct COLL_INFO* coll);
extern void LaraInitialiseMeshes();

//#define LaraBurn ((void (__cdecl*)()) 0x0048AD60)
//#define InitialiseLaraLoad ((void (__cdecl*)(__int16)) 0x004568C0)
