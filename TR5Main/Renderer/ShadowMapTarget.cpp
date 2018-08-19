#include "ShadowMapTarget.h"
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <DxErr.h>

ShadowMapTarget::ShadowMapTarget(LPDIRECT3DDEVICE9 device, __int32 w, __int32 h)
{
	m_device = device;
	m_width = w;
	m_height = h;

	m_target = NULL;
	m_target = new RenderTarget2D(device, w, h, D3DFORMAT::D3DFMT_R32F);

	LitItems.reserve(32);
}


ShadowMapTarget::~ShadowMapTarget()
{
	if (m_target != NULL)
		delete m_target;
}

RenderTarget2D* ShadowMapTarget::GetShadowMap()
{
	return m_target;
}