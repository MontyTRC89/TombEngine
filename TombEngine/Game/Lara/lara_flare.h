#pragma once

enum GAME_OBJECT_ID : short;
class Vector3i;
struct CollisionInfo;
struct ItemInfo;

void FlareControl(short itemNumber);
void ReadyFlare(ItemInfo& laraItem);
void UndrawFlareMeshes(ItemInfo& laraItem);
void DrawFlareMeshes(ItemInfo& laraItem);
void UndrawFlare(ItemInfo& laraItem);
void DrawFlare(ItemInfo& laraItem);
void SetFlareArm(ItemInfo& laraItem, int armFrame);
void CreateFlare(ItemInfo& laraItem, GAME_OBJECT_ID objectNumber, bool isThrown);
void DrawFlareInAir(ItemInfo& flareItem);
void DoFlareInHand(ItemInfo& laraItem, int flareLife);
bool DoFlareLight(const Vector3i& pos, int flareLife);
