#pragma once
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <DxErr.h>

class RenderTarget2D
{
private:
	LPDIRECT3DDEVICE9				m_device;
	LPDIRECT3DTEXTURE9				m_renderTarget;
	LPDIRECT3DSURFACE9				m_renderTargetDepth;
	LPDIRECT3DSURFACE9				m_backBufferTarget;
	LPDIRECT3DSURFACE9				m_backBufferDepth;
	__int32							m_width;
	__int32							m_height;
	D3DFORMAT						m_format;
	D3DVIEWPORT9					m_backBufferViewport;

public:
	RenderTarget2D(LPDIRECT3DDEVICE9 device, __int32 w, __int32 h, D3DFORMAT format);
	~RenderTarget2D();

	bool							Bind();
	bool							Unbind();
	LPDIRECT3DTEXTURE9				GetTexture();
	LPDIRECT3DSURFACE9				GetDepthStencil();
};

