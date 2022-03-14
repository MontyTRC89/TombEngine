#pragma once

struct ITEM_INFO;
struct CollisionInfo;
struct PHD_VECTOR;
enum GAME_OBJECT_ID : short;

constexpr auto FLARE_LIFE_MAX = 60 * 30;	// 60 * 30 frames = 60 seconds.

void FlareControl(short itemNumber);
void ReadyFlare(ITEM_INFO* laraItem);
void UndrawFlareMeshes(ITEM_INFO* laraItem);
void DrawFlareMeshes(ITEM_INFO* laraItem);
void UndrawFlare(ITEM_INFO* laraItem);
void DrawFlare(ITEM_INFO* laraItem);
void SetFlareArm(ITEM_INFO* laraItem, int armFrame);
void CreateFlare(ITEM_INFO* laraItem, GAME_OBJECT_ID objectNumber, bool thrown);
void DrawFlareInAir(ITEM_INFO* flareItem);
void DoFlareInHand(ITEM_INFO* laraItem, int flareLife);
int DoFlareLight(PHD_VECTOR* pos, int flareLife);
