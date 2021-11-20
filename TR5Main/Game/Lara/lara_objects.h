#pragma once
#include "lara_struct.h"

/*pickups*/
void lara_as_pickup(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_pickupflare(ITEM_INFO* item, COLL_INFO* coll);
/*end pickups*/
/*-*/
/*switches*/
void lara_as_switchon(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_switchoff(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_turnswitch(ITEM_INFO* item, COLL_INFO* coll);
/*end switches*/
/*-*/
/*puzzles and keys*/
void lara_as_usekey(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_usepuzzle(ITEM_INFO* item, COLL_INFO* coll);
/*end puzzles and keys*/
/*-*/
/*pushables*/
void lara_as_pushblock(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_pullblock(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_ppready(ITEM_INFO* item, COLL_INFO* coll);
/*end pushables*/
/*-*/
/*pulley*/
void lara_as_pulley(ITEM_INFO* item, COLL_INFO* coll);
/*end pulley*/
/*-*/
/*parallel bars*/
void lara_as_parallelbars(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_pbleapoff(ITEM_INFO* item, COLL_INFO* coll);
/*end parallel bars*/
/*-*/
/*tightropes*/
#ifdef NEW_TIGHTROPE
void lara_as_trexit(ITEM_INFO* item, COLL_INFO* coll);
#endif // NEW_TIGHTROPE
void lara_as_trpose(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_trwalk(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_trfall(ITEM_INFO* item, COLL_INFO* coll);
/*end tightropes*/
/*-*/
/*ropes*/
void lara_as_ropel(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_roper(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_rope(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_rope(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_ropefwd(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_climbrope(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_climbroped(ITEM_INFO* item, COLL_INFO* coll);
/*end ropes*/
/*-*/
/*poles*/
void lara_col_polestat(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_poleup(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_poledown(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_poleleft(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_poleright(ITEM_INFO* item, COLL_INFO* coll);
/*end poles*/
/*-*/
/*deathslide*/
void lara_as_deathslide(ITEM_INFO* item, COLL_INFO* coll);
/*end deathslide*/
