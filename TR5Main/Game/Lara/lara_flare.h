#pragma once

struct ITEM_INFO;
struct COLL_INFO;
struct PHD_VECTOR;
enum GAME_OBJECT_ID : short;

constexpr auto FLARE_AGE = 120 * 30;		// 120 seconds * 30 frames

void FlareControl(short item_number);
void ready_flare(ITEM_INFO* lara);
void undraw_flare_meshes(ITEM_INFO* lara);
void draw_flare_meshes(ITEM_INFO* lara);
void undraw_flare(ITEM_INFO* lara);
void draw_flare(ITEM_INFO* lara);
void set_flare_arm(ITEM_INFO* lara, int frame);
void CreateFlare(ITEM_INFO* lara, GAME_OBJECT_ID object, int thrown);
void DrawFlareInAir(ITEM_INFO* item);
void DoFlareInHand(ITEM_INFO* lara, int flare_age);
int DoFlareLight(PHD_VECTOR* pos, int flare_age);
