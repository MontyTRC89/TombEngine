#include "RenderTargetCube.h"
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <DxErr.h>

RenderTargetCube::RenderTargetCube(LPDIRECT3DDEVICE9 device, __int32 size, D3DFORMAT format)
{
	m_size = size;
	m_format = format;
	m_device = device;

	HRESULT res;

	m_renderTarget = NULL;
	m_renderTargetDepth = NULL;

	res = m_device->CreateCubeTexture(size, 1, D3DUSAGE_RENDERTARGET, format, D3DPOOL_DEFAULT, &m_renderTarget, NULL);
	if (res != S_OK)
		return;

	res = m_device->CreateDepthStencilSurface(size, size, D3DFMT_D24S8, D3DMULTISAMPLE_NONE, 0, true, &m_renderTargetDepth, NULL);
	if (res != S_OK)
		return;
}


RenderTargetCube::~RenderTargetCube()
{
	if (m_renderTarget != NULL)
		m_renderTarget->Release();
	if (m_renderTargetDepth != NULL)
		m_renderTargetDepth->Release();
}

bool RenderTargetCube::Bind(D3DCUBEMAP_FACES face)
{
	HRESULT res;

	m_backBufferTarget = NULL;
	res = m_device->GetRenderTarget(0, &m_backBufferTarget);
	if (res != S_OK)
		return false;

	LPDIRECT3DSURFACE9 surface;
	res = m_renderTarget->GetCubeMapSurface(face, 0, &surface);
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

	return true;
}

bool RenderTargetCube::Unbind()
{
	HRESULT res;

	res = m_device->SetRenderTarget(0, m_backBufferTarget);
	if (res != S_OK)
		return false;

	res = m_device->SetDepthStencilSurface(m_backBufferDepth);
	if (res != S_OK)
		return false;

	return true;
}

LPDIRECT3DCUBETEXTURE9 RenderTargetCube::GetTexture()
{
	return m_renderTarget;
}