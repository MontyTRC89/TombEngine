#pragma once

typedef DWORD D3DCOLOR;

class RGBAColor8Byte
{
public:
	RGBAColor8Byte(D3DCOLOR);

	RGBAColor8Byte(byte r, byte g, byte b);
	RGBAColor8Byte(byte r, byte g, byte b, byte a);
	RGBAColor8Byte(Vector3 const &);
	RGBAColor8Byte(Vector4 const &);

	operator Vector3() const;
	operator Vector4() const;
	operator D3DCOLOR() const;

	byte GetR() const;
	void SetR(byte v);
	byte GetG() const;
	void SetG(byte v);
	byte GetB() const;
	void SetB(byte v);
	byte GetA() const;
	void SetA(byte v);

private:
	byte r{ 0 };
	byte g{ 0 };
	byte b{ 0 };
	byte a{ 255 };

};

