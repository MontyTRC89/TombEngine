#pragma once

#include "Enums.h"
#include "RenderTarget2D.h"
#include "..\Global\global.h"

#include <vector>
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <DxErr.h>

using namespace std;

class ShadowMapTarget
{
private:
	RenderTarget2D*			m_target;
	__int32					m_width;
	__int32					m_height;
	LPDIRECT3DDEVICE9		m_device;

public:
	ShadowMapTarget(LPDIRECT3DDEVICE9 device, __int32 w, __int32 h);
	~ShadowMapTarget();

	RenderTarget2D*			GetShadowMap();

	D3DXVECTOR3				To;
	tr5_room_light*			Light;
	vector<__int32>			LitItems;
	D3DXMATRIX				View;
	D3DXMATRIX				Projection;
	D3DXMATRIX				ViewProjection;
};

