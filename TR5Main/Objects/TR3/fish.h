#pragma once

struct FISH_INFO
{
	short x;
	short y;
	short z;
	int	angle;
	short destY;
	short angAdd;
	unsigned char speed;
	unsigned char acc;
	unsigned char swim;
};

struct FISH_LEADER_INFO
{
	short angle;
	unsigned char speed;
	unsigned char on;
	short angleTime;
	short speedTime;
	short xRange, yRange, zRange;
};
