#pragma once
#include "savegame.h"

struct SavegameInfo
{
	bool present;
	char levelName[75];
	int saveNumber;
	short days;
	short hours;
	short minutes;
	short seconds;
	char fileName[255];
};

#define MAX_SAVEGAMES 16

struct GAMEFLOW
{
	unsigned int CheatEnabled : 1;
	unsigned int LoadSaveEnabled : 1;
	unsigned int TitleEnabled : 1;
	unsigned int PlayAnyLevel : 1;
	unsigned int Language : 3;
	unsigned int DemoDisc : 1;
	unsigned int Unused : 24;
	unsigned int InputTimeout;
	unsigned char SecurityTag;
	unsigned char nLevels;
	unsigned char nFileNames;
	unsigned char Pad;
	unsigned short FileNameLen;
	unsigned short ScriptLen;
};

extern SavegameInfo g_SavegameInfos[MAX_SAVEGAMES];
extern std::vector<std::string> g_NewStrings;
extern SaveGameHeader g_NewSavegameInfos[MAX_SAVEGAMES];

int LoadSavegameInfos();