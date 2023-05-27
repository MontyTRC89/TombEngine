#include "framework.h"
#include "RGBAColor8Byte.h"

static byte FloatComponentToByte(float v)
{
	//todo look into what these actually do AND TEST THEM
	//todo like, see if these are actually not undefined or some shit
	auto lval = std::lroundf((v / 2.0f) * 255.0f);
	return static_cast<byte>(lval);
}

static float ByteComponentToFloat(byte b)
{
	//todo look into what these actually do AND TEST THEM
	//todo like, see if these are actually not undefined or some shit
	float f = b;
	f = (f / 255.0f) * 2.0f;
	return f;
}


RGBAColor8Byte::RGBAColor8Byte(D3DCOLOR col)
{
	b = col & 0xFF;
	col >>= 8;
	g = col & 0xFF;
	col >>= 8;
	r = col & 0xFF;
	col >>= 8;
	a = col & 0xFF;
}

RGBAColor8Byte::operator D3DCOLOR() const
{
	D3DCOLOR col = a;
	col <<= 8;
	col += r;
	col <<= 8;
	col += g;
	col <<= 8;
	col += b;

	return col;
}

byte RGBAColor8Byte::GetR() const
{
	return r;
}

void RGBAColor8Byte::SetR(byte v)
{
	r = std::clamp<byte>(v, 0, 255);
}

byte RGBAColor8Byte::GetG() const
{
	return g;
}

void RGBAColor8Byte::SetG(byte v)
{
	g = std::clamp<byte>(v, 0, 255);
}

byte RGBAColor8Byte::GetB() const
{
	return b;
}

void RGBAColor8Byte::SetB(byte v)
{
	b = std::clamp<byte>(v, 0, 255);
}

byte RGBAColor8Byte::GetA() const
{
	return a;
}

void RGBAColor8Byte::SetA(byte v)
{
	a = std::clamp<byte>(v, 0, 255);
}

RGBAColor8Byte::RGBAColor8Byte(Vector3 const& col) :
	r(FloatComponentToByte(col.x)),
	g(FloatComponentToByte(col.y)),
	b(FloatComponentToByte(col.z))
{
}

RGBAColor8Byte::RGBAColor8Byte(Vector4 const& col) :
	r(FloatComponentToByte(col.x)),
	g(FloatComponentToByte(col.y)),
	b(FloatComponentToByte(col.z)),
	a(FloatComponentToByte(col.w))
{
}

RGBAColor8Byte::operator Vector3() const
{
	return Vector3{ ByteComponentToFloat(r), ByteComponentToFloat(g), ByteComponentToFloat(b) };
}

RGBAColor8Byte::operator Vector4() const
{
	return Vector4{ ByteComponentToFloat(r), ByteComponentToFloat(g), ByteComponentToFloat(b), ByteComponentToFloat(a) };
}

RGBAColor8Byte::RGBAColor8Byte(byte r, byte g, byte b)
{
	SetR(r);
	SetG(g);
	SetB(b);
}

RGBAColor8Byte::RGBAColor8Byte(byte r, byte g, byte b, byte a) : RGBAColor8Byte(r, g, b)
{
	SetA(a);
}

