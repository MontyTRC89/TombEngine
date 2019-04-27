#pragma once
#include "..\Global\global.h"

#define TR_KEY_ESCAPE          0x01
#define TR_KEY_1               0x02
#define TR_KEY_2               0x03
#define TR_KEY_3               0x04
#define TR_KEY_4               0x05
#define TR_KEY_5               0x06
#define TR_KEY_6               0x07
#define TR_KEY_7               0x08
#define TR_KEY_8               0x09
#define TR_KEY_9               0x0A
#define TR_KEY_0               0x0B
#define TR_KEY_MINUS           0x0C    /* - on main keyboard */
#define TR_KEY_EQUALS          0x0D
#define TR_KEY_BACK            0x0E    /* backspace */
#define TR_KEY_TAB             0x0F
#define TR_KEY_Q               0x10
#define TR_KEY_W               0x11
#define TR_KEY_E               0x12
#define TR_KEY_R               0x13
#define TR_KEY_T               0x14
#define TR_KEY_Y               0x15
#define TR_KEY_U               0x16
#define TR_KEY_I               0x17
#define TR_KEY_O               0x18
#define TR_KEY_P               0x19
#define TR_KEY_LBRACKET        0x1A
#define TR_KEY_RBRACKET        0x1B
#define TR_KEY_RETURN          0x1C    /* Enter on main keyboard */
#define TR_KEY_LCONTROL        0x1D
#define TR_KEY_A               0x1E
#define TR_KEY_S               0x1F
#define TR_KEY_D               0x20
#define TR_KEY_F               0x21
#define TR_KEY_G               0x22
#define TR_KEY_H               0x23
#define TR_KEY_J               0x24
#define TR_KEY_K               0x25
#define TR_KEY_L               0x26
#define TR_KEY_SEMICOLON       0x27
#define TR_KEY_APOSTROPHE      0x28
#define TR_KEY_GRAVE           0x29    /* accent grave */
#define TR_KEY_LSHIFT          0x2A
#define TR_KEY_BACKSLASH       0x2B
#define TR_KEY_Z               0x2C
#define TR_KEY_X               0x2D
#define TR_KEY_C               0x2E
#define TR_KEY_V               0x2F
#define TR_KEY_B               0x30
#define TR_KEY_N               0x31
#define TR_KEY_M               0x32
#define TR_KEY_COMMA           0x33
#define TR_KEY_PERIOD          0x34    /* . on main keyboard */
#define TR_KEY_SLASH           0x35    /* / on main keyboard */
#define TR_KEY_RSHIFT          0x36
#define TR_KEY_MULTIPLY        0x37    /* * on numeric keypad */
#define TR_KEY_LMENU           0x38    /* left Alt */
#define TR_KEY_SPACE           0x39
#define TR_KEY_CAPITAL         0x3A
#define TR_KEY_F1              0x3B
#define TR_KEY_F2              0x3C
#define TR_KEY_F3              0x3D
#define TR_KEY_F4              0x3E
#define TR_KEY_F5              0x3F
#define TR_KEY_F6              0x40
#define TR_KEY_F7              0x41
#define TR_KEY_F8              0x42
#define TR_KEY_F9              0x43
#define TR_KEY_F10             0x44
#define TR_KEY_NUMLOCK         0x45
#define TR_KEY_SCROLL          0x46    /* Scroll Lock */
#define TR_KEY_NUMPAD7         0x47
#define TR_KEY_NUMPAD8         0x48
#define TR_KEY_NUMPAD9         0x49
#define TR_KEY_SUBTRACT        0x4A    /* - on numeric keypad */
#define TR_KEY_NUMPAD4         0x4B
#define TR_KEY_NUMPAD5         0x4C
#define TR_KEY_NUMPAD6         0x4D
#define TR_KEY_ADD             0x4E    /* + on numeric keypad */
#define TR_KEY_NUMPAD1         0x4F
#define TR_KEY_NUMPAD2         0x50
#define TR_KEY_NUMPAD3         0x51
#define TR_KEY_NUMPAD0         0x52
#define TR_KEY_DECIMAL         0x53    /* . on numeric keypad */
#define TR_KEY_OEM_102         0x56    /* <> or \| on RT 102-key keyboard (Non-U.S.) */
#define TR_KEY_F11             0x57
#define TR_KEY_F12             0x58
#define TR_KEY_F13             0x64    /*                     (NEC PC98) */
#define TR_KEY_F14             0x65    /*                     (NEC PC98) */
#define TR_KEY_F15             0x66    /*                     (NEC PC98) */
#define TR_KEY_KANA            0x70    /* (Japanese keyboard)            */
#define TR_KEY_ABNT_C1         0x73    /* /? on Brazilian keyboard */
#define TR_KEY_CONVERT         0x79    /* (Japanese keyboard)            */
#define TR_KEY_NOCONVERT       0x7B    /* (Japanese keyboard)            */
#define TR_KEY_YEN             0x7D    /* (Japanese keyboard)            */
#define TR_KEY_ABNT_C2         0x7E    /* Numpad . on Brazilian keyboard */
#define TR_KEY_NUMPADEQUALS    0x8D    /* = on numeric keypad (NEC PC98) */
#define TR_KEY_PREVTRACK       0x90    /* Previous Track (TR_KEY_CIRCUMFLEX on Japanese keyboard) */
#define TR_KEY_AT              0x91    /*                     (NEC PC98) */
#define TR_KEY_COLON           0x92    /*                     (NEC PC98) */
#define TR_KEY_UNDERLINE       0x93    /*                     (NEC PC98) */
#define TR_KEY_KANJI           0x94    /* (Japanese keyboard)            */
#define TR_KEY_STOP            0x95    /*                     (NEC PC98) */
#define TR_KEY_AX              0x96    /*                     (Japan AX) */
#define TR_KEY_UNLABELED       0x97    /*                        (J3100) */
#define TR_KEY_NEXTTRACK       0x99    /* Next Track */
#define TR_KEY_NUMPADENTER     0x9C    /* Enter on numeric keypad */
#define TR_KEY_RCONTROL        0x9D
#define TR_KEY_MUTE            0xA0    /* Mute */
#define TR_KEY_CALCULATOR      0xA1    /* Calculator */
#define TR_KEY_PLAYPAUSE       0xA2    /* Play / Pause */
#define TR_KEY_MEDIASTOP       0xA4    /* Media Stop */
#define TR_KEY_VOLUMEDOWN      0xAE    /* Volume - */
#define TR_KEY_VOLUMEUP        0xB0    /* Volume + */
#define TR_KEY_WEBHOME         0xB2    /* Web home */
#define TR_KEY_NUMPADCOMMA     0xB3    /* , on numeric keypad (NEC PC98) */
#define TR_KEY_DIVIDE          0xB5    /* / on numeric keypad */
#define TR_KEY_SYSRQ           0xB7
#define TR_KEY_RMENU           0xB8    /* right Alt */
#define TR_KEY_PAUSE           0xC5    /* Pause */
#define TR_KEY_HOME            0xC7    /* Home on arrow keypad */
#define TR_KEY_UP              0xC8    /* UpArrow on arrow keypad */
#define TR_KEY_PRIOR           0xC9    /* PgUp on arrow keypad */
#define TR_KEY_LEFT            0xCB    /* LeftArrow on arrow keypad */
#define TR_KEY_RIGHT           0xCD    /* RightArrow on arrow keypad */
#define TR_KEY_END             0xCF    /* End on arrow keypad */
#define TR_KEY_DOWN            0xD0    /* DownArrow on arrow keypad */
#define TR_KEY_NEXT            0xD1    /* PgDn on arrow keypad */
#define TR_KEY_INSERT          0xD2    /* Insert on arrow keypad */
#define TR_KEY_DELETE          0xD3    /* Delete on arrow keypad */
#define TR_KEY_LWIN            0xDB    /* Left Windows key */
#define TR_KEY_RWIN            0xDC    /* Right Windows key */
#define TR_KEY_APPS            0xDD    /* AppMenu key */
#define TR_KEY_POWER           0xDE    /* System Power */
#define TR_KEY_SLEEP           0xDF    /* System Sleep */
#define TR_KEY_WAKE            0xE3    /* System Wake */
#define TR_KEY_WEBSEARCH       0xE5    /* Web Search */
#define TR_KEY_WEBFAVORITES    0xE6    /* Web Favorites */
#define TR_KEY_WEBREFRESH      0xE7    /* Web Refresh */
#define TR_KEY_WEBSTOP         0xE8    /* Web Stop */
#define TR_KEY_WEBFORWARD      0xE9    /* Web Forward */
#define TR_KEY_WEBBACK         0xEA    /* Web Back */
#define TR_KEY_MYCOMPUTER      0xEB    /* My Computer */
#define TR_KEY_MAIL            0xEC    /* Mail */
#define TR_KEY_MEDIASELECT     0xED    /* Media Select */

extern char* g_KeyNames[];

#define S_UpdateInput ((__int32 (__cdecl*)()) 0x004A92D0)
#define CheckKeyConflicts ((__int32 (__cdecl*)()) 0x004ADF40)

void __cdecl InitialiseDirectInput(HWND handle, HINSTANCE instance);
void __cdecl DI_ReadKeyboard(byte* keys);
__int32 __cdecl DD_SpinMessageLoopMaybe();

void Inject_Input();