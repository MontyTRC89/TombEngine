#pragma once

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