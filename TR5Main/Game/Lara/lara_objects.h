#pragma once

struct ITEM_INFO;
struct COLL_INFO;

// -----------------------------------
// MISCELLANEOUS INTERACTABLE OBJECT
// State Control & Collision Functions
// -----------------------------------

// ------
// PICKUP
// ------

void lara_as_pickup(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_pickupflare(ITEM_INFO* item, COLL_INFO* coll);

// ------
// SWITCH
// ------

void lara_as_switchon(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_switchoff(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_turnswitch(ITEM_INFO* item, COLL_INFO* coll);

// ----------
// RECEPTACLE
// ----------

void lara_as_usekey(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_usepuzzle(ITEM_INFO* item, COLL_INFO* coll);

// --------
// PUSHABLE
// --------

void lara_as_pushblock(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_pullblock(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_ppready(ITEM_INFO* item, COLL_INFO* coll);

// ------
// PULLEY
// ------

void lara_as_pulley(ITEM_INFO* item, COLL_INFO* coll);

// --------------
// HORIZONTAL BAR
// --------------

void lara_as_parallelbars(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_pbleapoff(ITEM_INFO* item, COLL_INFO* coll);

// ---------
// TIGHTROPE
// ---------

#ifdef NEW_TIGHTROPE
void lara_as_trexit(ITEM_INFO* item, COLL_INFO* coll);
#endif
void lara_as_trpose(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_trwalk(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_trfall(ITEM_INFO* item, COLL_INFO* coll);

// ----
// ROPE
// ----

void lara_as_ropel(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_roper(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_rope(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_rope(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_ropefwd(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_climbrope(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_climbroped(ITEM_INFO* item, COLL_INFO* coll);

// -------------
// VERTICAL POLE
// -------------

void lara_as_pole_idle(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_pole_idle(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_pole_up(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_pole_up(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_pole_down(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_pole_down(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_pole_turn_clockwise(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_pole_turn_clockwise(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_pole_turn_counter_clockwise(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_pole_turn_counter_clockwise(ITEM_INFO* item, COLL_INFO* coll);

// --------
// ZIP-LINE
// --------

void lara_as_deathslide(ITEM_INFO* item, COLL_INFO* coll);
