#pragma once
#include "lara_struct.h"

#define SLOPE_ANGLE_NORMAL	ANGLE(30.0f)
#define SLOPE_ANGLE_STEEP	ANGLE(60.0f)

// Auxiliary functions.
bool TestLaraSlide(COLL_INFO* coll);
float GetLaraSlideDirection(COLL_INFO* coll);
void SetLaraSlide(ITEM_INFO* item, COLL_INFO* coll);
void PerformLaraSlide(ITEM_INFO* item, COLL_INFO* coll);
void PerformLaraSlideEdgeJump(ITEM_INFO* item, COLL_INFO* coll);

int Old_TestLaraSlide(ITEM_INFO* item, COLL_INFO* coll);

// Sliding control & collision functions.
void lara_as_slide(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_slide(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_slide_back(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_slide_back(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_steep_slide(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_steep_slide(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_steep_slide_back(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_steep_slide_back(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_slide_turn_180(ITEM_INFO* item, COLL_INFO* coll);
