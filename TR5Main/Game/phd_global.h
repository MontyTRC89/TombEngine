#pragma once

typedef struct PHD_VECTOR
{
	int x;
	int y;
	int z;

	PHD_VECTOR()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
	}

	PHD_VECTOR(int xpos, int ypos, int zpos)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
	}
};

typedef struct PHD_3DPOS
{
	int xPos;
	int yPos;
	int zPos;
	short xRot;
	short yRot;
	short zRot;

	PHD_3DPOS()
	{
		this->xPos = 0;
		this->yPos = 0;
		this->zPos = 0;
		this->xRot = 0;
		this->yRot = 0;
		this->zRot = 0;
	}

	PHD_3DPOS(int x, int y, int z)
	{
		this->xPos = x;
		this->yPos = y;
		this->zPos = z;
		this->xRot = 0;
		this->yRot = 0;
		this->zRot = 0;
	}

	PHD_3DPOS(short xrot, short yrot, short zrot)
	{
		this->xPos = 0;
		this->yPos = 0;
		this->zPos = 0;
		this->xRot = xrot;
		this->yRot = yrot;
		this->zRot = zrot;
	}

	PHD_3DPOS(int x, int y, int z, short xrot, short yrot, short zrot)
	{
		this->xPos = x;
		this->yPos = y;
		this->zPos = z;
		this->xRot = xrot;
		this->yRot = yrot;
		this->zRot = zrot;
	}
};

typedef struct GAME_VECTOR
{
	int x;
	int y;
	int z;
	short roomNumber;
	short boxNumber;

	GAME_VECTOR()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->roomNumber = 0;
		this->boxNumber = 0;
	}

	GAME_VECTOR(int xpos, int ypos, int zpos)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->roomNumber = 0;
		this->boxNumber = 0;
	}

	GAME_VECTOR(int xpos, int ypos, int zpos, short roomNumber)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->roomNumber = roomNumber;
		this->boxNumber = 0;
	}

	GAME_VECTOR(int xpos, int ypos, int zpos, short roomNumber, short boxNumber)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->roomNumber = roomNumber;
		this->boxNumber = boxNumber;
	}
};

typedef struct OBJECT_VECTOR
{
	int x;
	int y;
	int z;
	short data;
	short flags;

	OBJECT_VECTOR()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->data = NULL;
		this->flags = NULL;
	}

	OBJECT_VECTOR(int xpos, int ypos, int zpos)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->data = NULL;
		this->flags = NULL;
	}

	OBJECT_VECTOR(int xpos, int ypos, int zpos, short newdata)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->data = newdata;
		this->flags = NULL;
	}

	OBJECT_VECTOR(int xpos, int ypos, int zpos, short flags, bool isFlags) // use isFlags to use flag instead of newdata !
	{
		UNREFERENCED_PARAMETER(isFlags);
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->data = NULL;
		this->flags = flags;
	}

	OBJECT_VECTOR(int xpos, int ypos, int zpos, short newdata, short newflags)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->data = newdata;
		this->flags = newflags;
	}
};

typedef struct VECTOR
{
	int vx;
	int vy;
	int vz;
	int pad;
};

typedef struct SVECTOR
{
	short vx;
	short vy;
	short vz;
	short pad;
};

typedef struct CVECTOR
{
	byte r;
	byte g;
	byte b;
	byte cd;
};

typedef struct TR_VERTEX
{
	int x;
	int y;
	int z;
};

typedef enum MATRIX_ARRAY_VALUE
{
	M00, M01, M02, M03,
	M10, M11, M12, M13,
	M20, M21, M22, M23
};

typedef struct MATRIX3D
{
	short m00;
	short m01;
	short m02;
	short m10;
	short m11;
	short m12;
	short m20;
	short m21;
	short m22;
	short pad;
	int tx;
	int ty;
	int tz;
};