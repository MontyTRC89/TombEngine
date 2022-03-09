#pragma once

enum GAME_OBJECT_ID : short;
struct LaraInfo;

bool TryAddingWeapon(LaraInfo& lara, GAME_OBJECT_ID objectID, int amount = 0);
bool TryRemovingWeapon(LaraInfo& lara, GAME_OBJECT_ID objectID, int amount = 0);
std::optional<bool> HasWeapon(LaraInfo&, GAME_OBJECT_ID objectID);
