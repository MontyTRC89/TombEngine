#pragma once
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <DxErr.h>

class Shader
{
private:
	LPDIRECT3DDEVICE9		m_device;
	char*					m_fileName;

protected:
	LPD3DXEFFECT			m_effect;

public:
	Shader(LPDIRECT3DDEVICE9 device, char* fileName);
	~Shader();

	bool					Compile();
	LPD3DXEFFECT			GetEffect();
};