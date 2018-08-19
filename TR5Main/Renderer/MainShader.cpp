#include "MainShader.h"

MainShader::MainShader(LPDIRECT3DDEVICE9 device)
	: Shader(device, (char*)"Shader.fx")
{
	Initialise();
}

MainShader::~MainShader()
{

}

bool MainShader::Initialise()
{
	if (!Compile())
		return false;

	// Get handles for all parameters
	m_world = m_effect->GetParameterByName(NULL, "World");
	m_view = m_effect->GetParameterByName(NULL, "View");
	m_projection = m_effect->GetParameterByName(NULL, "Projection");
	m_useSkinning = m_effect->GetParameterByName(NULL, "UseSkinning");
	m_blendMode = m_effect->GetParameterByName(NULL, "BlendMode");
	m_bones = m_effect->GetParameterByName(NULL, "Bones");
	m_enableShadows = m_effect->GetParameterByName(NULL, "EnableShadows");
	m_lightActive = m_effect->GetParameterByName(NULL, "LightActive");
	m_lightPosition = m_effect->GetParameterByName(NULL, "LightPosition");
	m_lightColor = m_effect->GetParameterByName(NULL, "LightColor");
	m_lightDirection = m_effect->GetParameterByName(NULL, "LightDirection");
	m_lightType = m_effect->GetParameterByName(NULL, "LightType");
	m_lightIn = m_effect->GetParameterByName(NULL, "LightIn");
	m_lightOut = m_effect->GetParameterByName(NULL, "LightOut");
	m_lightRange = m_effect->GetParameterByName(NULL, "LightRange");
	m_ambientLight = m_effect->GetParameterByName(NULL, "AmbientLight");
	m_shadowMap = m_effect->GetParameterByName(NULL, "ShadowMap");
	m_modelType = m_effect->GetParameterByName(NULL, "ModelType");
	m_cameraPosition = m_effect->GetParameterByName(NULL, "CameraPosition");
	m_lightView = m_effect->GetParameterByName(NULL, "LightView");
	m_lightProjection = m_effect->GetParameterByName(NULL, "LightProjection");
	m_texture = m_effect->GetParameterByName(NULL, "TextureAtlas");
	m_shadowMap = m_effect->GetParameterByName(NULL, "ShadowMap");
}

bool MainShader::SetWorld(D3DXMATRIX* value)
{
	return (m_effect->SetMatrix(m_effect->GetParameterByName(NULL, "World"), value) == S_OK);
}

bool MainShader::SetView(D3DXMATRIX* value)
{
	return (m_effect->SetMatrix(m_effect->GetParameterByName(NULL, "View"), value) == S_OK);
}

bool MainShader::SetProjection(D3DXMATRIX* value)
{
	return (m_effect->SetMatrix(m_effect->GetParameterByName(NULL, "Projection"), value) == S_OK);
}

bool MainShader::SetBones(D3DXMATRIX* value, __int32 count)
{
	return (m_effect->SetMatrixArray(m_effect->GetParameterByName(NULL, "Bones"), value, count) == S_OK);
}

bool MainShader::SetUseSkinning(bool value)
{
	return (m_effect->SetBool(m_effect->GetParameterByName(NULL, "UseSkinning"), value) == S_OK);
}

bool MainShader::SetBlendMode(__int32 value)
{
	return (m_effect->SetInt(m_effect->GetParameterByName(NULL, "BlendMode"), value) == S_OK);
}

bool MainShader::SetTexture(LPDIRECT3DBASETEXTURE9 value)
{
	return (m_effect->SetTexture(m_effect->GetParameterByName(NULL, "TextureAtlas"), value) == S_OK);
}

bool MainShader::SetShadowMap(LPDIRECT3DBASETEXTURE9 value)
{
	return (m_effect->SetTexture(m_effect->GetParameterByName(NULL, "ShadowMap"), value) == S_OK);
}

bool MainShader::SetLightView(D3DXMATRIX* value)
{
	return (m_effect->SetMatrix(m_effect->GetParameterByName(NULL, "LightView"), value) == S_OK);
}

bool MainShader::SetLightProjection(D3DXMATRIX* value)
{
	return (m_effect->SetMatrix(m_effect->GetParameterByName(NULL, "LightProjection"), value) == S_OK);
}

bool MainShader::SetLightActive(bool value)
{
	return (m_effect->SetBool(m_effect->GetParameterByName(NULL, "LightActive"), value) == S_OK);
}

bool MainShader::SetEnableShadows(bool value)
{
	return (m_effect->SetBool(m_effect->GetParameterByName(NULL, "EnableShadows"), value) == S_OK);
}

bool MainShader::SetLightPosition(D3DXVECTOR4* value)
{
	return (m_effect->SetVector(m_effect->GetParameterByName(NULL, "LightPosition"), value) == S_OK);
}

bool MainShader::SetLightDirection(D3DXVECTOR4* value)
{
	return (m_effect->SetVector(m_effect->GetParameterByName(NULL, "LightDirection"), value) == S_OK);
}

bool MainShader::SetLightColor(D3DXVECTOR4* value)
{
	return (m_effect->SetVector(m_effect->GetParameterByName(NULL, "LightColor"), value) == S_OK);
}

bool MainShader::SetLightType(__int32 value)
{
	return (m_effect->SetInt(m_effect->GetParameterByName(NULL, "LightType"), value) == S_OK);
}

bool MainShader::SetLightIn(float value)
{
	return (m_effect->SetFloat(m_effect->GetParameterByName(NULL, "LightIn"), value) == S_OK);
}

bool MainShader::SetLightOut(float value)
{
	return (m_effect->SetFloat(m_effect->GetParameterByName(NULL, "LightOut"), value) == S_OK);
}

bool MainShader::SetLightRange(float value)
{
	return (m_effect->SetFloat(m_effect->GetParameterByName(NULL, "LightRange"), value) == S_OK);
}

bool MainShader::SetCameraPosition(D3DXVECTOR4* value)
{
	return (m_effect->SetVector(m_effect->GetParameterByName(NULL, "CameraPosition"), value) == S_OK);
}

bool MainShader::SetAmbientLight(D3DXVECTOR4* value)
{
	return (m_effect->SetVector(m_effect->GetParameterByName(NULL, "AmbientLight"), value) == S_OK);
}

bool MainShader::SetModelType(__int32 value)
{
	return (m_effect->SetInt(m_effect->GetParameterByName(NULL, "ModelType"), value) == S_OK);
}

bool MainShader::SetDynamicLight(bool value)
{
	return (m_effect->SetBool(m_effect->GetParameterByName(NULL, "DynamicLight"), value) == S_OK);
}