#include "DepthShader.h"

DepthShader::DepthShader(LPDIRECT3DDEVICE9 device)
	: Shader(device, (char*)"Depth.fx")
{
	Initialise();
}


DepthShader::~DepthShader()
{

}

bool DepthShader::Initialise()
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
	m_texture = m_effect->GetParameterByName(NULL, "TextureAtlas");
}

bool DepthShader::SetWorld(D3DXMATRIX* value)
{
	return (m_effect->SetMatrix(m_world, value) == S_OK);
}

bool DepthShader::SetView(D3DXMATRIX* value)
{
	return (m_effect->SetMatrix(m_view, value) == S_OK);
}

bool DepthShader::SetProjection(D3DXMATRIX* value)
{
	return (m_effect->SetMatrix(m_projection, value) == S_OK);
}

bool DepthShader::SetBones(D3DXMATRIX* value, __int32 count)
{
	return (m_effect->SetMatrixArray(m_bones, value, count) == S_OK);
}

bool DepthShader::SetUseSkinning(bool value)
{
	return (m_effect->SetBool(m_useSkinning, value) == S_OK);
}

bool DepthShader::SetBlendMode(__int32 value)
{
	return (m_effect->SetInt(m_blendMode, value) == S_OK);
}

bool DepthShader::SetTexture(LPDIRECT3DBASETEXTURE9 value)
{
	return (m_effect->SetTexture(m_texture, value) == S_OK);
}