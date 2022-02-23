#pragma once

// Tables
static constexpr char const ScriptReserved_TEN[]				= "TEN";
static constexpr char const ScriptReserved_Flow[]			= "Flow";
static constexpr char const ScriptReserved_Logic[]			= "Logic";
static constexpr char const ScriptReserved_Objects[]			= "Objects";
static constexpr char const ScriptReserved_Strings[]			= "Strings";
static constexpr char const ScriptReserved_Inventory[]			= "Inventory";
static constexpr char const ScriptReserved_Misc[]			= "Misc";

// Classes
static constexpr char const ScriptReserved_Moveable[]			= "Moveable";
static constexpr char const ScriptReserved_Static[]			= "Static";
static constexpr char const ScriptReserved_Camera[]			= "Camera";
static constexpr char const ScriptReserved_Sink[]			= "Sink";
static constexpr char const ScriptReserved_SoundSource[]	= "SoundSource";
static constexpr char const ScriptReserved_AIObject[]			= "AIObject";
static constexpr char const ScriptReserved_DisplayString[]		= "DisplayString";

// Member functions
static constexpr char const ScriptReserved_new[]				= "New";
static constexpr char const ScriptReserved_newTemporary[]		= "NewTemporary";
static constexpr char const ScriptReserved_Init[]				= "Init";
static constexpr char const ScriptReserved_Enable[]				= "Enable";
static constexpr char const ScriptReserved_Disable[]			= "Disable";
static constexpr char const ScriptReserved_MakeInvisible[]		= "MakeInvisible";
static constexpr char const ScriptReserved_GetColor[]			= "GetColor";
static constexpr char const ScriptReserved_SetColor[]			= "SetColor";
static constexpr char const ScriptReserved_GetPosition[]		= "GetPosition";
static constexpr char const ScriptReserved_SetPosition[]		= "SetPosition";
static constexpr char const ScriptReserved_GetRotation[]		= "GetRotation";
static constexpr char const ScriptReserved_SetRotation[]		= "SetRotation";
static constexpr char const ScriptReserved_GetName[]			= "GetName";
static constexpr char const ScriptReserved_SetName[]			= "SetName";
static constexpr char const ScriptReserved_GetSlot[]			= "GetSlot";
static constexpr char const ScriptReserved_SetSlot[]			= "SetSlot";
static constexpr char const ScriptReserved_GetObjectID[]		= "GetObjectID";
static constexpr char const ScriptReserved_SetObjectID[]		= "SetObjectID";
static constexpr char const ScriptReserved_GetHP[]				= "GetHP";
static constexpr char const ScriptReserved_SetHP[]				= "SetHP";
static constexpr char const ScriptReserved_GetFrameNumber[]		= "GetFrame";
static constexpr char const ScriptReserved_SetFrameNumber[]		= "SetFrame";
static constexpr char const ScriptReserved_GetAnimNumber[]		= "GetAnim";
static constexpr char const ScriptReserved_SetAnimNumber[]		= "SetAnim";
static constexpr char const ScriptReserved_GetOCB[]				= "GetOCB";
static constexpr char const ScriptReserved_SetOCB[]				= "SetOCB";
static constexpr char const ScriptReserved_GetStatus[]			= "GetStatus";
static constexpr char const ScriptReserved_GetAIBits[]			= "GetAIBits";
static constexpr char const ScriptReserved_SetAIBits[]			= "SetAIBits";
static constexpr char const ScriptReserved_GetHitStatus[]		= "GetHitStatus";
static constexpr char const ScriptReserved_GetActive[]			= "GetActive";
static constexpr char const ScriptReserved_GetRoom[]			= "GetRoom";
static constexpr char const ScriptReserved_SetRoom[]			= "SetRoom";

// Flow Functions
static constexpr char const ScriptReserved_AddLevel[]			= "AddLevel";
static constexpr char const ScriptReserved_SetIntroImagePath[]	= "SetIntroImagePath";
static constexpr char const ScriptReserved_SetTitleScreenImagePath[]	= "SetTitleScreenImagePath";
static constexpr char const ScriptReserved_SetFarView[]	= "SetFarView";
static constexpr char const ScriptReserved_SetSettings[]	= "SetSettings";
static constexpr char const ScriptReserved_SetAnimations[]	= "SetAnimations";

// Flow Functions
static constexpr char const ScriptReserved_SetStrings[]	= "SetStrings";
static constexpr char const ScriptReserved_SetLanguageNames[]	= "SetLanguageNames";

// Flow Tables
static constexpr char const ScriptReserved_WeatherType[]	= "WeatherType";
static constexpr char const ScriptReserved_LaraType[]	= "LaraType";
static constexpr char const ScriptReserved_InvItem[]	= "InvID";
static constexpr char const ScriptReserved_RotationAxis[]	= "RotationAxis";
static constexpr char const ScriptReserved_ItemAction[]	= "ItemAction";
static constexpr char const ScriptReserved_ErrorMode[]	= "ErrorMode";
static constexpr char const ScriptReserved_InventoryItem[]	= "InventoryItem";

// Functions
static constexpr char const ScriptReserved_ShowString[]			= "ShowString";
static constexpr char const ScriptReserved_HideString[]			= "HideString";
static constexpr char const ScriptReserved_SetAmbientTrack[] = "SetAmbientTrack";
static constexpr char const ScriptReserved_PlayAudioTrack[] = "PlayAudioTrack";
static constexpr char const ScriptReserved_GiveInvItem[] = "GiveItem";
static constexpr char const ScriptReserved_TakeInvItem[] = "TakeItem";
static constexpr char const ScriptReserved_GetInvItemCount[] = "GetItemCount";
static constexpr char const ScriptReserved_SetInvItemCount[] = "SetItemCount";
static constexpr char const ScriptReserved_GetMoveableByName[] = "GetMoveableByName";
static constexpr char const ScriptReserved_GetStaticByName[] = "GetStaticByName";
static constexpr char const ScriptReserved_GetCameraByName[] = "GetCameraByName";
static constexpr char const ScriptReserved_GetSinkByName[] = "GetSinkByName";
static constexpr char const ScriptReserved_GetSoundSourceByName[] = "GetSoundSourceByName";
static constexpr char const ScriptReserved_CalculateDistance[] = "CalculateDistance";
static constexpr char const ScriptReserved_CalculateHorizontalDistance[] = "CalculateHorizontalDistance";
static constexpr char const ScriptReserved_ScreenToPercent[] = "ScreenToPercent";
static constexpr char const ScriptReserved_PercentToScreen[] = "PercentToScreen";

// Enums
static constexpr char const ScriptReserved_ObjID[]					= "ObjID";
static constexpr char const ScriptReserved_DisplayStringOption[]	= "DisplayStringOption";

static constexpr char const ScriptReserved_LevelVars[]= "LevelVars";
static constexpr char const ScriptReserved_GameVars[] = "GameVars";
static constexpr char const ScriptReserved_LevelFuncs[] = "LevelFuncs";

