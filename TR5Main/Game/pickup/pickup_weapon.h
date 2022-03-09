#pragma once

enum GAME_OBJECT_ID : short;
struct LaraInfo;

bool TryAddWeapon(LaraInfo& lara, GAME_OBJECT_ID object, int amount = 0);
bool TryRemoveWeapon(LaraInfo& lara, GAME_OBJECT_ID object, int amount = 0);
std::optional<bool> HasWeapon(LaraInfo&, GAME_OBJECT_ID object);
