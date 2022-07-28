#pragma once
#include "Game/control/control.h"

struct CollisionInfo;
struct ItemInfo;
struct Vector3Int;
enum GAME_OBJECT_ID : short;

void FlareControl(short itemNumber);
void ReadyFlare(ItemInfo* laraItem);
void UndrawFlareMeshes(ItemInfo* laraItem);
void DrawFlareMeshes(ItemInfo* laraItem);
void UndrawFlare(ItemInfo* laraItem);
void DrawFlare(ItemInfo* laraItem);
void SetFlareArm(ItemInfo* laraItem, int armFrame);
void CreateFlare(ItemInfo* laraItem, GAME_OBJECT_ID objectNumber, bool thrown);
void DrawFlareInAir(ItemInfo* flareItem);
void DoFlareInHand(ItemInfo* laraItem, int flareLife);
int DoFlareLight(Vector3Int* pos, int flareLife);
