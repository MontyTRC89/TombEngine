#pragma once

enum GAME_OBJECT_ID : short;
struct LaraInfo;

bool TryAddMiscItem(LaraInfo& lara, GAME_OBJECT_ID objectID);
bool TryRemoveMiscItem(LaraInfo& lara, GAME_OBJECT_ID objectID);
std::optional<bool> HasMiscItem(LaraInfo& lara, GAME_OBJECT_ID objectID);
