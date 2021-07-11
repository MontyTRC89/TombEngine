#pragma once
enum GAME_OBJECT_ID : short;
struct LaraInfo;

bool TryAddAmmo(LaraInfo &, GAME_OBJECT_ID id, int amt = 0);
bool TryRemoveAmmo(LaraInfo &, GAME_OBJECT_ID id, int amt = 0);
std::optional<int> GetAmmoCount(LaraInfo &, GAME_OBJECT_ID id);
