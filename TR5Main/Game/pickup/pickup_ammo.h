#pragma once

enum GAME_OBJECT_ID : short;
struct LaraInfo;

bool TryAddingAmmo(LaraInfo&, GAME_OBJECT_ID objectID, int amount = 0);
bool TryRemovingAmmo(LaraInfo&, GAME_OBJECT_ID objectID, int amount = 0);
std::optional<int> GetAmmoCount(LaraInfo&, GAME_OBJECT_ID objectID);
