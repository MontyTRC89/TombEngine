#include "RenderTarget2D.h"
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <DxErr.h>

RenderTarget2D::RenderTarget2D(LPDIRECT3DDEVICE9 device, __int32 w, __int32 h, D3DFORMAT format)
{
	m_width = w;
	m_height = h;
	m_format = format;
	m_device = device;

	HRESULT res;

	m_renderTarget = NULL;
	m_renderTargetDepth = NULL;

	res = m_device->CreateTexture(w, h, 1, D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT, &m_renderTarget, NULL);
	if (res != S_OK)
		return;

	res = m_device->CreateDepthStencilSurface(w, h, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, true, &m_renderTargetDepth, NULL);
	if (res != S_OK)
		return;
}


RenderTarget2D::~RenderTarget2D()
{
	if (m_renderTarget != NULL)
		m_renderTarget->Release();
	if (m_renderTargetDepth != NULL)
		m_renderTargetDepth->Release();
}

bool RenderTarget2D::Bind()
{
	HRESULT res;

	m_backBufferTarget = NULL;
	res = m_device->GetRenderTarget(0, &m_backBufferTarget);
	if (res != S_OK)
		return false;

	LPDIRECT3DSURFACE9 surface;
	res = m_renderTarget->GetSurfaceLevel(0, &surface);
	if (res != S_OK)
		return false;

	res = m_device->SetRenderTarget(0, surface);
	if (res != S_OK)
		return false;

	surface->Release();

	m_backBufferDepth = NULL;
	res = m_device->GetDepthStencilSurface(&m_backBufferDepth);
	if (res != S_OK)
		return false;

	res = m_device->SetDepthStencilSurface(m_renderTargetDepth);
	if (res != S_OK)
		return false;
	 
	res = m_device->GetViewport(&m_backBufferViewport);
	if (res != S_OK)
		return false;

	/*D3DVIEWPORT9 viewport;
	viewport.X = 0;
	viewport.Y = 0;
	viewport.Width = m_width;
	viewport.Height = m_height;
	viewport.MinZ = 0.0f;
	viewport.MaxZ = 1.0f;

	res = m_device->SetViewport(&viewport);
	if (res != S_OK)
		return false;*/
	
	return true;
}

bool RenderTarget2D::Unbind()
{	
	HRESULT res;
	
	res = m_device->SetRenderTarget(0, m_backBufferTarget);
	if (res != S_OK)
		return false;

	res = m_device->SetDepthStencilSurface(m_backBufferDepth);
	if (res != S_OK)
		return false;

	/*res = m_device->SetViewport(&m_backBufferViewport);
	if (res != S_OK)
		return false;*/

	return true;
}

LPDIRECT3DTEXTURE9 RenderTarget2D::GetTexture()
{
	return m_renderTarget;
}

LPDIRECT3DSURFACE9 RenderTarget2D::GetDepthStencil()
{
	return m_renderTargetDepth;
}