#pragma once
enum GAME_OBJECT_ID : short;
struct LaraInfo;

bool TryAddWeapon(LaraInfo& lara, GAME_OBJECT_ID obj, int amt = 0);
bool TryRemoveWeapon(LaraInfo& lara, GAME_OBJECT_ID obj, int amt = 0);
std::optional<bool> HasWeapon(LaraInfo &, GAME_OBJECT_ID id);
