#pragma once

// TODO: Move to EulerAngle.h and EulerAngle.cpp.
class EulerAngle
{
public:
	// TODO: Due to clamping requirements and assumed [-M_PI, M_PI] range invariant, make components private?
	// Euler components in radians
	float x;
	float y;
	float z;

	// Constructors
	EulerAngle();
	EulerAngle(float xAngle, float yAngle, float zAngle);

	// Getters
	float   GetX();
	float   GetY();
	float   GetZ();

	// Setters
	void Set(EulerAngle orient);
	void Set(float xAngle, float yAngle, float zAngle);
	void SetX(float angle);
	void SetY(float angle);
	void SetZ(float angle);

	// Utilities
	bool Compare(EulerAngle orient, float epsilon);
	static bool Compare(EulerAngle orient0, EulerAngle orient1, float epsilon);
	static bool Compare(float angle0, float angle1, float epsilon);

	void Clamp();
	static EulerAngle Clamp(EulerAngle orient);
	static float Clamp(float angle);

	void Interpolate(EulerAngle orientTo, float rate, float epsilon);
	static EulerAngle Interpolate(EulerAngle orientFrom, EulerAngle orientTo, float rate, float epsilon);
	static float Interpolate(float angleFrom, float angleTo, float rate, float epsilon);

	EulerAngle ShortestAngle(EulerAngle orientTo);
	static EulerAngle ShortestAngle(EulerAngle orientFrom, EulerAngle orientTo);
	static float ShortestAngle(float angleFrom, float angleTo);

	static float AngleBetweenTwoPoints(Vector3 point0, Vector3 point1);
	static float DeltaHeading(Vector3 origin, Vector3 target, float heading);

	Vector3 ToVector3();

	// Converters
	static float DegToRad(float degrees);
	static float RadToDeg(float radians);
	static float ShrtToRad(short shortForm); // Temporary legacy short form support for particularly cryptic code.
	static short DegToShrt(float degrees);
	static short RadToShrt(float radians);

	// Operators
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
};

inline EulerAngle::EulerAngle()
{
	this->Set(0, 0, 0);
}

inline EulerAngle::EulerAngle(float xAngle, float yAngle, float zAngle)
{
	this->Set(xAngle, yAngle, zAngle);
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

inline void EulerAngle::Set(EulerAngle orient)
{
	*this = Clamp(orient);
}

inline void EulerAngle::Set(float xAngle, float yAngle, float zAngle)
{
	this->SetX(xAngle);
	this->SetY(yAngle);
	this->SetZ(zAngle);
}

inline void EulerAngle::SetX(float angle = 0.0f)
{
	this->x = Clamp(angle);
}

inline void EulerAngle::SetY(float angle = 0.0f)
{
	this->y = Clamp(angle);
}

inline void EulerAngle::SetZ(float angle = 0.0f)
{
	this->z = Clamp(angle);
}

inline bool EulerAngle::Compare(EulerAngle orient, float epsilon = 0.0f)
{
	return Compare(*this, orient, epsilon);
}

inline bool EulerAngle::Compare(EulerAngle orient0, EulerAngle orient1, float epsilon = 0.0f)
{
	return (Compare(orient0.x, orient1.x, epsilon) && Compare(orient0.y, orient1.y, epsilon) && Compare(orient0.z, orient1.z, epsilon));
}

inline bool EulerAngle::Compare(float angle0, float angle1, float epsilon = 0.0f)
{
	angle0 = Clamp(angle0);
	angle1 = Clamp(angle1);

	float difference = ShortestAngle(angle0, angle1);
	if (abs(difference) > epsilon)
		return false;
	else
		return true;
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

inline float EulerAngle::Clamp(float angle)
{
	//if (angle < -M_PI || angle > M_PI)
	return atan2(sin(angle), cos(angle));
	//else
	//	return angle;

	// Alternative method:
	/*return (angle > 0) ?
		fmod(angle + M_PI, M_PI * 2) - M_PI :
		fmod(angle - M_PI, M_PI * 2) + M_PI;*/
}

inline void EulerAngle::Interpolate(EulerAngle orientTo, float rate = 1.0f, float epsilon = 0.0f)
{
	*this = Interpolate(*this, orientTo, rate, epsilon);
}

inline EulerAngle EulerAngle::Interpolate(EulerAngle orientFrom, EulerAngle orientTo, float rate = 1.0f, float epsilon = 0.0f)
{
	return EulerAngle(
		Interpolate(orientFrom.GetX(), orientTo.GetX(), rate, epsilon),
		Interpolate(orientFrom.GetY(), orientTo.GetY(), rate, epsilon),
		Interpolate(orientFrom.GetZ(), orientTo.GetZ(), rate, epsilon)
	);
}

inline float EulerAngle::Interpolate(float angleFrom, float angleTo, float rate = 1.0f, float epsilon = 0.0f)
{
	rate = (abs(rate) > 1.0f) ? 1.0f : abs(rate);

	float difference = ShortestAngle(angleFrom, angleTo);
	if (abs(difference) > epsilon)
		return Clamp(angleFrom + (difference * rate));
	else
		return Clamp(angleTo);
}

inline EulerAngle EulerAngle::ShortestAngle(EulerAngle orientTo)
{
	return (orientTo - *this);
}

inline EulerAngle EulerAngle::ShortestAngle(EulerAngle orientFrom, EulerAngle orientTo)
{
	return (orientTo - orientFrom);
}

inline float EulerAngle::ShortestAngle(float angleFrom, float angleTo)
{
	return Clamp(angleTo - angleFrom);
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

inline Vector3 EulerAngle::ToVector3()
{
	return Vector3(this->GetX(), this->GetY(), this->GetZ());
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
	
	bool operator ==(Vector3Int vector)
	{
		return (x == vector.x && y == vector.y && z == vector.z);
	}
	
	bool operator !=(Vector3Int vector)
	{
		return (x != vector.x || y != vector.y || z != vector.z);
	}

	Vector3Int operator =(Vector3Int vector)
	{
		this->x = vector.x;
		this->y = vector.y;
		this->z = vector.z;
		return *this;
	}

	Vector3Int operator +(Vector3Int vector)
	{
		return Vector3Int(x + vector.x, y + vector.y, z + vector.z);
	}

	Vector3Int operator -(Vector3Int vector)
	{
		return Vector3Int(x - vector.x, y - vector.y, z - vector.z);
	}

	Vector3Int operator *(Vector3Int vector)
	{
		return Vector3Int(x * vector.x, y * vector.y, z * vector.z);
	}
	
	Vector3Int operator *(float value)
	{
		return Vector3Int((int)round(x * value), (int)round(y * value), (int)round(z * value));
	}

	Vector3Int operator /(float value)
	{
		return Vector3Int((int)round(x / value), (int)round(y / value), (int)round(z / value));
	}

	Vector3Int& operator +=(Vector3Int vector)
	{
		*this = *this + vector;
		return *this;
	}

	Vector3Int& operator -=(Vector3Int vector)
	{
		*this = *this - vector;
		return *this;
	}

	Vector3Int& operator *=(Vector3Int vector)
	{
		*this = *this * vector;
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

struct PoseData
{
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
