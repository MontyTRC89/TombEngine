#pragma once
#include "Shader.h"
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <DxErr.h>

class MainShader :
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
	D3DXHANDLE			m_lightView;
	D3DXHANDLE			m_lightProjection;
	D3DXHANDLE			m_enableShadows;
	D3DXHANDLE			m_lightPosition;
	D3DXHANDLE			m_lightDirection;
	D3DXHANDLE			m_lightColor;
	D3DXHANDLE			m_lightType;
	D3DXHANDLE			m_lightIn;
	D3DXHANDLE			m_lightOut;
	D3DXHANDLE			m_lightRange;
	D3DXHANDLE			m_lightActive;
	D3DXHANDLE			m_ambientLight;
	D3DXHANDLE			m_modelType;
	D3DXHANDLE			m_shadowMap;
	D3DXHANDLE			m_cameraPosition;

public:
	MainShader(LPDIRECT3DDEVICE9 device);
	~MainShader();

	bool				Initialise();
	bool				SetWorld(D3DXMATRIX* value);
	bool				SetView(D3DXMATRIX* value);
	bool				SetProjection(D3DXMATRIX* value);
	bool				SetBones(D3DXMATRIX* value, __int32 count);
	bool				SetUseSkinning(bool value);
	bool				SetBlendMode(__int32 value);
	bool				SetTexture(LPDIRECT3DBASETEXTURE9 value);
	bool				SetShadowMap(LPDIRECT3DBASETEXTURE9 value);
	bool				SetLightView(D3DXMATRIX* value);
	bool				SetLightProjection(D3DXMATRIX* value);
	bool				SetLightActive(bool value);
	bool				SetEnableShadows(bool value);
	bool				SetLightPosition(D3DXVECTOR4* value);
	bool				SetLightDirection(D3DXVECTOR4* value);
	bool				SetLightColor(D3DXVECTOR4* value);
	bool				SetLightType(__int32 value);
	bool				SetLightIn(float value);
	bool				SetLightOut(float value);
	bool				SetLightRange(float value);
	bool				SetCameraPosition(D3DXVECTOR4* value);
	bool				SetAmbientLight(D3DXVECTOR4* value);
	bool				SetModelType(__int32 value);
	bool				SetDynamicLight(bool value);
};

