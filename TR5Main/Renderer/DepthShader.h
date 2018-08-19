#pragma once
#include "Shader.h"
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <DxErr.h>

class DepthShader : 
	public Shader
{
private:
	D3DXHANDLE			m_world;
	D3DXHANDLE			m_view;
	D3DXHANDLE			m_projection;
	D3DXHANDLE			m_bones;
	D3DXHANDLE			m_useSkinning;
	D3DXHANDLE			m_blendMode;
	D3DXHANDLE			m_texture;

public:
	DepthShader(LPDIRECT3DDEVICE9 device);
	~DepthShader();

	bool				Initialise();
	bool				SetWorld(D3DXMATRIX* value);
	bool				SetView(D3DXMATRIX* value);
	bool				SetProjection(D3DXMATRIX* value);
	bool				SetBones(D3DXMATRIX* value, __int32 count);
	bool				SetUseSkinning(bool value);
	bool				SetBlendMode(__int32 value);
	bool				SetTexture(LPDIRECT3DBASETEXTURE9 value);
};

