#pragma once

class Angle
{
public:
	// Utilities
	static float Normalize(float angle);
	static bool  Compare(float angle0, float angle1, float epsilon = 0.0f);
	static float ShortestAngularDistance(float angleFrom, float angleTo);
	static float InterpolateLinear(float angleFrom, float angleTo, float alpha = 1.0f, float epsilon = 0.0f);
	static float InterpolateConstant(float angleFrom, float angleTo, float rate);
	static float InterpolateConstantEaseOut(float angleFrom, float angleTo, float rate, float alpha = 1.0f, float epsilon = 0.0f);
	static float OrientBetweenPoints(Vector3 point0, Vector3 point1);

	static float DeltaHeading(Vector3 origin, Vector3 target, float heading); // TODO: I don't even know what this does.

	// Converters
	static float DegToRad(float degrees);
	static float RadToDeg(float radians);

	// Temporary legacy short form support for particularly cryptic code
	static float ShrtToRad(short shortForm);
	static short DegToShrt(float degrees);
	static short RadToShrt(float radians);

private:
	// Utilities
	static float ClampAlpha(float value);
	static float ClampEpsilon(float value);
};

//-------------------------------------------------------------------------------------------------------------------

// TODO: Move to EulerAngles.h and EulerAngles.cpp.
class EulerAngles
{
public:
	// Normalized angle components in radians
	float x;
	float y;
	float z;

	// Constructors
	EulerAngles();
	EulerAngles(float xAngle, float yAngle, float zAngle);

	// TODO: Roll back OOy get/set methods and use a strong typedef to define angles.

	// Getters
	float GetX();
	float GetY();
	float GetZ();

	// Setters
	void Set(EulerAngles orient);
	void Set(float xAngle, float yAngle, float zAngle);
	void SetX(float angle = 0.0f);
	void SetY(float angle = 0.0f);
	void SetZ(float angle = 0.0f);

	// Utilities
	void Normalize();
	static EulerAngles Normalize(EulerAngles orient);

	bool Compare(EulerAngles orient, float epsilon = 0.0f);
	static bool Compare(EulerAngles orient0, EulerAngles orient1, float epsilon = 0.0f);

	EulerAngles ShortestAngularDistance(EulerAngles orientTo);
	static EulerAngles ShortestAngularDistance(EulerAngles orientFrom, EulerAngles orientTo);

	void InterpolateLinear(EulerAngles orientTo, float alpha = 1.0f, float epsilon = 0.0f);
	static EulerAngles InterpolateLinear(EulerAngles orientFrom, EulerAngles orientTo, float alpha = 1.0f, float epsilon = 0.0f);

	void InterpolateConstant(EulerAngles orientTo, float rate);
	static EulerAngles InterpolateConstant(EulerAngles orientFrom, EulerAngles orientTo, float rate);

	void InterpolateConstantEaseOut(EulerAngles orientTo, float rate, float alpha = 1.0f, float epsilon = 0.0f);
	static EulerAngles InterpolateConstantEaseOut(EulerAngles orientFrom, EulerAngles orientTo, float rate, float alpha = 1.0f, float epsilon = 0.0f);

	static EulerAngles OrientBetweenPoints(Vector3 point0, Vector3 point1);

	Vector3 ToVector3();

	// Constants
	static const EulerAngles Zero;

	// Operators
	bool operator ==(EulerAngles orient);
	bool operator !=(EulerAngles orient);

	EulerAngles operator +(EulerAngles orient);
	EulerAngles operator -(EulerAngles orient);
	EulerAngles operator *(EulerAngles orient);
	EulerAngles operator *(float value);
	EulerAngles operator /(float value);

	EulerAngles& operator +=(EulerAngles orient);
	EulerAngles& operator -=(EulerAngles orient);
	EulerAngles& operator *=(EulerAngles orient);
	EulerAngles& operator *=(float value);
	EulerAngles& operator /=(float value);
};

inline EulerAngles::EulerAngles()
{
	this->Set(0.0f, 0.0f, 0.0f);
}

inline EulerAngles::EulerAngles(float xAngle, float yAngle, float zAngle)
{
	this->Set(xAngle, yAngle, zAngle);
}

inline float EulerAngles::GetX()
{
	return x;
}

inline float EulerAngles::GetY()
{
	return y;
}

inline float EulerAngles::GetZ()
{
	return z;
}

inline void EulerAngles::Set(EulerAngles orient)
{
	*this = orient;
}

inline void EulerAngles::Set(float xAngle, float yAngle, float zAngle)
{
	this->SetX(xAngle);
	this->SetY(yAngle);
	this->SetZ(zAngle);
}

inline void EulerAngles::SetX(float angle)
{
	this->x = Angle::Normalize(angle);
}

inline void EulerAngles::SetY(float angle)
{
	this->y = Angle::Normalize(angle);
}

inline void EulerAngles::SetZ(float angle)
{
	this->z = Angle::Normalize(angle);
}

inline void EulerAngles::Normalize()
{
	this->Set(this->GetX(), this->GetY(), this->GetZ());
}

inline EulerAngles EulerAngles::Normalize(EulerAngles orient)
{
	orient.Normalize();
	return orient;
}

inline bool EulerAngles::Compare(EulerAngles orient, float epsilon)
{
	return Compare(*this, orient, epsilon);
}

inline bool EulerAngles::Compare(EulerAngles orient0, EulerAngles orient1, float epsilon)
{
	if (Angle::Compare(orient0.x, orient1.x, epsilon) &&
		Angle::Compare(orient0.y, orient1.y, epsilon) &&
		Angle::Compare(orient0.z, orient1.z, epsilon))
	{
		return true;
	}

	return false;
}

inline EulerAngles EulerAngles::ShortestAngularDistance(EulerAngles orientTo)
{
	return (orientTo - *this);
}

inline EulerAngles EulerAngles::ShortestAngularDistance(EulerAngles orientFrom, EulerAngles orientTo)
{
	return (orientTo - orientFrom);
}

inline void EulerAngles::InterpolateLinear(EulerAngles orientTo, float alpha, float epsilon)
{
	this->Set(
		Angle::InterpolateLinear(this->GetX(), orientTo.GetX(), alpha, epsilon),
		Angle::InterpolateLinear(this->GetY(), orientTo.GetY(), alpha, epsilon),
		Angle::InterpolateLinear(this->GetZ(), orientTo.GetZ(), alpha, epsilon));
}

inline EulerAngles EulerAngles::InterpolateLinear(EulerAngles orientFrom, EulerAngles orientTo, float alpha, float epsilon)
{
	return EulerAngles(
		Angle::InterpolateLinear(orientFrom.GetX(), orientTo.GetX(), alpha, epsilon),
		Angle::InterpolateLinear(orientFrom.GetY(), orientTo.GetY(), alpha, epsilon),
		Angle::InterpolateLinear(orientFrom.GetZ(), orientTo.GetZ(), alpha, epsilon)
	);
}

inline void EulerAngles::InterpolateConstant(EulerAngles orientTo, float rate)
{
	this->Set(
		Angle::InterpolateConstant(this->GetX(), orientTo.GetX(), rate),
		Angle::InterpolateConstant(this->GetY(), orientTo.GetY(), rate),
		Angle::InterpolateConstant(this->GetZ(), orientTo.GetZ(), rate));
}

inline EulerAngles EulerAngles::InterpolateConstant(EulerAngles orientFrom, EulerAngles orientTo, float rate)
{
	return EulerAngles(
		Angle::InterpolateConstant(orientFrom.GetX(), orientTo.GetX(), rate),
		Angle::InterpolateConstant(orientFrom.GetY(), orientTo.GetY(), rate),
		Angle::InterpolateConstant(orientFrom.GetZ(), orientTo.GetZ(), rate)
	);
}

inline void EulerAngles::InterpolateConstantEaseOut(EulerAngles orientTo, float rate, float alpha, float epsilon)
{
	this->Set(
		Angle::InterpolateConstantEaseOut(this->GetX(), orientTo.GetX(), rate, alpha, epsilon),
		Angle::InterpolateConstantEaseOut(this->GetY(), orientTo.GetY(), rate, alpha, epsilon),
		Angle::InterpolateConstantEaseOut(this->GetZ(), orientTo.GetZ(), rate, alpha, epsilon));
}

inline EulerAngles EulerAngles::InterpolateConstantEaseOut(EulerAngles orientFrom, EulerAngles orientTo, float rate, float alpha, float epsilon)
{
	return EulerAngles(
		Angle::InterpolateConstantEaseOut(orientFrom.GetX(), orientTo.GetX(), rate, alpha, epsilon),
		Angle::InterpolateConstantEaseOut(orientFrom.GetY(), orientTo.GetY(), rate, alpha, epsilon),
		Angle::InterpolateConstantEaseOut(orientFrom.GetZ(), orientTo.GetZ(), rate, alpha, epsilon)
	);
}

inline EulerAngles EulerAngles::OrientBetweenPoints(Vector3 point0, Vector3 point1)
{
	auto direction = point1 - point0;

	const float yOrient = atan2(direction.x, direction.z);

	auto vector = Vector3(direction.x, direction.y, direction.z);
	const auto matrix = Matrix::CreateRotationY(-yOrient);
	Vector3::Transform(vector, matrix, vector);

	float xOrient = -atan2(direction.y, vector.z);
	return EulerAngles(xOrient, yOrient, 0.0f);
}

inline Vector3 EulerAngles::ToVector3()
{
	return Vector3(this->GetX(), this->GetY(), this->GetZ());
}

inline EulerAngles const EulerAngles::Zero = EulerAngles(0.0f, 0.0f, 0.0f);

inline bool EulerAngles::operator ==(EulerAngles orient)
{
	auto orient0 = *this;
	auto orient1 = orient;
	return (orient0.GetX() == orient1.GetX() && orient0.GetY() == orient1.GetY() && orient0.GetZ() == orient1.GetZ());
}

inline bool EulerAngles::operator !=(EulerAngles orient)
{
	auto orient0 = *this;
	auto orient1 = orient;
	return (orient0.GetX() != orient1.GetX() || orient0.GetY() != orient1.GetY() || orient0.GetZ() != orient1.GetZ());
}

inline EulerAngles EulerAngles::operator +(EulerAngles orient)
{
	return EulerAngles(this->GetX() + orient.GetX(), this->GetY() + orient.GetY(), this->GetZ() + orient.GetZ());
}

inline EulerAngles EulerAngles::operator -(EulerAngles orient)
{
	return EulerAngles(this->GetX() - orient.GetX(), this->GetY() - orient.GetY(), this->GetZ() - orient.GetZ());
}

inline EulerAngles EulerAngles::operator *(EulerAngles orient)
{
	return EulerAngles(this->GetX() * orient.GetX(), this->GetY() * orient.GetY(), this->GetZ() * orient.GetZ());
}

inline EulerAngles EulerAngles::operator *(float value)
{
	return EulerAngles(this->GetX() * value, this->GetY() * value, this->GetZ() * value);
}

inline EulerAngles EulerAngles::operator /(float value)
{
	return EulerAngles(this->GetX() / value, this->GetY() / value, this->GetZ() / value);
}

inline EulerAngles& EulerAngles::operator +=(EulerAngles orient)
{
	this->Set(this->GetX() + orient.GetX(), this->GetY() + orient.GetY(), this->GetZ() + orient.GetZ());
	return *this;
}

inline EulerAngles& EulerAngles::operator -=(EulerAngles orient)
{
	this->Set(this->GetX() - orient.GetX(), this->GetY() - orient.GetY(), this->GetZ() - orient.GetZ());
	return *this;
}

inline EulerAngles& EulerAngles::operator *=(EulerAngles orient)
{
	this->Set(this->GetX() * orient.GetX(), this->GetY() * orient.GetY(), this->GetZ() * orient.GetZ());
	return *this;
}

inline EulerAngles& EulerAngles::operator *=(float value)
{
	this->Set(this->GetX() * value, this->GetY() * value, this->GetZ() * value);
	return *this;
}

inline EulerAngles& EulerAngles::operator /=(float value)
{
	this->Set(this->GetX() / value, this->GetY() / value, this->GetZ() / value);
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

	Vector2 ToVector2()
	{
		return Vector2(x, y);
	}
};

struct Vector3Int
{
	int x;
	int y;
	int z;

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
};

struct RendererRectangle
{
	int left;
	int top;
	int right;
	int bottom;

	RendererRectangle()
	{
		left = 0;
		top = 0;
		right = 0;
		bottom = 0;
	}

	RendererRectangle(int left, int top, int right, int bottom)
	{
		this->left = left;
		this->top = top;
		this->right = right;
		this->bottom = bottom;
	}
};

struct PHD_3DPOS
{
	Vector3Int  Position;
	EulerAngles Orientation;

	PHD_3DPOS()
	{
		this->Position = Vector3Int();
		this->Orientation = EulerAngles::Zero;
	}

	PHD_3DPOS(Vector3Int pos)
	{
		this->Position = pos;
		this->Orientation = EulerAngles::Zero;
	}

	PHD_3DPOS(int xPos, int yPos, int zPos)
	{
		this->Position = Vector3Int(xPos, yPos, zPos);
		this->Orientation = EulerAngles::Zero;
	}

	PHD_3DPOS(EulerAngles orient)
	{
		this->Position = Vector3Int();
		this->Orientation = orient;
	}

	PHD_3DPOS(float xOrient, float yOrient, float zOrient)
	{
		this->Position = Vector3Int();
		this->Orientation.Set(xOrient, yOrient, zOrient);
	}

	PHD_3DPOS(Vector3Int pos, EulerAngles orient)
	{
		this->Position = pos;
		this->Orientation = orient;
	}

	PHD_3DPOS(Vector3Int pos, float xOrient, float yOrient, float zOrient)
	{
		this->Position = pos;
		this->Orientation.Set(xOrient, yOrient, zOrient);
	}

	PHD_3DPOS(int xPos, int yPos, int zPos, EulerAngles orient)
	{
		this->Position = Vector3Int(xPos, yPos, zPos);
		this->Orientation = orient;
	}

	PHD_3DPOS(int xPos, int yPos, int zPos, float xOrient, float yOrient, float zOrient)
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

BOUNDING_BOX operator+(BOUNDING_BOX const& box, PHD_3DPOS const& vec);
