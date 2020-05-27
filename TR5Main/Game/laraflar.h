#pragma once
#include "global.h"

void FlareControl(short item_number);
void ready_flare();
void undraw_flare_meshes();
void draw_flare_meshes();
void undraw_flare();
void draw_flare();
void set_flare_arm(int frame);
void CreateFlare(short object, int thrown);
void DrawFlareInAir(ITEM_INFO* item);
void DoFlareInHand(int flare_age);
int DoFlareLight(PHD_VECTOR* pos, int flare_age);
