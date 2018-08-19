#include "Shader.h"

Shader::Shader(LPDIRECT3DDEVICE9 device, char* fileName)
{
	m_device = device;
	m_fileName = fileName;
	m_effect = NULL;
}
 
Shader::~Shader()
{
	if (m_effect != NULL)
		m_effect->Release();
}

bool Shader::Compile()
{
	printf("Compiling shader %s ...\n", m_fileName);

	__int64 flags = D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY;
	LPD3DXBUFFER errors = NULL;

	HRESULT res = D3DXCreateEffectFromFile(m_device, m_fileName, NULL, NULL, flags, NULL, &m_effect, &errors);

	if (res != S_OK)
	{
		//char* message = (char*)errors->GetBufferPointer();
		//printf("%s\n", message);
		//errors->Release();

		return false;
	}
	
	printf("Compilation successful\n");

	return true;
}

LPD3DXEFFECT Shader::GetEffect()
{
 	return m_effect;
}