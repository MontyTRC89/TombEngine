#include "framework.h"
#include "Math/Angles/EulerAngles.h"

//namespace TEN::Math
//{
	EulerAngles const EulerAngles::Zero = EulerAngles(0, 0, 0);

	EulerAngles::EulerAngles()
	{
	}

	EulerAngles::EulerAngles(short x, short y, short z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}

	bool EulerAngles::operator ==(const EulerAngles& eulers)
	{
		return (x == eulers.x && y == eulers.y && z == eulers.z);
	}

	bool EulerAngles::operator !=(const EulerAngles& eulers)
	{
		return (x != eulers.x || y != eulers.y || z != eulers.z);
	}

	EulerAngles EulerAngles::operator +(const EulerAngles& eulers)
	{
		return EulerAngles(x + eulers.x, y + eulers.y, z + eulers.z);
	}

	EulerAngles EulerAngles::operator -(const EulerAngles& eulers)
	{
		return EulerAngles(x - eulers.x, y - eulers.y, z - eulers.z);
	}

	EulerAngles EulerAngles::operator *(const EulerAngles& eulers)
	{
		return EulerAngles(x * eulers.x, y * eulers.y, z * eulers.z);
	}

	EulerAngles EulerAngles::operator *(float value)
	{
		return EulerAngles((short)round(x * value), (short)round(y * value), (short)round(z * value));
	}

	EulerAngles EulerAngles::operator /(float value)
	{
		return EulerAngles((short)round(x / value), (short)round(y / value), (short)round(z / value));
	}

	EulerAngles& EulerAngles::operator =(const EulerAngles& eulers)
	{
		this->x = eulers.x;
		this->y = eulers.y;
		this->z = eulers.z;
		return *this;
	}

	EulerAngles& EulerAngles::operator +=(const EulerAngles& eulers)
	{
		this->x += eulers.x;
		this->y += eulers.y;
		this->z += eulers.z;
		return *this;
	}

	EulerAngles& EulerAngles::operator -=(const EulerAngles& eulers)
	{
		this->x -= eulers.x;
		this->y -= eulers.y;
		this->z -= eulers.z;
		return *this;
	}

	EulerAngles& EulerAngles::operator *=(const EulerAngles& eulers)
	{
		this->x *= eulers.x;
		this->y *= eulers.y;
		this->z *= eulers.z;
		return *this;
	}

	EulerAngles& EulerAngles::operator *=(float value)
	{
		this->x *= value;
		this->y *= value;
		this->z *= value;
		return *this;
	}

	EulerAngles& EulerAngles::operator /=(float value)
	{
		this->x /= value;
		this->y /= value;
		this->z /= value;
		return *this;
	}
//}
