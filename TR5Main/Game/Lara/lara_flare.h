#pragma once

struct ITEM_INFO;
struct COLL_INFO;
struct PHD_VECTOR;
enum GAME_OBJECT_ID : short;

constexpr auto FLARE_AGE = 60 * 30;		// 60 * 30 frames = 60 seconds.

void FlareControl(short itemNum);
void ReadyFlare(ITEM_INFO* laraItem);
void UndrawFlareMeshes(ITEM_INFO* laraItem);
void DrawFlareMeshes(ITEM_INFO* laraItem);
void UndrawFlare(ITEM_INFO* laraItem);
void DrawFlare(ITEM_INFO* laraItem);
void SetFlareArm(ITEM_INFO* laraItem, int armFrame);
void CreateFlare(ITEM_INFO* laraItem, GAME_OBJECT_ID object, bool thrown);
void DrawFlareInAir(ITEM_INFO* flareItem);
void DoFlareInHand(ITEM_INFO* laraItem, int flareAge);
int DoFlareLight(PHD_VECTOR* pos, int flareAge);
