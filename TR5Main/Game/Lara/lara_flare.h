#pragma once

struct ITEM_INFO;
struct COLL_INFO;
struct PHD_VECTOR;
enum GAME_OBJECT_ID : short;

constexpr auto FLARE_AGE = 30 * 30;	//30 seconds * 30 frames

void FlareControl(short item_number);
void ready_flare();
void undraw_flare_meshes();
void draw_flare_meshes();
void undraw_flare(ITEM_INFO* item);
void draw_flare(ITEM_INFO* item);
void set_flare_arm(int frame);
void CreateFlare(ITEM_INFO* l, GAME_OBJECT_ID object, int thrown);
void DrawFlareInAir(ITEM_INFO* item);
void DoFlareInHand(int flare_age);
int DoFlareLight(PHD_VECTOR* pos, int flare_age);
