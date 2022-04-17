#pragma once

// TODO: Move to EulerAngle.h and EulerAngle.cpp.
class EulerAngle
{
public:
	float x;
	float y;
	float z;

	EulerAngle();
	EulerAngle(float xRadians, float yRadians, float zRadians);

	float GetX();
	float GetY();
	float GetZ();
	Vector3 ToVector3();

	void Set(EulerAngle orient);
	void Set(float xRadians, float yRadians, float zRadians);
	void SetX(float radians);
	void SetY(float radians);
	void SetZ(float radians);

	void Clamp();
	static EulerAngle Clamp(EulerAngle orient);
	static float Clamp(float radians);

	void Interpolate(EulerAngle orientTo, float rate, float radianThreshold);
	static EulerAngle Interpolate(EulerAngle orientFrom, EulerAngle orientTo, float rate, float radianThreshold);
	static float Interpolate(float radiansFrom, float radiansTo, float rate, float radianThreshold);

	static EulerAngle ShortestAngle(EulerAngle orientFrom, EulerAngle orientTo);
	static float ShortestAngle(float radiansFrom, float radiansTo);

	static float AngleBetweenTwoPoints(Vector3 point0, Vector3 point1);

	static float DeltaHeading(Vector3 origin, Vector3 target, float heading);

	static float DegToRad(float degrees);
	static float RadToDeg(float radians);
	static float ShrtToRad(short shortForm); // Temporary legacy short form support for particularly cryptic code.
	static short DegToShrt(float degrees);
	static short RadToShrt(float radians);

	bool operator ==(EulerAngle orient);
	bool operator !=(EulerAngle orient);
	EulerAngle operator +(EulerAngle orient);
	EulerAngle operator -(EulerAngle orient);
	EulerAngle operator *(EulerAngle orient);
	EulerAngle operator *(float value);
	EulerAngle operator /(float value);
	EulerAngle& operator +=(EulerAngle orient);
	EulerAngle& operator -=(EulerAngle orient);
	EulerAngle& operator *=(EulerAngle orient);
	EulerAngle& operator *=(float value);
	EulerAngle& operator /=(float value);

private:
	// TODO: Due to clamping requirements, the components should be private?
	/*float x;
	float y;
	float z;*/
};

inline EulerAngle::EulerAngle()
{
	this->Set(0, 0, 0);
}

inline EulerAngle::EulerAngle(float xRadians, float yRadians, float zRadians)
{
	this->Set(xRadians, yRadians, zRadians);
}

inline float EulerAngle::GetX()
{
	return x;
}

inline float EulerAngle::GetY()
{
	return y;
}

inline float EulerAngle::GetZ()
{
	return z;
}

inline Vector3 EulerAngle::ToVector3()
{
	return Vector3(this->GetX(), this->GetY(), this->GetZ());
}

inline void EulerAngle::Set(EulerAngle orient)
{
	*this = Clamp(orient);
}

inline void EulerAngle::Set(float xRadians, float yRadians, float zRadians)
{
	this->SetX(xRadians);
	this->SetY(yRadians);
	this->SetZ(zRadians);
}

inline void EulerAngle::SetX(float radians = 0.0f)
{
	this->x = Clamp(radians);
}

inline void EulerAngle::SetY(float radians = 0.0f)
{
	this->y = Clamp(radians);
}

inline void EulerAngle::SetZ(float radians = 0.0f)
{
	this->z = Clamp(radians);
}

inline void EulerAngle::Clamp()
{
	this->SetX(Clamp(this->GetX()));
	this->SetY(Clamp(this->GetY()));
	this->SetZ(Clamp(this->GetZ()));
}

inline EulerAngle EulerAngle::Clamp(EulerAngle orient)
{
	orient.Clamp();
	return orient;
}

inline float EulerAngle::Clamp(float radians)
{
	//if (radians < -M_PI || radians > M_PI)
	return atan2(sin(radians), cos(radians));
	//else
	//	return radians;

	// Alternative method:
	/*return (radians > 0) ?
		fmod(radians + M_PI, M_PI * 2) - M_PI :
		fmod(radians - M_PI, M_PI * 2) + M_PI;*/
}

inline void EulerAngle::Interpolate(EulerAngle orientTo, float rate = 1.0f, float radianThreshold = 0.0f)
{
	*this = Interpolate(*this, orientTo, rate, radianThreshold);
}

inline EulerAngle EulerAngle::Interpolate(EulerAngle orientFrom, EulerAngle orientTo, float rate = 1.0f, float radianThreshold = 0.0f)
{
	return EulerAngle(
		Interpolate(orientFrom.GetX(), orientTo.GetX(), rate, radianThreshold),
		Interpolate(orientFrom.GetY(), orientTo.GetY(), rate, radianThreshold),
		Interpolate(orientFrom.GetZ(), orientTo.GetZ(), rate, radianThreshold)
	);
}

inline float EulerAngle::Interpolate(float radiansFrom, float radiansTo, float rate = 1.0f, float radianThreshold = 0.0f)
{
	rate = (abs(rate) > 1.0f) ? 1.0f : abs(rate);

	float difference = ShortestAngle(radiansFrom, radiansTo);
	if (abs(difference) > radianThreshold)
		return Clamp(radiansFrom + (difference * rate));
	else
		return Clamp(radiansTo);
}

inline EulerAngle EulerAngle::ShortestAngle(EulerAngle orientFrom, EulerAngle orientTo)
{
	return (orientTo - orientFrom);
}

inline float EulerAngle::ShortestAngle(float radiansFrom, float radiansTo)
{
	return Clamp(radiansTo - radiansFrom);
}

inline float EulerAngle::AngleBetweenTwoPoints(Vector3 point0, Vector3 point1)
{
	auto difference = point0 - point1;
	return atan2(difference.x, difference.z);
}

inline float EulerAngle::DeltaHeading(Vector3 origin, Vector3 target, float heading)
{
	auto difference = AngleBetweenTwoPoints(origin, target);
	return ShortestAngle(heading, difference + DegToRad(90.0f));
}

inline float EulerAngle::DegToRad(float degrees)
{
	return Clamp(degrees * (M_PI / 180.0f));
}

inline float EulerAngle::RadToDeg(float radians)
{
	float degrees = Clamp(radians) * (180.0f / M_PI);
	return fmod(degrees + 360.0f, 360.0f);
}

inline float EulerAngle::ShrtToRad(short shortForm)
{
	return Clamp(shortForm * (360.0f / (USHRT_MAX + 1)) * (180.0f / M_PI));
}

inline short EulerAngle::DegToShrt(float degrees)
{
	return (degrees * ((USHRT_MAX + 1) / 360.0f));
}

inline short EulerAngle::RadToShrt(float radians)
{
	return ((radians / (180.0f / M_PI)) * ((USHRT_MAX + 1) / 360.0f));
}

inline bool EulerAngle::operator ==(EulerAngle orient)
{
	auto orient1 = Clamp(*this);
	auto orient2 = Clamp(orient);
	return (orient1.GetX() == orient2.GetX() && orient1.GetY() == orient2.GetY() && orient1.GetZ() == orient2.GetZ());
}

inline bool EulerAngle::operator !=(EulerAngle orient)
{
	auto orient1 = Clamp(*this);
	auto orient2 = Clamp(orient);
	return (orient1.GetX() != orient2.GetX() || orient1.GetY() != orient2.GetY() || orient1.GetZ() != orient2.GetZ());
}

inline EulerAngle EulerAngle::operator +(EulerAngle orient)
{
	return Clamp(EulerAngle(this->GetX() + orient.GetX(), this->GetY() + orient.GetY(), this->GetZ() + orient.GetZ()));
}

inline EulerAngle EulerAngle::operator -(EulerAngle orient)
{
	return Clamp(EulerAngle(this->GetX() - orient.GetX(), this->GetY() - orient.GetY(), this->GetZ() - orient.GetZ()));
}

inline EulerAngle EulerAngle::operator *(EulerAngle orient)
{
	return Clamp(EulerAngle(this->GetX() * orient.GetX(), this->GetY() * orient.GetY(), this->GetZ() * orient.GetZ()));
}

inline EulerAngle EulerAngle::operator *(float value)
{
	return Clamp(EulerAngle(this->GetX() * value, this->GetY() * value, this->GetZ() * value));
}

inline EulerAngle EulerAngle::operator /(float value)
{
	return Clamp(EulerAngle(this->GetX() / value, this->GetY() / value, this->GetZ() / value));
}

inline EulerAngle& EulerAngle::operator +=(EulerAngle orient)
{
	this->Set(*this + orient);
	return *this;
}

inline EulerAngle& EulerAngle::operator -=(EulerAngle orient)
{
	this->Set(*this + orient);
	return *this;
}

inline EulerAngle& EulerAngle::operator *=(EulerAngle orient)
{
	this->Set(*this * orient);
	return *this;
}

inline EulerAngle& EulerAngle::operator *=(float value)
{
	this->Set(*this * value);
	return *this;
}

inline EulerAngle& EulerAngle::operator /=(float value)
{
	this->Set(*this / value);
	return *this;
}

// ---------

struct Vector2Int
{
	int x;
	int y;

	Vector2Int()
	{
		this->x = 0;
		this->y = 0;
	}

	Vector2Int(int x, int y)
	{
		this->x = x;
		this->y = y;
	}
};

struct Vector3Int
{
	int x;
	int y;
	int z;
	
	bool operator ==(Vector3Int pos)
	{
		return (x == pos.x && y == pos.y && z == pos.z);
	}
	
	bool operator !=(Vector3Int pos)
	{
		return (x != pos.x || y != pos.y || z != pos.z);
	}

	Vector3Int operator =(Vector3Int pos)
	{
		this->x = pos.x;
		this->y = pos.y;
		this->z = pos.z;
		return *this;
	}

	Vector3Int operator +(Vector3Int pos)
	{
		return Vector3Int(x + pos.x, y + pos.y, z + pos.z);
	}

	Vector3Int operator -(Vector3Int pos)
	{
		return Vector3Int(x - pos.x, y - pos.y, z - pos.z);
	}

	Vector3Int operator *(Vector3Int pos)
	{
		return Vector3Int(x * pos.x, y * pos.y, z * pos.z);
	}
	
	Vector3Int operator *(float value)
	{
		return Vector3Int((int)round(x * value), (int)round(y * value), (int)round(z * value));
	}

	Vector3Int operator /(float value)
	{
		return Vector3Int((int)round(x / value), (int)round(y / value), (int)round(z / value));
	}

	Vector3Int& operator +=(const Vector3Int pos)
	{
		*this = *this + pos;
		return *this;
	}

	Vector3Int& operator -=(Vector3Int pos)
	{
		*this = *this - pos;
		return *this;
	}

	Vector3Int& operator *=(Vector3Int pos)
	{
		*this = *this * pos;
		return *this;
	}
	
	Vector3Int& operator *=(float value)
	{
		*this = *this * value;
		return *this;
	}

	Vector3Int& operator /=(float value)
	{
		*this = *this / value;
		return *this;
	}
	
	Vector3Int()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
	}

	Vector3Int(int x, int y, int z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	Vector3 ToVector3()
	{
		return Vector3(x, y, z);
	}
};

class PoseData
{
public:
	Vector3Int Position;
	EulerAngle Orientation;

	PoseData()
	{
		this->Position = Vector3Int();
		this->Orientation.Set(EulerAngle());
	}

	PoseData(Vector3Int pos)
	{
		this->Position = pos;
		this->Orientation.Set(EulerAngle());
	}

	PoseData(int xPos, int yPos, int zPos)
	{
		this->Position = Vector3Int(xPos, yPos, zPos);
		this->Orientation.Set(EulerAngle());
	}

	PoseData(EulerAngle orient)
	{
		this->Position = Vector3Int();
		this->Orientation = orient;
	}

	PoseData(float xOrient, float yOrient, float zOrient)
	{
		this->Position = Vector3Int();
		this->Orientation.Set(xOrient, yOrient, zOrient);
	}

	PoseData(Vector3Int pos, EulerAngle orient)
	{
		this->Position = pos;
		this->Orientation = orient;
	}

	PoseData(int xPos, int yPos, int zPos, float xOrient, float yOrient, float zOrient)
	{
		this->Position = Vector3Int(xPos, yPos, zPos);
		this->Orientation.Set(xOrient, yOrient, zOrient);
	}
};

struct GameVector
{
	int x;
	int y;
	int z;
	int boxNumber;
	short roomNumber;

	GameVector()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->roomNumber = 0;
		this->boxNumber = 0;
	}

	GameVector(int xpos, int ypos, int zpos)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->roomNumber = 0;
		this->boxNumber = 0;
	}

	GameVector(int xpos, int ypos, int zpos, short roomNumber)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->roomNumber = roomNumber;
		this->boxNumber = 0;
	}

	GameVector(int xpos, int ypos, int zpos, short roomNumber, short boxNumber)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->roomNumber = roomNumber;
		this->boxNumber = boxNumber;
	}
};

struct LEVEL_CAMERA_INFO
{
	int x;
	int y;
	int z;
	int roomNumber;
	int flags;
	int speed;
	std::string luaName;

	LEVEL_CAMERA_INFO()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->roomNumber = 0;
		this->flags = 0x0;
		this->speed = 1;
	}

	LEVEL_CAMERA_INFO(int xpos, int ypos, int zpos)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->roomNumber = 0;
		this->flags = 0x0;
		this->speed = 1;
	}

	LEVEL_CAMERA_INFO(int xpos, int ypos, int zpos, short room)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->roomNumber = room;
		this->flags = 0x0;
		this->speed = 1;
	}

	LEVEL_CAMERA_INFO(int xpos, int ypos, int zpos, short flags, bool isFlags) // use isFlags to use flag instead of newdata !
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->roomNumber = 0;
		this->flags = flags;
		this->speed = 1;
	}

	LEVEL_CAMERA_INFO(int xpos, int ypos, int zpos, short room, short newflags)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->roomNumber = room;
		this->flags = newflags;
		this->speed = 1;
	}
};

struct SINK_INFO
{
	int x;
	int y;
	int z;
	int strength;
	int boxIndex;
	std::string luaName;

	SINK_INFO()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->strength = 0;
		this->boxIndex = 0;
	}

	SINK_INFO(int xpos, int ypos, int zpos)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->strength = 0;
		this->boxIndex = 0;
	}

	SINK_INFO(int xpos, int ypos, int zpos, short strength)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->strength = strength;
		this->boxIndex = 0;
	}

	SINK_INFO(int xpos, int ypos, int zpos, short strength, short boxIndex)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->strength = strength;
		this->boxIndex = boxIndex;
	}
};

struct SOUND_SOURCE_INFO
{
	int x;
	int y;
	int z;
	int soundId;
	int flags;
	std::string luaName;

	SOUND_SOURCE_INFO()
	{
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->soundId = 0;
		this->flags = 0x0;
	}

	SOUND_SOURCE_INFO(int xpos, int ypos, int zpos)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->soundId = 0;
		this->flags = 0x0;
	}

	SOUND_SOURCE_INFO(int xpos, int ypos, int zpos, short soundId)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->soundId = soundId;
		this->flags = 0x0;
	}

	SOUND_SOURCE_INFO(int xpos, int ypos, int zpos, short soundId, short newflags)
	{
		this->x = xpos;
		this->y = ypos;
		this->z = zpos;
		this->soundId = soundId;
		this->flags = newflags;
	}
};

struct VECTOR
{
	int vx;
	int vy;
	int vz;
	int pad;
};

struct SVECTOR
{
	short vx;
	short vy;
	short vz;
	short pad;
};

struct CVECTOR
{
	byte r;
	byte g;
	byte b;
	byte cd;
};

struct TR_VERTEX
{
	int x;
	int y;
	int z;
};

enum MATRIX_ARRAY_VALUE
{
	M00, M01, M02, M03,
	M10, M11, M12, M13,
	M20, M21, M22, M23
};

struct MATRIX3D
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

struct BOUNDING_BOX
{
	short X1;
	short X2;
	short Y1;
	short Y2;
	short Z1;
	short Z2;
};
