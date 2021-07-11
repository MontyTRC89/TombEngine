#pragma once
enum GAME_OBJECT_ID : short;
struct LaraInfo;

bool TryAddMiscItem(LaraInfo& lara, GAME_OBJECT_ID obj);
bool TryRemoveMiscItem(LaraInfo& lara, GAME_OBJECT_ID obj);
std::optional<bool> HasMiscItem(LaraInfo& lara, GAME_OBJECT_ID obj);
