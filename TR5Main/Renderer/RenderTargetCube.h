#pragma once
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <DxErr.h>

class RenderTargetCube
{
private:
	LPDIRECT3DDEVICE9				m_device;
	LPDIRECT3DCUBETEXTURE9			m_renderTarget;
	LPDIRECT3DSURFACE9				m_renderTargetDepth;
	LPDIRECT3DSURFACE9				m_backBufferTarget;
	LPDIRECT3DSURFACE9				m_backBufferDepth;
	__int32							m_size;
	D3DFORMAT						m_format;

public:
	RenderTargetCube(LPDIRECT3DDEVICE9 device, __int32 size, D3DFORMAT format);
	~RenderTargetCube();
	bool							Bind(D3DCUBEMAP_FACES face);
	bool							Unbind();
	LPDIRECT3DCUBETEXTURE9			GetTexture();
};

