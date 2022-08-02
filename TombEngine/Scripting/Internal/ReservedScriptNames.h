#pragma once

// Tables
static constexpr char ScriptReserved_TEN[]				= "TEN";
static constexpr char ScriptReserved_Flow[]				= "Flow";
static constexpr char ScriptReserved_Logic[]			= "Logic";
static constexpr char ScriptReserved_Objects[]			= "Objects";
static constexpr char ScriptReserved_Strings[]			= "Strings";
static constexpr char ScriptReserved_Inventory[]		= "Inventory";
static constexpr char ScriptReserved_Misc[]				= "Misc";

// Classes
static constexpr char ScriptReserved_Moveable[]			= "Moveable";
static constexpr char ScriptReserved_Static[]			= "Static";
static constexpr char ScriptReserved_Camera[]			= "Camera";
static constexpr char ScriptReserved_Sink[]				= "Sink";
static constexpr char ScriptReserved_SoundSource[]		= "SoundSource";
static constexpr char ScriptReserved_AIObject[]			= "AIObject";
static constexpr char ScriptReserved_DisplayString[]	= "DisplayString";
static constexpr char ScriptReserved_Vec3[]				= "Vec3";

// Member functions
static constexpr char ScriptReserved_new[]				= "new";
static constexpr char ScriptReserved_Init[]				= "Init";
static constexpr char ScriptReserved_Enable[]			= "Enable";
static constexpr char ScriptReserved_Disable[]			= "Disable";
static constexpr char ScriptReserved_MakeInvisible[]	= "MakeInvisible";
static constexpr char ScriptReserved_GetColor[]			= "GetColor";
static constexpr char ScriptReserved_SetColor[]			= "SetColor";
static constexpr char ScriptReserved_GetPosition[]		= "GetPosition";
static constexpr char ScriptReserved_SetPosition[]		= "SetPosition";
static constexpr char ScriptReserved_GetRotation[]		= "GetRotation";
static constexpr char ScriptReserved_SetRotation[]		= "SetRotation";
static constexpr char ScriptReserved_GetRotationY[]		= "GetRotationY";
static constexpr char ScriptReserved_SetRotationY[]		= "SetRotationY";
static constexpr char ScriptReserved_GetName[]			= "GetName";
static constexpr char ScriptReserved_SetName[]			= "SetName";
static constexpr char ScriptReserved_GetSlot[]			= "GetSlot";
static constexpr char ScriptReserved_SetSlot[]			= "SetSlot";
static constexpr char ScriptReserved_GetObjectID[]		= "GetObjectID";
static constexpr char ScriptReserved_SetObjectID[]		= "SetObjectID";
static constexpr char ScriptReserved_GetSoundID[]		= "GetSoundID";
static constexpr char ScriptReserved_SetSoundID[]		= "SetSoundID";
static constexpr char ScriptReserved_GetHP[]			= "GetHP";
static constexpr char ScriptReserved_SetHP[]			= "SetHP";
static constexpr char ScriptReserved_GetSlotHP[]		= "GetSlotHP";
static constexpr char ScriptReserved_GetFrameNumber[]	= "GetFrame";
static constexpr char ScriptReserved_SetFrameNumber[]	= "SetFrame";
static constexpr char ScriptReserved_GetAnimNumber[]	= "GetAnim";
static constexpr char ScriptReserved_SetAnimNumber[]	= "SetAnim";
static constexpr char ScriptReserved_GetOCB[]			= "GetOCB";
static constexpr char ScriptReserved_SetOCB[]			= "SetOCB";
static constexpr char ScriptReserved_GetStatus[]		= "GetStatus";
static constexpr char ScriptReserved_GetAIBits[]		= "GetAIBits";
static constexpr char ScriptReserved_SetAIBits[]		= "SetAIBits";
static constexpr char ScriptReserved_GetItemFlags[]		= "GetItemFlags";
static constexpr char ScriptReserved_SetItemFlags[]		= "SetItemFlags";
static constexpr char ScriptReserved_GetHitStatus[]		= "GetHitStatus";
static constexpr char ScriptReserved_GetActive[]		= "GetActive";
static constexpr char ScriptReserved_GetRoom[]			= "GetRoom";
static constexpr char ScriptReserved_SetRoom[]			= "SetRoom";
static constexpr char ScriptReserved_GetStrength[]		= "GetStrength";
static constexpr char ScriptReserved_SetStrength[]		= "SetStrength";
static constexpr char ScriptReserved_GetValid[]			= "GetValid";
static constexpr char ScriptReserved_Destroy[]			= "Destroy";
static constexpr char ScriptReserved_GetKey[]			= "GetKey";
static constexpr char ScriptReserved_SetKey[]			= "SetKey";
static constexpr char ScriptReserved_GetOnKilled[]		= "GetOnKilled";
static constexpr char ScriptReserved_SetOnKilled[]		= "SetOnKilled";
static constexpr char ScriptReserved_GetOnHit[]			= "GetOnHit";
static constexpr char ScriptReserved_SetOnHit[]			= "SetOnHit";
static constexpr char ScriptReserved_GetOnCollidedWithObject[]	= "GetOnCollidedWithObject";
static constexpr char ScriptReserved_SetOnCollidedWithObject[]	= "SetOnCollidedWithObject";
static constexpr char ScriptReserved_GetOnCollidedWithRoom[]	= "GetOnCollidedWithRoom";
static constexpr char ScriptReserved_SetOnCollidedWithRoom[]	= "SetOnCollidedWithRoom";
static constexpr char ScriptReserved_ToLength[]					= "ToLength";

// Flow Functions
static constexpr char ScriptReserved_AddLevel[]					= "AddLevel";
static constexpr char ScriptReserved_SetIntroImagePath[]		= "SetIntroImagePath";
static constexpr char ScriptReserved_SetTitleScreenImagePath[]	= "SetTitleScreenImagePath";
static constexpr char ScriptReserved_SetFarView[]				= "SetFarView";
static constexpr char ScriptReserved_SetSettings[]				= "SetSettings";
static constexpr char ScriptReserved_SetAnimations[]			= "SetAnimations";

// Flow Functions
static constexpr char ScriptReserved_SetStrings[]			= "SetStrings";
static constexpr char ScriptReserved_GetString[]			= "GetString";
static constexpr char ScriptReserved_SetLanguageNames[]		= "SetLanguageNames";

// Flow Tables
static constexpr char ScriptReserved_WeatherType[]		= "WeatherType";
static constexpr char ScriptReserved_LaraType[]			= "LaraType";
static constexpr char ScriptReserved_InvItem[]			= "InvID";
static constexpr char ScriptReserved_RotationAxis[]		= "RotationAxis";
static constexpr char ScriptReserved_ItemAction[]		= "ItemAction";
static constexpr char ScriptReserved_ErrorMode[]		= "ErrorMode";
static constexpr char ScriptReserved_InventoryItem[]	= "InventoryItem";

// Functions
static constexpr char ScriptReserved_ShowString[]					= "ShowString";
static constexpr char ScriptReserved_HideString[]					= "HideString";
static constexpr char ScriptReserved_SetAmbientTrack[]				= "SetAmbientTrack";
static constexpr char ScriptReserved_PlayAudioTrack[]				= "PlayAudioTrack";
static constexpr char ScriptReserved_GiveInvItem[]					= "GiveItem";
static constexpr char ScriptReserved_TakeInvItem[]					= "TakeItem";
static constexpr char ScriptReserved_GetInvItemCount[]				= "GetItemCount";
static constexpr char ScriptReserved_SetInvItemCount[]				= "SetItemCount";
static constexpr char ScriptReserved_GetMoveableByName[]			= "GetMoveableByName";
static constexpr char ScriptReserved_GetStaticByName[]				= "GetStaticByName";
static constexpr char ScriptReserved_GetCameraByName[]				= "GetCameraByName";
static constexpr char ScriptReserved_GetSinkByName[]				= "GetSinkByName";
static constexpr char ScriptReserved_GetAIObjectByName[]			= "GetAIObjectByName";
static constexpr char ScriptReserved_GetSoundSourceByName[]			= "GetSoundSourceByName";
static constexpr char ScriptReserved_CalculateDistance[]			= "CalculateDistance";
static constexpr char ScriptReserved_CalculateHorizontalDistance[]	= "CalculateHorizontalDistance";
static constexpr char ScriptReserved_ScreenToPercent[]				= "ScreenToPercent";
static constexpr char ScriptReserved_PercentToScreen[]				= "PercentToScreen";
static constexpr char ScriptReserved_HasLineOfSight[]				= "HasLineOfSight";

// Enums
static constexpr char ScriptReserved_ObjID[]					= "ObjID";
static constexpr char ScriptReserved_DisplayStringOption[]		= "DisplayStringOption";

static constexpr char ScriptReserved_LevelVars[]	= "LevelVars";
static constexpr char ScriptReserved_GameVars[]		= "GameVars";
static constexpr char ScriptReserved_LevelFuncs[]	= "LevelFuncs";

