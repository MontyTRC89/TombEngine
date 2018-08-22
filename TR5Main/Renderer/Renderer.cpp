#include "Renderer.h"

#include "Structs.h"
#include "Enums.h"
#include "RendererBucket.h"
#include "RendererMesh.h"
#include "RendererObject.h"

#include <stdio.h>
#include <stack> 
#include <chrono> 

#include <d3d9.h>
#include <d3dx9.h>
#include <DxErr.h>

#include "..\Global\global.h"

#include "..\Specific\input.h"
#include "..\Specific\winmain.h"
#include "..\Specific\roomload.h"
#include "..\Specific\game.h"

#include "..\Game\draw.h"
#include "..\Game\healt.h"
#include "..\Game\pickup.h"
#include "..\Game\inventory.h"
#include "..\Game\gameflow.h"
#include "..\Game\lara.h"

using ns = chrono::nanoseconds;
using get_time = chrono::steady_clock;

Renderer::Renderer()
{
	memset(m_skinJointRemap, -1, 15 * 128 * 2);

	m_skinJointRemap[1][0] = 0;
	m_skinJointRemap[1][1] = 0;
	m_skinJointRemap[1][2] = 0;
	m_skinJointRemap[1][3] = 0;
	m_skinJointRemap[1][4] = 0;
	m_skinJointRemap[1][5] = 0;

	m_skinJointRemap[2][0] = 1;
	m_skinJointRemap[2][1] = 1;
	m_skinJointRemap[2][2] = 1;
	m_skinJointRemap[2][3] = 1;
	m_skinJointRemap[2][4] = 1;

	m_skinJointRemap[3][4] = 2;
	m_skinJointRemap[3][5] = 2;
	m_skinJointRemap[3][6] = 2;
	m_skinJointRemap[3][7] = 2;

	m_skinJointRemap[4][0] = 0;
	m_skinJointRemap[4][1] = 0;
	m_skinJointRemap[4][2] = 0;
	m_skinJointRemap[4][3] = 0;
	m_skinJointRemap[4][4] = 0;
	m_skinJointRemap[4][5] = 0;

	m_skinJointRemap[5][0] = 4;
	m_skinJointRemap[5][1] = 4;
	m_skinJointRemap[5][2] = 4;
	m_skinJointRemap[5][3] = 4;
	m_skinJointRemap[5][4] = 4;

	m_skinJointRemap[6][4] = 5;
	m_skinJointRemap[6][5] = 5;
	m_skinJointRemap[6][6] = 5;
	m_skinJointRemap[6][7] = 5;

	m_skinJointRemap[7][0] = 0;
	m_skinJointRemap[7][1] = 0;
	m_skinJointRemap[7][2] = 0;
	m_skinJointRemap[7][3] = 0;
	m_skinJointRemap[7][4] = 0;
	m_skinJointRemap[7][5] = 0;

	m_skinJointRemap[8][6] = 7;
	m_skinJointRemap[8][7] = 7;
	m_skinJointRemap[8][8] = 7;
	m_skinJointRemap[8][9] = 7;
	m_skinJointRemap[8][10] = 7;
	m_skinJointRemap[8][11] = 7;

	m_skinJointRemap[9][5] = 8;
	m_skinJointRemap[9][6] = 8;
	m_skinJointRemap[9][7] = 8;
	m_skinJointRemap[9][8] = 8;
	m_skinJointRemap[9][9] = 8;

	m_skinJointRemap[10][0] = 9;
	m_skinJointRemap[10][1] = 9;
	m_skinJointRemap[10][2] = 9;
	m_skinJointRemap[10][3] = 9;
	m_skinJointRemap[10][4] = 9;

	m_skinJointRemap[11][6] = 7;
	m_skinJointRemap[11][7] = 7;
	m_skinJointRemap[11][8] = 7;
	m_skinJointRemap[11][9] = 7;
	m_skinJointRemap[11][10] = 7;
	m_skinJointRemap[11][11] = 7;

	m_skinJointRemap[12][5] = 11;
	m_skinJointRemap[12][6] = 11;
	m_skinJointRemap[12][7] = 11;
	m_skinJointRemap[12][8] = 11;
	m_skinJointRemap[12][9] = 11;

	m_skinJointRemap[13][0] = 12;
	m_skinJointRemap[13][1] = 12;
	m_skinJointRemap[13][2] = 12;
	m_skinJointRemap[13][3] = 12;
	m_skinJointRemap[13][4] = 12;

	m_skinJointRemap[14][6] = 7;
	m_skinJointRemap[14][7] = 7;
	m_skinJointRemap[14][8] = 7;
	m_skinJointRemap[14][9] = 7;
	m_skinJointRemap[14][10] = 7;
	m_skinJointRemap[14][11] = 7;

	m_quadVertices[0].x = -1;
	m_quadVertices[0].y = 1;
	m_quadVertices[0].z = 1;
	m_quadVertices[0].u = 0;
	m_quadVertices[0].v = 0;

	m_quadVertices[1].x = 1;
	m_quadVertices[1].y = 1;
	m_quadVertices[1].z = 1;
	m_quadVertices[1].u = 1;
	m_quadVertices[1].v = 0;

	m_quadVertices[2].x = -1;
	m_quadVertices[2].y = -1;
	m_quadVertices[2].z = 1;
	m_quadVertices[2].u = 0;
	m_quadVertices[2].v = 1;

	m_quadVertices[3].x = 1;
	m_quadVertices[3].y = -1;
	m_quadVertices[3].z = 1;
	m_quadVertices[3].u = 1;
	m_quadVertices[3].v = 1;

	m_quadIndices[0] = 0;
	m_quadIndices[1] = 3;
	m_quadIndices[2] = 2;
	m_quadIndices[3] = 0;
	m_quadIndices[4] = 1;
	m_quadIndices[5] = 3;
	 
	for (__int32 i = 0; i < 1000; i++)
	{ 
		float x = rand() % 16000 + 2048;
		float y = rand() % 2048 + 256;
		float z = rand() % 16000 + 2048;

		float r = (rand() % 255) / 255.0f;
		float g = (rand() % 255) / 255.0f;
		float b = (rand() % 255) / 255.0f;

		RendererLight* light = new RendererLight();
		light->Position = D3DXVECTOR4(x, y, z, 1);
		light->Color = D3DXVECTOR4(r, g, b, 1);
		light->Type = LIGHT_TYPES::LIGHT_TYPE_POINT;
		light->Out = 1024.0f;

		m_testLights.push_back(light);
	}
}

Renderer::~Renderer()
{
	DX_RELEASE(m_d3D);
	DX_RELEASE(m_device);
	DX_RELEASE(m_textureAtlas);
	DX_RELEASE(m_fontAndMiscTexture);
	DX_RELEASE(m_debugFont);
	DX_RELEASE(m_gameFont);
	DX_RELEASE(m_skyTexture);

	delete m_lightBuffer;
	delete m_normalBuffer;
	delete m_vertexLightBuffer;
	delete m_colorBuffer;
	delete m_outputBuffer;
	delete m_shaderClearGBuffer;
	delete m_shaderFillGBuffer;
	delete m_shaderLight;
	delete m_shaderCombine;
	delete m_sphereMesh;
	delete m_shadowMap;
	delete m_renderTarget;
}

bool Renderer::Initialise(__int32 w, __int32 h, bool windowed, HWND handle)
{
	HRESULT res;

	DB_Log(2, "Renderer::Initialise - DLL");
	printf("Initialising DX\n");

	m_d3D = Direct3DCreate9(D3D_SDK_VERSION);
	if (m_d3D == NULL) 
		return false;

	D3DPRESENT_PARAMETERS d3dpp;
	 
	ScreenWidth = w;
	ScreenHeight = h;
	Windowed = windowed;

	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = windowed;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = handle;
	d3dpp.BackBufferWidth = w;
	d3dpp.BackBufferHeight = h;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;

	res = m_d3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, handle, D3DCREATE_HARDWARE_VERTEXPROCESSING,
							  &d3dpp, &m_device);
	if (res != S_OK)
		return false;

	// Load the white sprite 
	D3DXCreateTextureFromFileEx(m_device, "load.bmp", D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0,
								D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
								D3DCOLOR_XRGB(255, 255, 255), NULL, NULL, &m_titleScreen);

	// Create effects
	m_mainShader = new MainShader(m_device);
	if (m_mainShader->GetEffect() == NULL)
 		return false;

	m_depthShader = new DepthShader(m_device);
	if (m_depthShader->GetEffect() == NULL)
		return false;

	// Initialise the vertex declaration 
	D3DVERTEXELEMENT9 roomVertexElements[] =
						{
							{ 0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
							{ 0, 12, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },
							{ 0, 24, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
							{ 0, 32, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
							{ 0, 48, D3DDECLTYPE_FLOAT1,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
							D3DDECL_END()
						};
	
	m_vertexDeclaration = NULL;
	res = m_device->CreateVertexDeclaration(roomVertexElements, &m_vertexDeclaration);
	if (res != S_OK)
		return false;

	// Initialise fonts
	res = D3DXCreateFont(m_device, 13, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &m_debugFont);
	if (res != S_OK)
		return false;

	res = D3DXCreateFont(m_device, 28, 0, FW_NORMAL, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Ondine"), &m_gameFont);
	if (res != S_OK)
		return false;

	// Initialise the sprite object
	res = D3DXCreateSprite(m_device, &m_sprite);
	if (res != S_OK)
		return false;

	// Initialise line objects
	m_lines = (RendererLine2D*)malloc(MAX_LINES_2D * sizeof(RendererLine2D));
	m_line = NULL;
	res = D3DXCreateLine(m_device, &m_line);
	if (res != S_OK)
		return false;

	// Initialise the main render target for scene dump
	m_renderTarget = new RenderTarget2D(m_device, ScreenWidth, ScreenHeight, D3DFORMAT::D3DFMT_A8R8G8B8);
	if (m_renderTarget->GetTexture() == NULL)
		return false;

	// Initialise the shadow map
	m_shadowMap = new RenderTarget2D(m_device, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, D3DFORMAT::D3DFMT_R32F);
	if (m_shadowMap->GetTexture() == NULL)
		return false;

	m_shadowMapCube = new RenderTargetCube(m_device, SHADOW_MAP_SIZE, D3DFORMAT::D3DFMT_R32F);
	if (m_shadowMap->GetTexture() == NULL)
		return false;
	   
	// Initialise stuff for the new light pre-pass renderer
	m_depthBuffer = new RenderTarget2D(m_device, ScreenWidth, ScreenHeight, D3DFORMAT::D3DFMT_R32F);
	m_normalBuffer = new RenderTarget2D(m_device, ScreenWidth, ScreenHeight, D3DFORMAT::D3DFMT_A8R8G8B8);
	m_colorBuffer = new RenderTarget2D(m_device, ScreenWidth, ScreenHeight, D3DFORMAT::D3DFMT_A8R8G8B8);
	m_outputBuffer = new RenderTarget2D(m_device, ScreenWidth, ScreenHeight, D3DFORMAT::D3DFMT_A8R8G8B8);
	m_lightBuffer = new RenderTarget2D(m_device, ScreenWidth, ScreenHeight, D3DFORMAT::D3DFMT_A8R8G8B8);
	m_vertexLightBuffer = new RenderTarget2D(m_device, ScreenWidth, ScreenHeight, D3DFORMAT::D3DFMT_A8R8G8B8);
	m_shadowBuffer = new RenderTarget2D(m_device, ScreenWidth, ScreenHeight, D3DFORMAT::D3DFMT_A8R8G8B8);
	m_postprocessBuffer = new RenderTarget2D(m_device, ScreenWidth, ScreenHeight, D3DFORMAT::D3DFMT_A8R8G8B8);

	m_shaderClearGBuffer = new Shader(m_device, (char*)"ClearGBuffer.fx");
	m_shaderClearGBuffer->Compile();
	if (m_shaderClearGBuffer->GetEffect() == NULL)
		return false;

	m_shaderFillGBuffer = new Shader(m_device, (char*)"FillGBuffer.fx");
	m_shaderFillGBuffer->Compile();
	if (m_shaderFillGBuffer->GetEffect() == NULL)
		return false;

	m_shaderLight = new Shader(m_device, (char*)"Light.fx");
	m_shaderLight->Compile();
	if (m_shaderLight->GetEffect() == NULL)
		return false;

	m_shaderCombine = new Shader(m_device, (char*)"CombineFinal.fx");
 	m_shaderCombine->Compile();
	if (m_shaderCombine->GetEffect() == NULL)
		return false;	

	m_shaderBasic = new Shader(m_device, (char*)"Basic.fx");
	m_shaderBasic->Compile();
	if (m_shaderBasic->GetEffect() == NULL)
		return false;

	m_shaderRain = new Shader(m_device, (char*)"Rain.fx");
	m_shaderRain->Compile();
	if (m_shaderRain->GetEffect() == NULL)
		return false;

	/*m_shaderFXAA = new Shader(m_device, (char*)"FXAA.fx");
	m_shaderFXAA->Compile();
	if (m_shaderFXAA->GetEffect() == NULL)
		return false;*/

	m_shaderReconstructZBuffer = new Shader(m_device, (char*)"ReconstructZBuffer.fx");
	m_shaderReconstructZBuffer->Compile();
	if (m_shaderReconstructZBuffer->GetEffect() == NULL)
		return false;

	m_shaderSprites = new Shader(m_device, (char*)"Sprites.fx");
	m_shaderSprites->Compile();
	if (m_shaderSprites->GetEffect() == NULL)
		return false;

	m_sphereMesh = new RendererSphere(m_device, 1280.0f, 6);
	m_quad = new RendererQuad(m_device, 1024.0f);
	m_skyQuad = new RendererQuad(m_device, 9728.0f);

	m_halfPixelX = 0.5f / (float)ScreenWidth;
	m_halfPixelY = 0.5f / (float)ScreenHeight;
	 
	D3DXCreateTextureFromFileEx(m_device, "SSAO.png", D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0,
		D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
		D3DCOLOR_XRGB(255, 255, 255), NULL, NULL, &m_randomTexture);

	printf("DX initialised\n");

	// Initialise last non-DX stuff
	m_fadeTimer = 0;
	m_pickupRotation = 0;
	//m_itemsObjectMatrices = NULL;
	m_dynamicLights.reserve(1024);
	m_lights.reserve(1024);
	m_itemsToDraw.reserve(1024);
	m_firstWeather = true;

	ResetBlink();

	return true;
}

void Renderer::ResetBlink()
{
	m_blinkColorDirection = -1;
	m_blinkColorValue = 255;
}

void Renderer::PrintString(__int32 x, __int32 y, char* string, D3DCOLOR color, __int32 flags)
{
	__int32 realX = x;
	__int32 realY = y;
	float factor = ScreenWidth / 800.0f;

	RECT rect = { 0, 0, 0, 0 };

	m_gameFont->DrawTextA(NULL, string, strlen(string), &rect, DT_CALCRECT, color);
	
	if (flags & PRINTSTRING_CENTER)
	{
		__int32 width = rect.right - rect.left;
		rect.left = x - width / 2;
		rect.right = x + width / 2;
		rect.top += y;
		rect.bottom += y;
	}
	else
	{
		rect.left = x;
		rect.right += x;
		rect.top = y;
		rect.bottom += y;
	}

	rect.left *= factor;
	rect.right *= factor;
	rect.top *= factor;
	rect.bottom *= factor;

	if (flags & PRINTSTRING_BLINK)
	{
		color = D3DCOLOR_ARGB(255, m_blinkColorValue, m_blinkColorValue, m_blinkColorValue);

		if (!(flags & PRINTSTRING_DONT_UPDATE_BLINK))
		{
			m_blinkColorValue += m_blinkColorDirection * 16;
			if (m_blinkColorValue < 0)
			{
				m_blinkColorValue = 0;
				m_blinkColorDirection = 1;
			}
			if (m_blinkColorValue > 255)
			{
				m_blinkColorValue = 255;
				m_blinkColorDirection = -1;
			}
		}
	}

	if (flags & PRINTSTRING_OUTLINE)
	{
		__int16 outline = 1;
		rect.left += outline;
		rect.top += outline;
		rect.right += outline;
		rect.bottom += outline;

		m_gameFont->DrawTextA(NULL, string, -1, &rect, 0, D3DCOLOR_ARGB(255, 0, 0, 0));

		rect.left -= outline;
		rect.top -= outline;
		rect.right -= outline;
		rect.bottom -= outline;
	}

	m_gameFont->DrawTextA(NULL, string, -1, &rect, 0, color);
}

bool Renderer::PrintDebugMessage(__int32 x, __int32 y, __int32 alpha, byte r, byte g, byte b, LPCSTR Message)
{
	D3DCOLOR fontColor = D3DCOLOR_ARGB(alpha, r, g, b);
	RECT rct;
	rct.left = x;
	rct.right = 700;
	rct.top = y;
	rct.bottom = rct.top + 16;
	m_debugFont->DrawTextA(NULL, Message, -1, &rct, 0, fontColor);

	return true;
}
 
RendererMesh* Renderer::GetRendererMeshFromTrMesh(RendererObject* obj, __int16* meshPtr, __int16* refMeshPtr, __int16 boneIndex, __int32 isJoints, __int32 isHairs)
{  
	RendererMesh* mesh = new RendererMesh(m_device);

	__int16* basePtr = meshPtr;

	__int16 cx = *meshPtr++;
	__int16 cy = *meshPtr++;
	__int16 cz = *meshPtr++;
	__int16 r1 = *meshPtr++;
	__int16 r2 = *meshPtr++;

	__int16 numVertices = *meshPtr++;

	VECTOR* vertices = (VECTOR*)malloc(sizeof(VECTOR) * numVertices); 
	for (__int32 v = 0; v < numVertices; v++)
	{
		__int16 x = *meshPtr++;
		__int16 y = *meshPtr++;
		__int16 z = *meshPtr++;

		vertices[v].vx = x;
		vertices[v].vy = y;
		vertices[v].vz = z;

		mesh->Positions.push_back(D3DXVECTOR3(x, y, z));
	}

	__int16 numNormals = *meshPtr++;
	VECTOR* normals = NULL;
	if (numNormals > 0)
	{
		normals = (VECTOR*)malloc(sizeof(VECTOR) * numNormals);
		for (__int32 v = 0; v < numNormals; v++)
		{
			__int16 x = *meshPtr++;
			__int16 y = *meshPtr++;
			__int16 z = *meshPtr++;

			normals[v].vx = x;
			normals[v].vy = y;
			normals[v].vz = z;
		}
	}
	else
		meshPtr += (-numNormals);

	__int16 numRectangles = *meshPtr++;

	for (__int32 r = 0; r < numRectangles; r++)
	{
		__int16 v1 = *meshPtr++;
		__int16 v2 = *meshPtr++;
		__int16 v3 = *meshPtr++;
		__int16 v4 = *meshPtr++;
		__int16 textureId = *meshPtr++;
		__int16 effects = *meshPtr++;

		__int16 indices[4] = { v1,v2,v3,v4 };

		__int16 textureIndex = textureId & 0x7FFF;
		bool doubleSided = (textureId & 0x8000) >> 15;

		// Get the object texture
		OBJECT_TEXTURE* texture = &ObjectTextures[textureIndex];
		__int32 tile = texture->tileAndFlag & 0x7FFF;
			
		// Create vertices
		RendererBucket* bucket;
		__int32 bucketIndex = RENDERER_BUCKET_SOLID;
		if (!doubleSided)
		{
			if (texture->attribute == 2 || (effects & 1))
				bucketIndex = RENDERER_BUCKET_TRANSPARENT;
			else if (texture->attribute == 0)
				bucketIndex = RENDERER_BUCKET_SOLID;
			else
				bucketIndex = RENDERER_BUCKET_ALPHA_TEST;
		}
		else
		{
			if (texture->attribute == 2 || (effects & 1))
				bucketIndex = RENDERER_BUCKET_TRANSPARENT_DS;
			else if (texture->attribute == 0)
				bucketIndex = RENDERER_BUCKET_SOLID_DS;
			else
				bucketIndex = RENDERER_BUCKET_ALPHA_TEST_DS;
		}
		bucket = mesh->GetBucket(bucketIndex);
		obj->HasDataInBucket[bucketIndex] = true;

		__int32 baseVertices = bucket->NumVertices;
		for (__int32 v = 0; v < 4; v++)
		{
			RendererVertex vertex;

			vertex.x = vertices[indices[v]].vx;
			vertex.y = vertices[indices[v]].vy;
			vertex.z = vertices[indices[v]].vz;

			if (numNormals > 0)
			{
				vertex.nx = normals[indices[v]].vx;
				vertex.ny = normals[indices[v]].vy;
				vertex.nz = normals[indices[v]].vz;
			}

			vertex.u = (texture->vertices[v].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
			vertex.v = (texture->vertices[v].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;

			vertex.bone = boneIndex;
			if (isJoints && boneIndex != 0 && m_skinJointRemap[boneIndex][indices[v]] != -1)
				vertex.bone = m_skinJointRemap[boneIndex][indices[v]];
			if (isHairs)
				vertex.bone = indices[v];
			/*if (isHairs && boneIndex == 0 && indices[v] < 4)
				vertex.bone = 6;
			if (isHairs && boneIndex != 0 && indices[v] < 4)
				vertex.bone = boneIndex - 1;*/

			bucket->NumVertices++;
			bucket->Vertices.push_back(vertex);
		}

		bucket->Indices.push_back(baseVertices);
		bucket->Indices.push_back(baseVertices + 1);
		bucket->Indices.push_back(baseVertices + 3);
		bucket->Indices.push_back(baseVertices + 2);
		bucket->Indices.push_back(baseVertices + 3);
		bucket->Indices.push_back(baseVertices + 1);
		bucket->NumIndices += 6;

		RendererPolygon newPolygon;
		newPolygon.Shape = SHAPE_RECTANGLE;
		newPolygon.Indices[0] = baseVertices;
		newPolygon.Indices[1] = baseVertices + 1;
		newPolygon.Indices[2] = baseVertices + 2;
		newPolygon.Indices[3] = baseVertices + 3;
		bucket->Polygons.push_back(newPolygon);
	}

	__int16 numTriangles = *meshPtr++;

	for (__int32 r = 0; r < numTriangles; r++)
	{
		__int16 v1 = *meshPtr++;
		__int16 v2 = *meshPtr++;
		__int16 v3 = *meshPtr++;
		__int16 textureId = *meshPtr++;
		__int16 effects = *meshPtr++;

		__int16 indices[3] = { v1,v2,v3 };

		__int16 textureIndex = textureId & 0x7FFF;
		bool doubleSided = (textureId & 0x8000) >> 15;

		// Get the object texture
		OBJECT_TEXTURE* texture = &ObjectTextures[textureIndex];
		__int32 tile = texture->tileAndFlag & 0x7FFF;

		// Create vertices
		RendererBucket* bucket;
		__int32 bucketIndex = RENDERER_BUCKET_SOLID;
		if (!doubleSided)
		{
			if (texture->attribute == 2 || (effects & 1))
				bucketIndex = RENDERER_BUCKET_TRANSPARENT;
			else if (texture->attribute == 0)
				bucketIndex = RENDERER_BUCKET_SOLID;
			else
				bucketIndex = RENDERER_BUCKET_ALPHA_TEST;
		}
		else
		{
			if (texture->attribute == 2 || (effects & 1))
				bucketIndex = RENDERER_BUCKET_TRANSPARENT_DS;
			else if (texture->attribute == 0)
				bucketIndex = RENDERER_BUCKET_SOLID_DS;
			else
				bucketIndex = RENDERER_BUCKET_ALPHA_TEST_DS;
		}
		bucket = mesh->GetBucket(bucketIndex);
		obj->HasDataInBucket[bucketIndex] = true;

		__int32 baseVertices = bucket->NumVertices;
		for (__int32 v = 0; v < 3; v++)
		{
			RendererVertex vertex;

			vertex.x = vertices[indices[v]].vx;
			vertex.y = vertices[indices[v]].vy;
			vertex.z = vertices[indices[v]].vz;

			if (numNormals > 0)
			{
				vertex.nx = normals[indices[v]].vx;
				vertex.ny = normals[indices[v]].vy;
				vertex.nz = normals[indices[v]].vz;
			}

			vertex.u = (texture->vertices[v].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
			vertex.v = (texture->vertices[v].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;
		
			vertex.bone = boneIndex;
			if (isJoints && boneIndex != 0 && m_skinJointRemap[boneIndex][indices[v]] != -1)
				vertex.bone = m_skinJointRemap[boneIndex][indices[v]];
			if (isHairs)
				vertex.bone = indices[v];
			/*if (isHairs && boneIndex == 0 && indices[v] < 4)
				vertex.bone = 6;
			if (isHairs && boneIndex != 0 && indices[v] < 4)
				vertex.bone = boneIndex - 1;*/

			bucket->NumVertices++;
			bucket->Vertices.push_back(vertex);
		}

		bucket->Indices.push_back(baseVertices);
		bucket->Indices.push_back(baseVertices + 1);
		bucket->Indices.push_back(baseVertices + 2);
		bucket->NumIndices += 3;

		RendererPolygon newPolygon;
		newPolygon.Shape = SHAPE_TRIANGLE;
		newPolygon.Indices[0] = baseVertices;
		newPolygon.Indices[1] = baseVertices + 1;
		newPolygon.Indices[2] = baseVertices + 2;
		bucket->Polygons.push_back(newPolygon);
	}

	free(vertices);
	if (normals != NULL) free(normals);

	if (MeshPointersToMesh.find(refMeshPtr) == MeshPointersToMesh.end())
		MeshPointersToMesh.insert(pair<__int16*, RendererMesh*>(refMeshPtr, mesh));

	return mesh;
}

bool Renderer::PrepareDataForTheRenderer()
{
	// Step 0: free all previus resources
	for (map<__int32, RendererRoom*>::iterator it = m_rooms.begin(); it != m_rooms.end(); ++it)
		delete (it->second);
	m_rooms.clear();

	for (map<__int32, RendererObject*>::iterator it = m_moveableObjects.begin(); it != m_moveableObjects.end(); ++it)
		delete (it->second);
	m_moveableObjects.clear();

	for (map<__int32, RendererObject*>::iterator it = m_staticObjects.begin(); it != m_staticObjects.end(); ++it)
		delete (it->second);
	m_staticObjects.clear();

	// Step 1: create the texture atlas
	byte* buffer = (byte*)malloc(TEXTURE_ATLAS_SIZE * TEXTURE_ATLAS_SIZE * 4);
	__int32 blockX = 0;
	__int32 blockY = 0;

	ZeroMemory(buffer, TEXTURE_ATLAS_SIZE * TEXTURE_ATLAS_SIZE * 4);

	for (int p = 0; p < NumTexturePages; p++)
	{
		for (int y = 0; y < 256; y++)
		{
			for (int x = 0; x < 256; x++)
			{
				__int32 pixelIndex = blockY * TEXTURE_PAGE_SIZE * NUM_TEXTURE_PAGES_PER_ROW + y * 256 * NUM_TEXTURE_PAGES_PER_ROW * 4 + blockX * 256 * 4 + x * 4;
				__int32 oldPixelIndex = p * TEXTURE_PAGE_SIZE + y * 256 * 4 + x * 4;

				byte r = Texture32[oldPixelIndex];
				byte g = Texture32[oldPixelIndex + 1];
				byte b = Texture32[oldPixelIndex + 2];
				byte a = Texture32[oldPixelIndex + 3];

				//if (r == 0 && g == 0 && b == 0)
				//	a = 0;

				buffer[pixelIndex] = r;
				buffer[pixelIndex + 1] = g;
				buffer[pixelIndex + 2] = b;
				buffer[pixelIndex + 3] = a;
			}
		}

		blockX++;
		if (blockX == NUM_TEXTURE_PAGES_PER_ROW)
		{
			blockX = 0;
			blockY++;
		}
	}

	// Release the eventually old texture
	if (m_textureAtlas != NULL)
		m_textureAtlas->Release();

	// Create the new texture atlas
	D3DXCreateTexture(m_device, TEXTURE_ATLAS_SIZE, TEXTURE_ATLAS_SIZE, 0, D3DX_DEFAULT,
					  D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_textureAtlas);

	// Lock the texture pixels and copy them from the buffer
	D3DLOCKED_RECT rect;
	m_textureAtlas->LockRect(0, &rect, NULL, NULL);
	unsigned char* dest = static_cast<unsigned char*>(rect.pBits);
	memcpy(dest, buffer, TEXTURE_ATLAS_SIZE * TEXTURE_ATLAS_SIZE * 4);
	m_textureAtlas->UnlockRect(0);

	D3DXSaveTextureToFile("E:\\Atlas.png", D3DXIMAGE_FILEFORMAT::D3DXIFF_PNG, m_textureAtlas, NULL);

	// Release the temp buffer
	free(buffer);

	// Release the eventually old texture
	if (m_fontAndMiscTexture != NULL)
		m_fontAndMiscTexture->Release();

	if (m_skyTexture != NULL)
		m_skyTexture->Release();

	// Create the new texture atlas
	D3DXCreateTexture(m_device, 256, 768, 0, D3DX_DEFAULT, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_fontAndMiscTexture);

	// Lock the texture pixels and copy them from the buffer
	m_fontAndMiscTexture->LockRect(0, &rect, NULL, NULL);
	dest = static_cast<unsigned char*>(rect.pBits);
	memcpy(dest, MiscTextures, 256 * 768 * 4);
	m_fontAndMiscTexture->UnlockRect(0);
	//D3DXSaveTextureToFile("E:\\Misc.png", D3DXIMAGE_FILEFORMAT::D3DXIFF_PNG, m_fontAndMiscTexture, NULL);

	D3DXCreateTexture(m_device, 256, 256, 0, D3DX_DEFAULT, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_skyTexture);
	
	m_skyTexture->LockRect(0, &rect, NULL, NULL);
	dest = static_cast<unsigned char*>(rect.pBits);
	memcpy(dest, MiscTextures + 256 * 512 * 4, 256 * 256 * 4);
	m_fontAndMiscTexture->UnlockRect(0);

	// Step 2: prepare rooms
	for (__int32 i = 0; i < NumberRooms; i++)
	{
		ROOM_INFO* room = &Rooms[i];
		RendererObject* roomObject = new RendererObject(m_device, -1, 1);
		RendererMesh* mesh = new RendererMesh(m_device);
		RendererRoom* r = new RendererRoom();
		r->Room = room;
		r->RoomObject = roomObject;
		r->AmbientLight = D3DXVECTOR4(room->ambient.r / 255.0f, room->ambient.g / 255.0f, room->ambient.b / 255.0f, 1.0f);

		m_rooms.insert(pair<__int32, RendererRoom*>(i, r));

		if (room->NumVertices == 0)
			continue;

		__int32 lastRectangle = 0;
		__int32 lastTriangle = 0;

		tr5_room_layer* layers = (tr5_room_layer*)room->LayerOffset;

		for (__int32 l = 0; l < room->NumLayers; l++)
		{
			tr5_room_layer* layer = &layers[l];
			if (layer->NumLayerVertices == 0)
				continue;

			byte* polygons = (byte*)layer->PolyOffset;
			tr5_room_vertex* vertices = (tr5_room_vertex*)layer->VerticesOffset;

			if (layer->NumLayerRectangles > 0)
			{
				for (int r = 0; r < layer->NumLayerRectangles; r++)
				{
					tr4_mesh_face4* poly = (tr4_mesh_face4*)polygons;

					// Get the real texture index and if double sided
					__int16 textureIndex = poly->Texture & 0x7FFF;
					bool doubleSided = (poly->Texture & 0x8000) >> 15;

					// Get the object texture
					OBJECT_TEXTURE* texture = &ObjectTextures[textureIndex];
					__int32 tile = texture->tileAndFlag & 0x7FFF;

					// Create vertices
					RendererBucket* bucket;
					__int32 bucketIndex = RENDERER_BUCKET_SOLID;
					if (!doubleSided)
					{
						if (texture->attribute == 2)
							bucketIndex = RENDERER_BUCKET_TRANSPARENT;
						else if (texture->attribute == 0)
							bucketIndex = RENDERER_BUCKET_SOLID;
						else
							bucketIndex = RENDERER_BUCKET_ALPHA_TEST;
					}
					else
					{
						if (texture->attribute == 2)
							bucketIndex = RENDERER_BUCKET_TRANSPARENT_DS;
						else if (texture->attribute == 0)
							bucketIndex = RENDERER_BUCKET_SOLID_DS;
						else
							bucketIndex = RENDERER_BUCKET_ALPHA_TEST_DS;
					}
					bucket = mesh->GetBucket(bucketIndex);
					roomObject->HasDataInBucket[bucketIndex] = true;

					// Calculate face normal
					D3DXVECTOR3 p0 = D3DXVECTOR3(vertices[poly->Vertices[0]].Vertex.x,
						vertices[poly->Vertices[0]].Vertex.y,
						vertices[poly->Vertices[0]].Vertex.z);
					D3DXVECTOR3 p1 = D3DXVECTOR3(vertices[poly->Vertices[1]].Vertex.x,
						vertices[poly->Vertices[1]].Vertex.y,
						vertices[poly->Vertices[1]].Vertex.z);
					D3DXVECTOR3 p2 = D3DXVECTOR3(vertices[poly->Vertices[2]].Vertex.x,
						vertices[poly->Vertices[2]].Vertex.y,
						vertices[poly->Vertices[2]].Vertex.z);
					D3DXVECTOR3 e1 = p1 - p0;
					D3DXVECTOR3 e2 = p1 - p2;
					D3DXVECTOR3 normal;
					D3DXVec3Cross(&normal, &e1, &e2);
					D3DXVec3Normalize(&normal, &normal);

					__int32 baseVertices = bucket->NumVertices;
					for (__int32 v = 0; v < 4; v++)
					{
						RendererVertex vertex; // = new RendererVertex();

						vertex.x = vertices[poly->Vertices[v]].Vertex.x;
						vertex.y = vertices[poly->Vertices[v]].Vertex.y;
						vertex.z = vertices[poly->Vertices[v]].Vertex.z;

						vertex.nx = normal.x; // vertices[poly->Vertices[v]].Normal.x;
						vertex.ny = normal.y; // vertices[poly->Vertices[v]].Normal.y;
						vertex.nz = normal.z; // vertices[poly->Vertices[v]].Normal.z;

						vertex.u = (texture->vertices[v].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
						vertex.v = (texture->vertices[v].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;

						vertex.r = ((vertices[poly->Vertices[v]].Colour >> 16) & 0xFF) / 255.0f;
						vertex.g = ((vertices[poly->Vertices[v]].Colour >> 8) & 0xFF) / 255.0f;
						vertex.b = ((vertices[poly->Vertices[v]].Colour >> 0) & 0xFF) / 255.0f;
						vertex.a = 1.0f;

						bucket->NumVertices++;
						bucket->Vertices.push_back(vertex);
						roomObject->NumVertices++;
					}

					bucket->Indices.push_back(baseVertices);
					bucket->Indices.push_back(baseVertices + 1);
					bucket->Indices.push_back(baseVertices + 3);
					bucket->Indices.push_back(baseVertices + 2);
					bucket->Indices.push_back(baseVertices + 3);
					bucket->Indices.push_back(baseVertices + 1);
					bucket->NumIndices += 6;

					RendererPolygon newPolygon;
					newPolygon.Shape = SHAPE_RECTANGLE;
					newPolygon.Indices[0] = baseVertices;
					newPolygon.Indices[1] = baseVertices + 1;
					newPolygon.Indices[2] = baseVertices + 2;
					newPolygon.Indices[3] = baseVertices + 3;
					bucket->Polygons.push_back(newPolygon);

					polygons += sizeof(tr4_mesh_face4);
				}
			}

			if (layer->NumLayerTriangles > 0)
			{
				for (int r = 0; r < layer->NumLayerTriangles; r++)
				{
					tr4_mesh_face3* poly = (tr4_mesh_face3*)polygons;

					// Get the real texture index and if double sided
					__int16 textureIndex = poly->Texture & 0x7FFF;
					bool doubleSided = (poly->Texture & 0x8000) >> 15;

					// Get the object texture
					OBJECT_TEXTURE* texture = &ObjectTextures[textureIndex];
					__int32 tile = texture->tileAndFlag & 0x7FFF;

					// Create vertices
					RendererBucket* bucket;
					__int32 bucketIndex = RENDERER_BUCKET_SOLID;
					if (!doubleSided)
					{
						if (texture->attribute == 2)
							bucketIndex = RENDERER_BUCKET_TRANSPARENT;
						else if (texture->attribute == 0)
							bucketIndex = RENDERER_BUCKET_SOLID;
						else
							bucketIndex = RENDERER_BUCKET_ALPHA_TEST;
					}
					else
					{
						if (texture->attribute == 2)
							bucketIndex = RENDERER_BUCKET_TRANSPARENT_DS;
						else if (texture->attribute == 0)
							bucketIndex = RENDERER_BUCKET_SOLID_DS;
						else
							bucketIndex = RENDERER_BUCKET_ALPHA_TEST_DS;
					}
					bucket = mesh->GetBucket(bucketIndex);
					roomObject->HasDataInBucket[bucketIndex] = true;

					// Calculate face normal
					D3DXVECTOR3 p0 = D3DXVECTOR3(vertices[poly->Vertices[0]].Vertex.x,
						vertices[poly->Vertices[0]].Vertex.y,
						vertices[poly->Vertices[0]].Vertex.z);
					D3DXVECTOR3 p1 = D3DXVECTOR3(vertices[poly->Vertices[1]].Vertex.x,
						vertices[poly->Vertices[1]].Vertex.y,
						vertices[poly->Vertices[1]].Vertex.z);
					D3DXVECTOR3 p2 = D3DXVECTOR3(vertices[poly->Vertices[2]].Vertex.x,
						vertices[poly->Vertices[2]].Vertex.y,
						vertices[poly->Vertices[2]].Vertex.z);
					D3DXVECTOR3 e1 = p1 - p0;
					D3DXVECTOR3 e2 = p1 - p2;
					D3DXVECTOR3 normal;
					D3DXVec3Cross(&normal, &e1, &e2);
					D3DXVec3Normalize(&normal, &normal);

					__int32 baseVertices = bucket->NumVertices;
					for (__int32 v = 0; v < 3; v++)
					{
						RendererVertex vertex; // = new RendererVertex();

						vertex.x = vertices[poly->Vertices[v]].Vertex.x;
						vertex.y = vertices[poly->Vertices[v]].Vertex.y;
						vertex.z = vertices[poly->Vertices[v]].Vertex.z;

						vertex.nx = normal.x; // vertices[poly->Vertices[v]].Normal.x;
						vertex.ny = normal.y; // vertices[poly->Vertices[v]].Normal.y;
						vertex.nz = normal.z; // vertices[poly->Vertices[v]].Normal.z;

						vertex.u = (texture->vertices[v].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
						vertex.v = (texture->vertices[v].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;

						vertex.r = ((vertices[poly->Vertices[v]].Colour >> 16) & 0xFF) / 255.0f;
						vertex.g = ((vertices[poly->Vertices[v]].Colour >> 8) & 0xFF) / 255.0f;
						vertex.b = ((vertices[poly->Vertices[v]].Colour >> 0) & 0xFF) / 255.0f;
						vertex.a = 1.0f;

						bucket->NumVertices++;
						bucket->Vertices.push_back(vertex);
						roomObject->NumVertices++;
					}

					bucket->Indices.push_back(baseVertices);
					bucket->Indices.push_back(baseVertices + 1);
					bucket->Indices.push_back(baseVertices + 2);
					bucket->NumIndices += 3;

					RendererPolygon newPolygon;
					newPolygon.Shape = SHAPE_TRIANGLE;
					newPolygon.Indices[0] = baseVertices;
					newPolygon.Indices[1] = baseVertices + 1;
					newPolygon.Indices[2] = baseVertices + 2;
					bucket->Polygons.push_back(newPolygon);

					polygons += sizeof(tr4_mesh_face3);
				}
			}

			if (room->numLights != 0)
			{
				tr5_room_light* oldLight = room->light;

				for (__int32 l = 0; l < room->numLights; l++)
				{
					// DEBUG: only point lights

					RendererLight* light = new RendererLight();
					/*if (oldLight->LightType == LIGHT_TYPES::LIGHT_TYPE_SUN)
					{
						light->Color = D3DXVECTOR4(oldLight->r, oldLight->g, oldLight->b, 1.0f);
						light->Direction = D3DXVECTOR4(oldLight->dx, oldLight->dy, oldLight->dz, 1.0f);
						light->Type = LIGHT_TYPES::LIGHT_TYPE_SUN;
					}
					else*/ if (oldLight->LightType == LIGHT_TYPES::LIGHT_TYPE_POINT)
					{
						light->Position = D3DXVECTOR4(oldLight->x, oldLight->y, oldLight->z, 1.0f);
						light->Color = D3DXVECTOR4(oldLight->r, oldLight->g, oldLight->b, 1.0f);
						light->Direction = D3DXVECTOR4(oldLight->dx, oldLight->dy, oldLight->dz, 1.0f);
						light->Intensity = 1.0f;
						light->In = oldLight->In;
						light->Out = oldLight->Out;
						light->Type = LIGHT_TYPES::LIGHT_TYPE_POINT;
					}
					/*else if (oldLight->LightType == LIGHT_TYPES::LIGHT_TYPE_SHADOW)
					{
						light->Position = D3DXVECTOR4(oldLight->x, oldLight->y, oldLight->z, 1.0f);
						light->Color = D3DXVECTOR4(oldLight->r, oldLight->g, oldLight->b, 1.0f);
						light->Direction = D3DXVECTOR4(oldLight->dx, oldLight->dy, oldLight->dz, 1.0f);
						light->In = oldLight->In;
						light->Out = oldLight->Out;
						light->Type = LIGHT_TYPES::LIGHT_TYPE_SHADOW;
					}*/
					else if (oldLight->LightType == LIGHT_TYPES::LIGHT_TYPE_SPOT)
					{
						light->Position = D3DXVECTOR4(oldLight->x, oldLight->y, oldLight->z, 1.0f);
						light->Color = D3DXVECTOR4(oldLight->r, oldLight->g, oldLight->b, 1.0f);
						light->Direction = D3DXVECTOR4(oldLight->dx, oldLight->dy, oldLight->dz, 1.0f);
						light->Intensity = 1.0f;
						light->In = oldLight->In;
						light->Out = oldLight->Range;  //oldLight->Out;
						light->Range = oldLight->Range;
						light->Type = LIGHT_TYPES::LIGHT_TYPE_POINT; // LIGHT_TYPES::LIGHT_TYPE_SPOT;
					}

					r->Lights.push_back(light);
				}
			}
		}

		for (__int32 i = 0; i < NUM_BUCKETS; i++)
			mesh->GetBucket((RENDERER_BUCKETS)i)->UpdateBuffers();

		roomObject->ObjectMeshes.push_back(mesh);
	}

	m_numHairVertices = 0;
	m_numHairIndices = 0;

	// Step 3: prepare moveables
	for (__int32 i = 0; i < MoveablesIds.size(); i++)
	{
		__int32 objNum = MoveablesIds[i];
		OBJECT_INFO* obj = &Objects[objNum];
		
		if (obj->nmeshes > 0)
		{
			RendererObject* moveable = new RendererObject(m_device, MoveablesIds[i], obj->nmeshes);

			// Assign the draw routine
			if (objNum == ID_FLAME || objNum == ID_FLAME_EMITTER || objNum == ID_FLAME_EMITTER2 || objNum == ID_FLAME_EMITTER3 ||
				objNum == ID_TRIGGER_TRIGGERER || objNum == ID_TIGHT_ROPE || objNum == ID_AI_AMBUSH ||
				objNum == ID_AI_FOLLOW || objNum == ID_AI_GUARD || objNum == ID_AI_MODIFY ||
				objNum == ID_AI_PATROL1 || objNum == ID_AI_PATROL2 || objNum == ID_AI_X1 ||
				objNum == ID_AI_X2 || objNum == ID_DART_EMITTER || objNum == ID_HOMING_DART_EMITTER ||
				objNum == ID_ROPE || objNum == ID_KILL_ALL_TRIGGERS)
			{
				moveable->DoNotDraw = true;
			}
			else
			{
				moveable->DoNotDraw = false;
			}

			for (__int32 j = 0; j < obj->nmeshes; j++)
			{
				__int16* meshPtr = &RawMeshData[RawMeshPointers[obj->meshIndex / 2 + j] / 2];
				RendererMesh* mesh = GetRendererMeshFromTrMesh(moveable,
															   meshPtr, 
															   Meshes[obj->meshIndex + 2 * j],
															   j, MoveablesIds[i] == ID_LARA_SKIN_JOINTS,
															   MoveablesIds[i] == ID_HAIR);
				moveable->ObjectMeshes.push_back(mesh);
			}

			__int32* bone = &MeshTrees[obj->boneIndex];

			stack<RendererBone*> stack;

			for (int j = 0; j < obj->nmeshes; j++)
				moveable->LinearizedBones.push_back(new RendererBone(j));

			RendererBone* currentBone = moveable->LinearizedBones[0];
			RendererBone* stackBone = moveable->LinearizedBones[0];

			for (int mi = 0; mi < obj->nmeshes - 1; mi++)
			{
				int j = mi + 1;

				__int32 opcode = *(bone++);
				int linkX = *(bone++);
				int linkY = *(bone++);
				int linkZ = *(bone++);

				switch (opcode)
				{
				case 0:
					moveable->LinearizedBones[j]->Parent = currentBone;
					moveable->LinearizedBones[j]->Translation = D3DXVECTOR3(linkX, linkY, linkZ);
					currentBone->Children.push_back(moveable->LinearizedBones[j]);
					currentBone = moveable->LinearizedBones[j];

					break;
				case 1:
					if (stack.empty())
						continue;
					currentBone = stack.top();
					stack.pop();

					moveable->LinearizedBones[j]->Parent = currentBone;
					moveable->LinearizedBones[j]->Translation = D3DXVECTOR3(linkX, linkY, linkZ);
					currentBone->Children.push_back(moveable->LinearizedBones[j]);
					currentBone = moveable->LinearizedBones[j];

					break;
				case 2:
					stack.push(currentBone);

					moveable->LinearizedBones[j]->Translation = D3DXVECTOR3(linkX, linkY, linkZ);
					moveable->LinearizedBones[j]->Parent = currentBone;
					currentBone->Children.push_back(moveable->LinearizedBones[j]);
					currentBone = moveable->LinearizedBones[j];

					break;
				case 3:
					if (stack.empty())
						continue;
					RendererBone* theBone = stack.top();
					stack.pop();

					moveable->LinearizedBones[j]->Translation = D3DXVECTOR3(linkX, linkY, linkZ);
					moveable->LinearizedBones[j]->Parent = theBone;
					theBone->Children.push_back(moveable->LinearizedBones[j]);
					currentBone = moveable->LinearizedBones[j];
					stack.push(theBone);

					break;
				}
			}

			for (int n = 0; n < obj->nmeshes; n++)
				D3DXMatrixTranslation(&moveable->LinearizedBones[n]->Transform,
					moveable->LinearizedBones[n]->Translation.x,
					moveable->LinearizedBones[n]->Translation.y,
					moveable->LinearizedBones[n]->Translation.z);

			moveable->Skeleton = moveable->LinearizedBones[0];
			BuildHierarchy(moveable);

			// Fix Lara skin joints and hairs
			if (MoveablesIds[i] == ID_LARA_SKIN_JOINTS)
			{
				RendererObject* objSkin = m_moveableObjects[ID_LARA_SKIN];

				for (__int32 j = 1; j < obj->nmeshes; j++)
				{
					RendererMesh* jointMesh = moveable->ObjectMeshes[j];
					RendererBone* jointBone = moveable->LinearizedBones[j];

					for (__int32 b1 = 0; b1 < NUM_BUCKETS; b1++)
					{
						RendererBucket* jointBucket = jointMesh->GetBucket((RENDERER_BUCKETS)b1);
						for (__int32 v1 = 0; v1 < jointBucket->Vertices.size(); v1++)
						{
							RendererVertex* jointVertex = &jointBucket->Vertices[v1];
							if (jointVertex->bone != j)
							{
								RendererMesh* skinMesh = objSkin->ObjectMeshes[jointVertex->bone];
								RendererBone* skinBone = objSkin->LinearizedBones[jointVertex->bone];

								for (__int32 b2 = 0; b2 < NUM_BUCKETS; b2++)
								{
									RendererBucket* skinBucket = skinMesh->GetBucket((RENDERER_BUCKETS)b2);
									for (__int32 v2 = 0; v2 < skinBucket->Vertices.size(); v2++)
									{
										RendererVertex* skinVertex = &skinBucket->Vertices[v2];

										__int32 x1 = jointBucket->Vertices[v1].x + jointBone->GlobalTranslation.x;
										__int32 y1 = jointBucket->Vertices[v1].y + jointBone->GlobalTranslation.y;
										__int32 z1 = jointBucket->Vertices[v1].z + jointBone->GlobalTranslation.z;

										__int32 x2 = skinBucket->Vertices[v2].x + skinBone->GlobalTranslation.x;
										__int32 y2 = skinBucket->Vertices[v2].y + skinBone->GlobalTranslation.y;
										__int32 z2 = skinBucket->Vertices[v2].z + skinBone->GlobalTranslation.z;

										if (abs(x1 - x2) < 2 && abs(y1 - y2) < 2 && abs(z1 - z2) < 2)
										{
											jointVertex->x = skinVertex->x;
											jointVertex->y = skinVertex->y;
											jointVertex->z = skinVertex->z;
										}
									}
								}
							}
						}
					}
				}

				// Rebuild the LARA_SKIN object
				for (__int32 j = 0; j < objSkin->ObjectMeshes.size(); j++)
				{
					RendererMesh* mesh = objSkin->ObjectMeshes[j];
					for (__int32 n = 0; n < NUM_BUCKETS; n++)
						mesh->GetBucket((RENDERER_BUCKETS)n)->UpdateBuffers();
				}
			}

			if (MoveablesIds[i] == ID_HAIR)
			{
				for (__int32 j = 0; j < moveable->ObjectMeshes.size(); j++)
				{
					RendererMesh* mesh = moveable->ObjectMeshes[j];
					for (__int32 n = 0; n < NUM_BUCKETS; n++)
					{
						m_numHairVertices += mesh->GetBucket((RENDERER_BUCKETS)n)->NumVertices;
						m_numHairIndices += mesh->GetBucket((RENDERER_BUCKETS)n)->NumIndices;
					}
				}

				m_hairVertices = (RendererVertex*)malloc(m_numHairVertices * sizeof(RendererVertex));
				m_hairIndices = (__int32*)malloc(m_numHairIndices * 4);
			}

			// Initialise buffers
			for (__int32 j = 0; j < obj->nmeshes; j++)
			{
				RendererMesh* mesh = moveable->ObjectMeshes[j];
				for (__int32 n = 0; n < NUM_BUCKETS; n++)
					mesh->GetBucket((RENDERER_BUCKETS)n)->UpdateBuffers();
			}

			m_moveableObjects.insert(pair<__int32, RendererObject*>(MoveablesIds[i], moveable));
		}
	}

	// Step 4: prepare static meshes
	for (__int32 i = 0; i < StaticObjectsIds.size(); i++)
	{
		STATIC_INFO* obj = &StaticObjects[StaticObjectsIds[i]];
		RendererObject* staticObject = new RendererObject(m_device, StaticObjectsIds[i], 1);

		__int16* meshPtr = &RawMeshData[RawMeshPointers[obj->meshNumber / 2] / 2];
		RendererMesh* mesh = GetRendererMeshFromTrMesh(staticObject, meshPtr, Meshes[obj->meshNumber], 0, false, false);

		for (__int32 i = 0; i < NUM_BUCKETS; i++)
			mesh->GetBucket((RENDERER_BUCKETS)i)->UpdateBuffers();

		staticObject->ObjectMeshes.push_back(mesh);

		m_staticObjects.insert(pair<__int32, RendererObject*>(StaticObjectsIds[i], staticObject));
	}

	// Step 5: prepare sprites
	for (__int32 i = 0; i < 41; i++)
	{
		SPRITE* oldSprite = &Sprites[i];
		RendererSprite* sprite = new RendererSprite();

		sprite->Width = (oldSprite->right - oldSprite->left)*256.0f;
		sprite->Height = (oldSprite->bottom - oldSprite->top)*256.0f;

		float left = (oldSprite->left * 256.0f /*+ 0.5f*/ + GET_ATLAS_PAGE_X(oldSprite->tile - 1));
		float top = (oldSprite->top * 256.0f /*+ 0.5f*/ + GET_ATLAS_PAGE_Y(oldSprite->tile - 1));
		float right = (oldSprite->right * 256.0f /*+ 0.5f*/ + GET_ATLAS_PAGE_X(oldSprite->tile - 1));
		float bottom = (oldSprite->bottom * 256.0f /*+ 0.5f*/ + GET_ATLAS_PAGE_Y(oldSprite->tile - 1));

		sprite->UV[0] = D3DXVECTOR2(left / (float)TEXTURE_ATLAS_SIZE, top / (float)TEXTURE_ATLAS_SIZE);
		sprite->UV[1] = D3DXVECTOR2(right / (float)TEXTURE_ATLAS_SIZE, top / (float)TEXTURE_ATLAS_SIZE);
		sprite->UV[2] = D3DXVECTOR2(right / (float)TEXTURE_ATLAS_SIZE, bottom / (float)TEXTURE_ATLAS_SIZE);
		sprite->UV[3] = D3DXVECTOR2(left / (float)TEXTURE_ATLAS_SIZE, bottom / (float)TEXTURE_ATLAS_SIZE);

		m_sprites.insert(pair<__int32, RendererSprite*>(i, sprite));
	}

	for (__int32 i = 0; i < MoveablesIds.size(); i++)
	{
		OBJECT_INFO* obj = &Objects[MoveablesIds[i]];

		if (obj->nmeshes < 0)
		{
			__int16 numSprites = abs(obj->nmeshes);
			__int16 baseSprite = obj->meshIndex;

			RendererSpriteSequence* sequence = new RendererSpriteSequence(MoveablesIds[i]);
			
			for (__int32 j = baseSprite; j < baseSprite + numSprites; j++)
			{
				RendererSprite* sprite = m_sprites[j];
				sequence->SpritesList.push_back(sprite);
			}

			m_spriteSequences.insert(pair<__int32, RendererSpriteSequence*>(MoveablesIds[i], sequence));
		}
	}

	return true;
}

void Renderer::BuildHierarchyRecursive(RendererObject* obj, RendererBone* node, RendererBone* parentNode)
{
	D3DXMatrixMultiply(&node->GlobalTransform, &node->Transform, &parentNode->GlobalTransform);
	obj->BindPoseTransforms[node->Index] = node->GlobalTransform;
	obj->Skeleton->GlobalTranslation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	node->GlobalTranslation = node->Translation + parentNode->GlobalTranslation;

	for (int j = 0; j < node->Children.size(); j++)
	{
		BuildHierarchyRecursive(obj, node->Children[j], node);
	}
}

void Renderer::BuildHierarchy(RendererObject* obj)
{
	obj->Skeleton->GlobalTransform = obj->Skeleton->Transform;
	obj->BindPoseTransforms[obj->Skeleton->Index] = obj->Skeleton->GlobalTransform;
	obj->Skeleton->GlobalTranslation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	for (int j = 0; j < obj->Skeleton->Children.size(); j++)
	{
		BuildHierarchyRecursive(obj, obj->Skeleton->Children[j], obj->Skeleton);
	}
}

void Renderer::FromTrAngle(D3DXMATRIX* matrix, __int16* frameptr, __int32 index)
{
	__int16* ptr = &frameptr[0];

	ptr += 9;
	for (int i = 0; i < index; i++)
	{
		ptr += ((*ptr & 0xc000) == 0 ? 2 : 1);
	}

	int rot0 = *ptr++;
	int frameMode = (rot0 & 0xc000);

	int rot1;
	int rotX;
	int rotY;
	int rotZ;

	switch (frameMode)
	{
	case 0:
		rot1 = *ptr++;
		rotX = ((rot0 & 0x3ff0) >> 4);
		rotY = (((rot1 & 0xfc00) >> 10) | ((rot0 & 0xf) << 6) & 0x3ff);
		rotZ = ((rot1) & 0x3ff);

		D3DXMatrixRotationYawPitchRoll(matrix, rotY* (360.0f / 1024.0f) * RADIAN,
			rotX* (360.0f / 1024.0f) * RADIAN,
			rotZ* (360.0f / 1024.0f) * RADIAN);
		break;

	case 0x4000:
		D3DXMatrixRotationX(matrix, (rot0 & 0xfff)* (360.0f / 4096.0f) * RADIAN);
		break;

	case 0x8000:
		D3DXMatrixRotationY(matrix, (rot0 & 0xfff)* (360.0f / 4096.0f) * RADIAN);
		break;

	case 0xc000:
		D3DXMatrixRotationZ(matrix, (rot0 & 0xfff)* (360.0f / 4096.0f) * RADIAN);
		break;
	}
}

void Renderer::BuildAnimationPoseRecursive(RendererObject* obj, __int16** frmptr, D3DXMATRIX* parentTransform, 
										   __int16 frac, __int16 rate, RendererBone* bone, __int32 mask)
{
	bool calculateMatrix = (mask >> bone->Index) & 1;

	D3DXMATRIX rotation;
	if (calculateMatrix)
	{
		FromTrAngle(&rotation, frmptr[0], bone->Index);

		if (frac)
		{
			D3DXMATRIX rotation2;
			FromTrAngle(&rotation2, frmptr[1], bone->Index);

			D3DXQUATERNION q1, q2, q3;

			D3DXQuaternionRotationMatrix(&q1, &rotation);
			D3DXQuaternionRotationMatrix(&q2, &rotation2);

			D3DXQuaternionSlerp(&q3, &q1, &q2, frac / ((float)rate));

			D3DXMatrixRotationQuaternion(&rotation, &q3);
		}

		D3DXMATRIX extraRotation;		
		D3DXMatrixRotationYawPitchRoll(&extraRotation, bone->ExtraRotation.y, bone->ExtraRotation.x, bone->ExtraRotation.z);
	
		D3DXMatrixMultiply(&obj->AnimationTransforms[bone->Index], &extraRotation, &bone->Transform);
		D3DXMatrixMultiply(&obj->AnimationTransforms[bone->Index], &rotation, &obj->AnimationTransforms[bone->Index]);
		D3DXMatrixMultiply(&obj->AnimationTransforms[bone->Index], &obj->AnimationTransforms[bone->Index], parentTransform);
	}

	for (int j = 0; j < bone->Children.size(); j++)
	{
		BuildAnimationPoseRecursive(obj, frmptr, &obj->AnimationTransforms[bone->Index], frac, rate, 
									bone->Children[j], mask);
	}
}

void Renderer::BuildAnimationPose(RendererObject* obj, __int16** frmptr, __int16 frac, __int16 rate, __int32 mask)
{
	D3DXVECTOR3 p = D3DXVECTOR3((int)*(frmptr[0] + 6), (int)*(frmptr[0] + 7), (int)*(frmptr[0] + 8));

	bool calculateMatrix = (mask >> obj->Skeleton->Index) & 1;

	D3DXMATRIX rotation;
	if (calculateMatrix)
	{
		FromTrAngle(&rotation, frmptr[0], obj->Skeleton->Index);

		if (frac)
		{
			D3DXVECTOR3 p2 = D3DXVECTOR3((int)*(frmptr[1] + 6), (int)*(frmptr[1] + 7), (int)*(frmptr[1] + 8));
			D3DXVec3Lerp(&p, &p, &p2, frac / ((float)rate));

			D3DXMATRIX rotation2;
			FromTrAngle(&rotation2, frmptr[1], obj->Skeleton->Index);

			D3DXQUATERNION q1, q2, q3;

			D3DXQuaternionRotationMatrix(&q1, &rotation);
			D3DXQuaternionRotationMatrix(&q2, &rotation2);

			D3DXQuaternionSlerp(&q3, &q1, &q2, frac / ((float)rate));

			D3DXMatrixRotationQuaternion(&rotation, &q3);
		}

		D3DXMATRIX translation;
		D3DXMatrixTranslation(&translation, p.x, p.y, p.z);

		D3DXMATRIX extraRotation;
		D3DXMatrixRotationYawPitchRoll(&extraRotation, obj->Skeleton->ExtraRotation.y, obj->Skeleton->ExtraRotation.x, obj->Skeleton->ExtraRotation.z);

		D3DXMatrixMultiply(&obj->AnimationTransforms[obj->Skeleton->Index], &extraRotation, &translation);
		D3DXMatrixMultiply(&obj->AnimationTransforms[obj->Skeleton->Index], &rotation, &obj->AnimationTransforms[obj->Skeleton->Index]);
	}

	for (int j = 0; j < obj->Skeleton->Children.size(); j++)
	{
		BuildAnimationPoseRecursive(obj, frmptr, &obj->AnimationTransforms[obj->Skeleton->Index], frac, rate, 
									obj->Skeleton->Children[j], mask);
	}
}

void Renderer::GetVisibleRooms(int from, int to, D3DXVECTOR4* viewPort, bool water, int count) 
{
	if (count > 8) {
		return;
	}

	ROOM_INFO* room = &Rooms[to];

	if (!m_rooms[to]->Visited) {
		m_rooms[to]->Visited = true;
		m_roomsToDraw.push_back(to);
	}

	D3DXVECTOR4 clipPort;
	__int16 numDoors = *(room->door);
	if (numDoors)
	{
		__int16* door = room->door + 1;
		for (int i = 0; i < numDoors; i++) {
			__int16 adjoiningRoom = *(door);

			if (from != adjoiningRoom && CheckPortal(to, door, viewPort, &clipPort))
				GetVisibleRooms(to, adjoiningRoom, &clipPort, water, count + 1);

			door += 16;
		}
	}
}

bool Renderer::CheckPortal(__int16 roomIndex, __int16* portal, D3DXVECTOR4* viewPort, D3DXVECTOR4* clipPort) 
{
	ROOM_INFO* room = &Rooms[roomIndex];

	D3DXVECTOR3 n = D3DXVECTOR3(*(portal + 1), *(portal + 2), *(portal + 3));
	D3DXVECTOR3 v = D3DXVECTOR3(Camera.pos.x - (room->x + *(portal + 4)),
		Camera.pos.y - (room->y + *(portal + 5)),
		Camera.pos.z - (room->z + *(portal + 6)));

	if (D3DXVec3Dot(&n, &v) <= 0.0f)
		return false;

	int  zClip = 0;
	D3DXVECTOR4 p[4];

	clipPort->x = FLT_MAX;
	clipPort->y = FLT_MAX;
	clipPort->z = FLT_MIN;
	clipPort->w = FLT_MIN;

	D3DXMATRIX viewProj;
	D3DXMatrixMultiply(&viewProj, &ViewMatrix, &ProjectionMatrix);

	for (int i = 0; i < 4; i++) {

		D3DXVECTOR4 tmp = D3DXVECTOR4(*(portal + 4 + 3 * i) + room->x, *(portal + 4 + 3 * i+1) + room->y,
			*(portal + 4 + 3 * i+2) + room->z, 1.0f);
		D3DXVec4Transform(&p[i], &tmp, &viewProj);

		if (p[i].w > 0.0f) {
			p[i].x *= (1.0f / p[i].w);
			p[i].y *= (1.0f / p[i].w);
			p[i].z *= (1.0f / p[i].w);

			clipPort->x = min(clipPort->x, p[i].x);
			clipPort->y = min(clipPort->y, p[i].y);
			clipPort->z = max(clipPort->z, p[i].x);
			clipPort->w = max(clipPort->w, p[i].y);
		}
		else
			zClip++;
	}

	if (zClip == 4)
		return false;

	if (zClip > 0) {
		for (int i = 0; i < 4; i++) {
			D3DXVECTOR4 a = p[i];
			D3DXVECTOR4 b = p[(i + 1) % 4];

			if ((a.w > 0.0f) ^ (b.w > 0.0f)) {

				if (a.x < 0.0f && b.x < 0.0f)
					clipPort->x = -1.0f;
				else
					if (a.x > 0.0f && b.x > 0.0f)
						clipPort->z = 1.0f;
					else {
						clipPort->x = -1.0f;
						clipPort->z = 1.0f;
					}

					if (a.y < 0.0f && b.y < 0.0f)
						clipPort->y = -1.0f;
					else
						if (a.y > 0.0f && b.y > 0.0f)
							clipPort->w = 1.0f;
						else {
							clipPort->y = -1.0f;
							clipPort->w = 1.0f;
						}

			}
		}
	}

	if (clipPort->x > viewPort->z || clipPort->y > viewPort->w || clipPort->z < viewPort->x || clipPort->w < viewPort->y)
		return false;

	clipPort->x = max(clipPort->x, viewPort->x);
	clipPort->y = max(clipPort->y, viewPort->y);
	clipPort->z = min(clipPort->z, viewPort->z);
	clipPort->w = min(clipPort->w, viewPort->w);

	return true;
}

void Renderer::CollectRooms()
{
	__int16 baseRoomIndex = Camera.pos.roomNumber;

	for (__int32 i = 0; i < NumberRooms; i++)
		if (m_rooms[i] != NULL)
			m_rooms[i]->Visited = false;

	m_roomsToDraw.clear();
	GetVisibleRooms(-1, baseRoomIndex, &D3DXVECTOR4(-1.0f, -1.0f, 1.0f, 1.0f), false, 0);
}

void Renderer::CollectItems()
{
	for (vector<RendererItemToDraw*>::iterator it = m_itemsToDraw.begin(); it != m_itemsToDraw.end(); ++it)
		delete (*it);
	m_itemsToDraw.clear();

	RendererItemToDraw* newItem = new RendererItemToDraw(Lara.itemNumber, LaraItem);
	m_itemsToDraw.push_back(newItem);

	for (__int32 i = 0; i < m_roomsToDraw.size(); i++)
	{
		RendererRoom* room = m_rooms[m_roomsToDraw[i]];
		if (room == NULL)
			continue;

		ROOM_INFO* r = room->Room;
		
		__int16 itemNum = NO_ITEM;
		for (itemNum = r->itemNumber; itemNum != NO_ITEM; itemNum = Items[itemNum].nextItem)
		{
			ITEM_INFO* item = &Items[itemNum];
			if (item->objectNumber == ID_LARA)
				continue;

			newItem = new RendererItemToDraw(itemNum, item);
			m_itemsToDraw.push_back(newItem);
		}
	}
}

void Renderer::CollectLights()
{
	/*if (m_itemsLightInfo == NULL)
		m_itemsLightInfo = (RendererLightInfo*)malloc(NumItems * sizeof(RendererLightInfo));

	// Reset shadow map
	m_litItems.clear();
	m_enableShadows = false;

	for (__int32 i = 0; i < NumItems; i++)
		m_itemsLightInfo[i].Active = false;

	// Take the brightest/nearest light only for drawn items
	for (__int32 i = 0; i < m_itemsToDraw.size(); i++)
	{
		// NB: first item is always Lara
		//__int32 itemIndex = m_itemsToDraw[i];
		RendererItemToDraw* itemToDraw = m_itemsToDraw[i];
		ITEM_INFO* item = itemToDraw->Item;
		RendererRoom* room = m_rooms[item->roomNumber];

		// Search for the brightest light. We do a simple version of the classic calculation done in pixel shader.
		if (room->Lights.size() != 0)
		{
			m_itemsLightInfo[itemIndex].Active = false;

			RendererLight* brightestLight = NULL;
			float brightest = 0.0f;

			for (__int32 j = 0; j < room->Lights.size(); j++)
			{
				RendererLight* light = room->Lights[j];

				D3DXVECTOR4 itemPos = D3DXVECTOR4(item->pos.xPos, item->pos.yPos, item->pos.zPos, 1.0f);
				D3DXVECTOR4 lightVector = itemPos - light->Position;

				float distance = D3DXVec4Length(&lightVector);
				D3DXVec4Normalize(&lightVector, &lightVector);

				float intensity;
				float attenuation;
				float angle;
				float d;
				float attenuationRange;
				float attenuationAngle;

				switch (light->Type)
				{
				case LIGHT_TYPES::LIGHT_TYPE_POINT:
					if (distance > light->Out)
						continue;

					attenuation = 1.0f - distance / light->Out;
					intensity = max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

					if (intensity >= brightest)
					{
						brightest = intensity;
						brightestLight = light;
					}

					break;

				case  LIGHT_TYPES::LIGHT_TYPE_SPOT:
					if (distance > light->Range)
						continue;

					attenuation = 1.0f - distance / light->Range;
					intensity = max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

					if (intensity >= brightest)
					{
						brightest = intensity;
						brightestLight = light;
					}

					break;
					/*case LIGHTTYPE_SPOT:
						if (distance > light->Range)
							continue;

						d = D3DXVec3Dot(&lightDir, &lightVector);
						angle = acosf(d);

						if (angle > light->RadOut)
							continue;

						attenuationRange = 1.0f - distance / light->Range;
						attenuationAngle = 1.0f - angle / light->RadOut;
						intensity = max(0.0f, attenuationRange * attenuationAngle * (light->r + light->g + light->b) / 3.0f);

						if (intensity >= brightest)
						{
							brightest = intensity;
							brightestLight = light;
						}

						break;
				}

				light++;
			}

			// Try also dynamic lights
			for (__int32 j = 0; j < m_dynamicLights.size(); j++)
			{
				RendererLight* dynamicLight = m_dynamicLights[j];

				D3DXVECTOR4 itemPos = D3DXVECTOR4(item->pos.xPos, item->pos.yPos, item->pos.zPos, 1.0f);
				D3DXVECTOR4 lightVector = itemPos - dynamicLight->Position;

				float distance = D3DXVec4Length(&lightVector);
				D3DXVec4Normalize(&lightVector, &lightVector);

				float intensity;
				float attenuation;
				float angle;
				float d;
				float attenuationRange;
				float attenuationAngle;

				switch (dynamicLight->Type)
				{
				case LIGHT_TYPES::LIGHT_TYPE_POINT:
					if (distance > dynamicLight->Out)
						continue;

					attenuation = 1.0f - distance / dynamicLight->Out;
					intensity = max(0.0f, attenuation * (dynamicLight->Color.x + dynamicLight->Color.y + dynamicLight->Color.z) / 3.0f);

					if (intensity >= brightest)
					{
						brightest = intensity;
						brightestLight = dynamicLight;
					}

					break;

				case  LIGHT_TYPES::LIGHT_TYPE_SPOT:
					if (distance > dynamicLight->Range)
						continue;

					attenuation = 1.0f - distance / dynamicLight->Range;
					intensity = max(0.0f, attenuation * (dynamicLight->Color.x + dynamicLight->Color.y + dynamicLight->Color.z) / 3.0f);

					if (intensity >= brightest)
					{
						brightest = intensity;
						brightestLight = dynamicLight;
					}

					break;
				}
			}

			// If the brightest light is found, then fill the data structure
			if (brightestLight != NULL)
			{
				m_itemsLightInfo[itemIndex].Active = true;
				m_itemsLightInfo[itemIndex].AmbientLight = room->AmbientLight;
				m_itemsLightInfo[itemIndex].Light = brightestLight;

				// Add the light to shadow maps caster, if possible
				if (m_shadowLight == NULL)
				{
					m_shadowLight = brightestLight;
				}

				if (m_shadowLight == brightestLight)
				{
					m_litItems.push_back(i);
					break;
				}
			}
		}*
	}*/
}
 
void Renderer::PrepareShadowMaps()
{
	UINT cPasses = 1;
	D3DXMATRIX world;

	// We don't need 
	RENDERER_BUCKETS buckets[4] = {
		RENDERER_BUCKETS::RENDERER_BUCKET_SOLID,
		RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS,
		RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST,
		RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS
	};

	if (m_shadowLight == NULL)
		return;

	D3DXVECTOR3 lightPos = D3DXVECTOR3(m_shadowLight->Position.x, m_shadowLight->Position.y, m_shadowLight->Position.z);
	 
	m_depthShader->GetEffect()->Begin(&cPasses, 0);
	m_depthShader->SetTexture(m_textureAtlas);

	D3DXMatrixPerspectiveFovRH(&m_lightProjection, 90.0f * RADIAN, 1.0f, 1.0f, m_shadowLight->Out * 1.5f);
	m_depthShader->SetProjection(&m_lightProjection);

	/*for (__int32 f = 0; f < 6; f++)
	{ 
		D3DXVECTOR3 direction = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		switch (f)
		{
		case D3DCUBEMAP_FACES::D3DCUBEMAP_FACE_POSITIVE_X:
			direction = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
			break;
		case D3DCUBEMAP_FACES::D3DCUBEMAP_FACE_NEGATIVE_X:
			direction = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
			break;
		case D3DCUBEMAP_FACES::D3DCUBEMAP_FACE_POSITIVE_Y:
			direction = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
			break;
		case D3DCUBEMAP_FACES::D3DCUBEMAP_FACE_NEGATIVE_Y:
			direction = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			break;
		case D3DCUBEMAP_FACES::D3DCUBEMAP_FACE_POSITIVE_Z:
			direction = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
			break;
		case D3DCUBEMAP_FACES::D3DCUBEMAP_FACE_NEGATIVE_Z:
			direction = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
			break;
		}

		D3DXMatrixLookAtRH(&m_lightView,
			&lightPos, 
			&direction,
			&D3DXVECTOR3(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos),
			&D3DXVECTOR3(0.0f, -1.0f, 0.0f));
		*/

	D3DXMatrixLookAtRH(&m_lightView,
		&lightPos,
		&D3DXVECTOR3(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos),
		&D3DXVECTOR3(0.0f, -1.0f, 0.0f));

		m_depthShader->SetView(&m_lightView);
		
		// Bind the shadow map
		//m_shadowMapCube->Bind((D3DCUBEMAP_FACES)f);
		m_shadowMap->Bind();

		// Begin the scene 
		m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFFFFFFFF, 1.0f, 0);
		m_device->BeginScene();

		//Draw Lara
		for (__int32 k = 0; k < 4; k++)
		{
			DrawLaraLPP((RENDERER_BUCKETS)k, RENDERER_PASSES::RENDERER_PASS_SHADOW_MAP);
		}
		   
		// Draw visible items
		for (__int32 j = 0; j < m_itemsToDraw.size(); j++)
		{
			RendererItemToDraw* itemToDraw = m_itemsToDraw[j];
			ITEM_INFO* item = itemToDraw->Item;

			D3DXVECTOR3 itemPos = D3DXVECTOR3(item->pos.xPos, item->pos.yPos, item->pos.zPos);
			D3DXVECTOR3 lightVector = (itemPos - lightPos);
			float distance = D3DXVec3Length(&lightVector);

			if (distance > m_shadowLight->Out*1.5f)
				continue;

			for (__int32 k = 0; k < 4; k++)
			{
				DrawItemLPP(m_itemsToDraw[j], (RENDERER_BUCKETS)k, RENDERER_PASSES::RENDERER_PASS_SHADOW_MAP);
			}
		}

		m_device->EndScene();
		m_shadowMap->Unbind();

		//m_shadowMapCube->Unbind();
	//}
 
	m_depthShader->GetEffect()->End();
}

bool Renderer::DrawPrimitives(D3DPRIMITIVETYPE primitiveType, UINT baseVertexIndex, UINT minVertexIndex, UINT numVertices, UINT baseIndex, UINT primitiveCount)
{
	m_numVertices += numVertices;
	m_numTriangles += primitiveCount;
	m_numDrawCalls++;

	HRESULT res = m_device->DrawIndexedPrimitive(primitiveType, baseVertexIndex, minVertexIndex, numVertices, baseIndex, primitiveCount);
	return (res == S_OK);
}

bool Renderer::DrawRoom(__int32 roomIndex, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass)
{
	/*D3DXMATRIX world;
	UINT cPasses = 1;

	// If nothing to render then exit
	RendererRoom* room = m_rooms[roomIndex];
	RendererObject* obj = room->RoomObject;
	if (obj == NULL)
		return true;
	if (obj->ObjectMeshes.size() == 0)
		return true;

	RendererMesh* mesh = obj->ObjectMeshes[0];
	ROOM_INFO* r = room->Room;
	RendererBucket* bucket;

	// Draw room geometry first
	bucket = mesh->GetBucket(bucketIndex);
	if (bucket->NumVertices == 0)
		return true;
	 
	// Bind buffers
	m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
	m_device->SetIndices(bucket->GetIndexBuffer());

	// Set shader parameters
	LPD3DXEFFECT effect;
	if (pass == RENDERER_PASSES::RENDERER_PASS_DEPTH)
	{
		effect = m_depthShader->GetEffect();
	}
	else
	{
		effect = m_mainShader->GetEffect();

		if (m_shadowLight != NULL && LaraItem->roomNumber == roomIndex)
		{
			m_mainShader->SetEnableShadows(true);
			m_mainShader->SetShadowMap(m_shadowMap->GetTexture());
			m_mainShader->SetLightView(&m_lightView);
			m_mainShader->SetLightProjection(&m_lightProjection);

			RendererLightInfo* light = &m_itemsLightInfo[m_itemsToDraw[0]];
			m_mainShader->SetLightActive(light->Active);
			if (light->Active)
			{
				m_mainShader->SetLightPosition(&light->Light->Position);
				m_mainShader->SetLightDirection(&light->Light->Direction);
				m_mainShader->SetLightColor(&light->Light->Color);
				m_mainShader->SetLightIn(light->Light->In);
				m_mainShader->SetLightOut(light->Light->Out);
				m_mainShader->SetLightRange(light->Light->Range);
				m_mainShader->SetLightType(light->Light->Type);
			}
		}
		else
		{
			m_mainShader->SetEnableShadows(false);
			m_mainShader->SetShadowMap(NULL);
			m_mainShader->SetLightActive(false);
		}

		m_mainShader->SetAmbientLight(&room->AmbientLight);
		m_mainShader->SetModelType(MODEL_TYPES::MODEL_TYPE_ROOM);
	}
	       
	for (int iPass = 0; iPass < cPasses; iPass++)
	{
		effect->BeginPass(iPass);
		D3DXMatrixTranslation(&world, r->x, r->y, r->z);
		
		if (pass == RENDERER_PASSES::RENDERER_PASS_DEPTH)
			m_depthShader->SetWorld(&world);
		else
			m_mainShader->SetWorld(&world);

		effect->CommitChanges();

		DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

		effect->EndPass();
	}*/

	return true;
}

bool Renderer::DrawStatic(__int32 roomIndex, __int32 staticIndex, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass)
{
	D3DXMATRIX world;
	D3DXMATRIX rotation;
	UINT cPasses = 1;

	ROOM_INFO* room = &Rooms[roomIndex];
	MESH_INFO* sobj = &room->mesh[staticIndex];
	RendererObject* obj = m_staticObjects[sobj->staticNumber];
	RendererMesh* mesh = obj->ObjectMeshes[0];
	RendererBucket* bucket = mesh->GetBucket(bucketIndex);
	if (bucket->NumVertices == 0)
		return true;

	// Bind buffers
	m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
	m_device->SetIndices(bucket->GetIndexBuffer());

	LPD3DXEFFECT effect;
	if (pass == RENDERER_PASSES::RENDERER_PASS_DEPTH)
	{
		effect = m_depthShader->GetEffect();
		if (bucketIndex == RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST ||
			bucketIndex == RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS)
		{
			m_depthShader->SetBlendMode(BLENDMODE_ALPHATEST);
		}
		else
		{
			m_depthShader->SetBlendMode(BLENDMODE_OPAQUE);
		}
	}
	else
	{
		effect = m_mainShader->GetEffect();
		m_mainShader->SetModelType(MODEL_TYPES::MODEL_TYPE_STATIC);
	}
	
	for (int iPass = 0; iPass < cPasses; iPass++)
	{
		effect->BeginPass(iPass);

		D3DXMatrixTranslation(&world, sobj->x, sobj->y, sobj->z);
		D3DXMatrixRotationY(&rotation, TR_ANGLE_TO_RAD(sobj->yRot));
		D3DXMatrixMultiply(&world, &rotation, &world);

		if (pass == RENDERER_PASSES::RENDERER_PASS_DEPTH)
			m_depthShader->SetWorld(&world);
		else
			m_mainShader->SetWorld(&world);

		effect->CommitChanges();

		DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0,
			bucket->NumVertices, 0, bucket->NumIndices / 3);

		effect->EndPass();
	}

	return true;
}

void Renderer::UpdateLaraAnimations()
{
	D3DXMATRIX translation;
	D3DXMATRIX rotation;
	D3DXMATRIX lastMatrix;
	D3DXMATRIX hairMatrix;
	D3DXMATRIX identity;
	D3DXMATRIX world;

	RendererObject* laraObj = m_moveableObjects[ID_LARA];

	// Lara world matrix
	D3DXMatrixTranslation(&translation, LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
	D3DXMatrixRotationYawPitchRoll(&rotation, 
								   TR_ANGLE_TO_RAD(LaraItem->pos.yRot),
							       TR_ANGLE_TO_RAD(LaraItem->pos.xRot), 
								   TR_ANGLE_TO_RAD(LaraItem->pos.zRot));
	D3DXMatrixMultiply(&m_LaraWorldMatrix, &rotation, &translation);

	// Update first Lara's animations
	laraObj->LinearizedBones[TORSO]->ExtraRotation = D3DXVECTOR3(TR_ANGLE_TO_RAD(Lara.torsoXrot),
		TR_ANGLE_TO_RAD(Lara.torsoYrot), TR_ANGLE_TO_RAD(Lara.torsoZrot));
	laraObj->LinearizedBones[HEAD]->ExtraRotation = D3DXVECTOR3(TR_ANGLE_TO_RAD(Lara.headXrot),
		TR_ANGLE_TO_RAD(Lara.headYrot), TR_ANGLE_TO_RAD(Lara.headZrot));

	// First calculate matrices for legs, hips, head and torso
	__int32 mask = (1 << HIPS) | (1 << THIGH_L) | (1 << CALF_L) | (1 << FOOT_L) |
				   (1 << THIGH_R) | (1 << CALF_R) | (1 << FOOT_R) | (1 << TORSO) | (1 << HEAD);
	__int16	*framePtr[2];
	__int32 rate;
	__int32 frac = GetFrame_D2(LaraItem, framePtr, &rate);
	BuildAnimationPose(laraObj, framePtr, frac, rate, mask);

	// Then the arms, based on current weapon status
	if ((Lara.gunStatus == LG_NO_ARMS || Lara.gunStatus == LG_HANDS_BUSY) && Lara.gunType != WEAPON_FLARE)
	{
		// Both arms
		mask = (1 << UARM_L) | (1 << LARM_L) | (1 << HAND_L) | (1 << UARM_R) |
			   (1 << LARM_R) | (1 << HAND_R);
		frac = GetFrame_D2(LaraItem, framePtr, &rate);
		BuildAnimationPose(laraObj, framePtr, frac, rate, mask);
	}
	else
	{
		// While handling weapon some extra rotation could be applied to arms
		laraObj->LinearizedBones[UARM_L]->ExtraRotation = D3DXVECTOR3(TR_ANGLE_TO_RAD(Lara.leftArm.xRot),
			TR_ANGLE_TO_RAD(Lara.leftArm.yRot), TR_ANGLE_TO_RAD(Lara.leftArm.zRot));
		laraObj->LinearizedBones[UARM_R]->ExtraRotation = D3DXVECTOR3(TR_ANGLE_TO_RAD(Lara.rightArm.xRot),
			TR_ANGLE_TO_RAD(Lara.rightArm.yRot), TR_ANGLE_TO_RAD(Lara.rightArm.zRot));

		if (Lara.gunType != WEAPON_FLARE)
		{
			// Left arm
			mask = (1 << UARM_L) | (1 << LARM_L) | (1 << HAND_L);
			frac = GetFrame(Lara.leftArm.animNumber, Lara.leftArm.frameNumber, framePtr, &rate);
			BuildAnimationPose(laraObj, framePtr, frac, rate, mask);

			// Right arm
			mask = (1 << UARM_R) | (1 << LARM_R) | (1 << HAND_R);
			frac = GetFrame(Lara.rightArm.animNumber, Lara.rightArm.frameNumber, framePtr, &rate);
			BuildAnimationPose(laraObj, framePtr, frac, rate, mask);
		}
		else
		{
			// Left arm
			mask = (1 << UARM_L) | (1 << LARM_L) | (1 << HAND_L);
			frac = GetFrame(Lara.leftArm.animNumber, Lara.leftArm.frameNumber, framePtr, &rate);
			BuildAnimationPose(laraObj, framePtr, frac, rate, mask);

			// Right arm
			mask = (1 << UARM_R) | (1 << LARM_R) | (1 << HAND_R);
			frac = GetFrame_D2(LaraItem, framePtr, &rate);
			BuildAnimationPose(laraObj, framePtr, frac, rate, mask);
		}
	}

	// At this point, Lara's matrices are ready. Now let's do ponytails...
	if (m_moveableObjects.find(ID_HAIR) != m_moveableObjects.end())
	{
		RendererObject* hairsObj = m_moveableObjects[ID_HAIR];

		D3DXMatrixIdentity(&lastMatrix);
		D3DXMatrixIdentity(&identity);

		D3DXVECTOR3 parentVertices[6][4];
		D3DXMATRIX headMatrix;

		RendererObject* objSkin = m_moveableObjects[ID_LARA_SKIN];
		RendererObject* objLara = m_moveableObjects[ID_LARA];
		RendererMesh* parentMesh = objSkin->ObjectMeshes[HEAD];
		RendererBone* parentBone = objSkin->LinearizedBones[HEAD];

		D3DXMatrixMultiply(&world, &objLara->AnimationTransforms[HEAD], &m_LaraWorldMatrix);

		// We can't use hardware skinning here, however hairs have just a few vertices so 
		// it's not so bad doing skinning in software
		D3DXVec3TransformCoord(&parentVertices[0][0], &parentMesh->Positions[37], &world);
		D3DXVec3TransformCoord(&parentVertices[0][1], &parentMesh->Positions[39], &world);
		D3DXVec3TransformCoord(&parentVertices[0][2], &parentMesh->Positions[40], &world);
		D3DXVec3TransformCoord(&parentVertices[0][3], &parentMesh->Positions[38], &world);

		__int32 lastVertex = 0;
		__int32 lastIndex = 0;

		ZeroMemory(m_hairVertices, m_numHairVertices * sizeof(RendererObject));
		ZeroMemory(m_hairIndices, m_numHairIndices * 4);

		for (__int32 i = 0; i < 6; i++)
		{
			RendererMesh* mesh = hairsObj->ObjectMeshes[i];
			RendererBucket* bucket = mesh->GetBucket(RENDERER_BUCKETS::RENDERER_BUCKET_SOLID);

			D3DXMatrixTranslation(&translation, Hairs[i].pos.xPos, Hairs[i].pos.yPos, Hairs[i].pos.zPos);
			D3DXMatrixRotationYawPitchRoll(&rotation, 
										   TR_ANGLE_TO_RAD(Hairs[i].pos.yRot),
										   TR_ANGLE_TO_RAD(Hairs[i].pos.xRot), 
										   TR_ANGLE_TO_RAD(Hairs[i].pos.zRot));
			D3DXMatrixMultiply(&m_hairsMatrices[i], &rotation, &translation);

			__int32 baseVertex = lastVertex;

			for (__int32 j = 0; j < bucket->Vertices.size(); j++)
			{
				__int32 oldVertexIndex = (__int32)bucket->Vertices[j].bone;
				if (oldVertexIndex < 4)
				{
					m_hairVertices[lastVertex].x = parentVertices[i][oldVertexIndex].x;
					m_hairVertices[lastVertex].y = parentVertices[i][oldVertexIndex].y;
					m_hairVertices[lastVertex].z = parentVertices[i][oldVertexIndex].z;
					m_hairVertices[lastVertex].u = bucket->Vertices[j].u;
					m_hairVertices[lastVertex].v = bucket->Vertices[j].v;

					lastVertex++;
				}
				else
				{
					D3DXVECTOR3 in = D3DXVECTOR3(bucket->Vertices[j].x, bucket->Vertices[j].y, bucket->Vertices[j].z);
					D3DXVECTOR4 out;

					D3DXVec3Transform(&out, &in, &m_hairsMatrices[i]);

					if (i < 5)
					{
						parentVertices[i + 1][oldVertexIndex - 4].x = out.x;
						parentVertices[i + 1][oldVertexIndex - 4].y = out.y;
						parentVertices[i + 1][oldVertexIndex - 4].z = out.z;
					}

					m_hairVertices[lastVertex].x = out.x;
					m_hairVertices[lastVertex].y = out.y;
					m_hairVertices[lastVertex].z = out.z;
					m_hairVertices[lastVertex].u = bucket->Vertices[j].u;
					m_hairVertices[lastVertex].v = bucket->Vertices[j].v;

					lastVertex++;
				}
			}

			for (__int32 j = 0; j < bucket->Indices.size(); j++)
			{
				m_hairIndices[lastIndex] = baseVertex + bucket->Indices[j];
				lastIndex++;
			}
		}
	}
}

void Renderer::UpdateItemsAnimations()
{
	D3DXMATRIX translation;
	D3DXMATRIX rotation;

	// Alloc matrices of needed
	//if (m_itemsObjectMatrices == NULL)
	//	m_itemsObjectMatrices = (D3DXMATRIX*)malloc(sizeof(D3DXMATRIX) * NumItems);

	for (__int32 i = 0; i < m_itemsToDraw.size(); i++)
	{
		RendererItemToDraw* itemToDraw = m_itemsToDraw[i];
		ITEM_INFO* item = itemToDraw->Item;

		// Lara has her own routine
		if (item->objectNumber == ID_LARA || item->objectNumber == 434)
			continue;

		OBJECT_INFO* obj = &Objects[item->objectNumber];
		RendererObject* moveableObj = m_moveableObjects[item->objectNumber];

		// Update animation matrices
		if (obj->animIndex != -1)
		{
			__int16	*framePtr[2];
			__int32 rate;
			__int32 frac = GetFrame_D2(item, framePtr, &rate);
			BuildAnimationPose(moveableObj, framePtr, frac, rate, 0xFFFFFFFF);
		}

		// Update world matrix
		D3DXMatrixTranslation(&translation, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		D3DXMatrixRotationYawPitchRoll(&rotation, TR_ANGLE_TO_RAD(item->pos.yRot),
			TR_ANGLE_TO_RAD(item->pos.xRot), TR_ANGLE_TO_RAD(item->pos.zRot));
		D3DXMatrixMultiply(&itemToDraw->World, &rotation, &translation);
	}
}

bool Renderer::DrawLara(RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass)
{
	/*D3DXMATRIX world;
	D3DXMATRIX translation;
	D3DXMATRIX rotation;
	UINT cPasses = 1;

	RendererObject* laraObj = m_moveableObjects[ID_LARA];
	RendererObject* laraSkin = m_moveableObjects[ID_LARA_SKIN];
	 
	ITEM_INFO* item = LaraItem;
	OBJECT_INFO* obj = &Objects[0];

	RendererLightInfo* light = &m_itemsLightInfo[0];
	LPD3DXEFFECT effect;
	
	if (pass == RENDERER_PASSES::RENDERER_PASS_DEPTH)
	{
		effect = m_depthShader->GetEffect();

		m_depthShader->SetUseSkinning(true);
		m_depthShader->SetBones(laraObj->AnimationTransforms, laraObj->ObjectMeshes.size());
	}
	else
	{
		effect = m_mainShader->GetEffect();

		m_mainShader->SetUseSkinning(true);
		m_mainShader->SetBones(laraObj->AnimationTransforms, laraObj->ObjectMeshes.size());
		m_mainShader->SetModelType(MODEL_TYPES::MODEL_TYPE_LARA);

		RendererLightInfo* light = &m_itemsLightInfo[m_itemsToDraw[m_itemsToDraw[0]]];
		m_mainShader->SetLightActive(light->Active);
		if (light->Active)
		{
			m_mainShader->SetLightPosition(&light->Light->Position);
			m_mainShader->SetLightDirection(&light->Light->Direction);
			m_mainShader->SetLightColor(&light->Light->Color);
			m_mainShader->SetLightIn(light->Light->In);
			m_mainShader->SetLightOut(light->Light->Out);
			m_mainShader->SetLightRange(light->Light->Range);
			m_mainShader->SetLightType(light->Light->Type);
		}
	}

	for (__int32 i = 0; i < laraObj->ObjectMeshes.size(); i++)
	{
		// Lara has meshes overriden by weapons, crowbar, etc
		RendererMesh* mesh = MeshPointersToMesh[Lara.meshPtrs[i]];
		RendererBucket* bucket = mesh->GetBucket(bucketIndex);

		if (bucket->NumVertices == 0)
			continue;
				
		// Bind buffers
		m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
		m_device->SetIndices(bucket->GetIndexBuffer());

		for (int iPass = 0; iPass < cPasses; iPass++)
		{
			effect->BeginPass(iPass);

			if (pass == RENDERER_PASSES::RENDERER_PASS_DEPTH)
				m_depthShader->SetWorld(&m_LaraWorldMatrix);
			else
				m_mainShader->SetWorld(&m_LaraWorldMatrix);

			effect->CommitChanges();

			DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

			effect->EndPass();
		}

		// Draw joints, if present
		if (m_moveableObjects.find(ID_LARA_SKIN_JOINTS) != m_moveableObjects.end())
		{
			RendererObject* laraSkinJoints = m_moveableObjects[ID_LARA_SKIN_JOINTS];
			RendererMesh* jointMesh = laraSkinJoints->ObjectMeshes[i];
			bucket = jointMesh->GetBucket(bucketIndex);

			// Bind buffers
			m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
			m_device->SetIndices(bucket->GetIndexBuffer());

			for (int iPass = 0; iPass < cPasses; iPass++)
			{
				effect->BeginPass(iPass);

				if (pass == RENDERER_PASSES::RENDERER_PASS_DEPTH)
					m_depthShader->SetWorld(&m_LaraWorldMatrix);
				else
					m_mainShader->SetWorld(&m_LaraWorldMatrix);

				effect->CommitChanges();

				DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

				effect->EndPass();
			}
		}
	}

	// Disable skinning, following will use the old way
	if (pass == RENDERER_PASSES::RENDERER_PASS_DEPTH)
		m_depthShader->SetUseSkinning(false);
	else
		m_mainShader->SetUseSkinning(false);

	if (!(gfLevelFlags & 1))
	{
		// Draw holsters
		OBJECT_INFO* objHolsters = &Objects[Lara.holster];
		RendererObject* modelHolster = m_moveableObjects[Lara.holster];

		RendererMesh* leftHolster = modelHolster->ObjectMeshes[4];
		RendererMesh* rightHolster = modelHolster->ObjectMeshes[8];

		RendererBucket* bucket = leftHolster->GetBucket(bucketIndex);

		m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
		m_device->SetIndices(bucket->GetIndexBuffer());

		D3DXMatrixMultiply(&world, &laraObj->AnimationTransforms[THIGH_L], &m_LaraWorldMatrix);

		for (int iPass = 0; iPass < cPasses; iPass++)
		{
			effect->BeginPass(iPass);

			if (pass == RENDERER_PASSES::RENDERER_PASS_DEPTH)
				m_depthShader->SetWorld(&world);
			else
				m_mainShader->SetWorld(&world);

			effect->CommitChanges();

			DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0,
				bucket->NumVertices, 0, bucket->NumIndices / 3);

			effect->EndPass();
		}

		bucket = rightHolster->GetBucket(bucketIndex);

		m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
		m_device->SetIndices(bucket->GetIndexBuffer());

		D3DXMatrixMultiply(&world, &laraObj->AnimationTransforms[THIGH_R], &m_LaraWorldMatrix);

		for (int iPass = 0; iPass < cPasses; iPass++)
		{
			effect->BeginPass(iPass);

			if (pass == RENDERER_PASSES::RENDERER_PASS_DEPTH)
				m_depthShader->SetWorld(&world);
			else
				m_mainShader->SetWorld(&world);

			effect->CommitChanges();

			DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

			effect->EndPass();
		}

		// Draw back gun
		if (Lara.backGun)
		{
			OBJECT_INFO* backGunObject = &Objects[Lara.backGun];
			RendererObject* modelBackGun = m_moveableObjects[Lara.backGun];
			RendererMesh* backGunMesh = modelBackGun->ObjectMeshes[HEAD];

			RendererBucket* bucket = backGunMesh->GetBucket(bucketIndex);

			m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
			m_device->SetIndices(bucket->GetIndexBuffer());

			D3DXMatrixMultiply(&world, &laraObj->AnimationTransforms[HEAD], &m_LaraWorldMatrix);

			for (int iPass = 0; iPass < cPasses; iPass++)
			{
				effect->BeginPass(iPass);

				if (pass == RENDERER_PASSES::RENDERER_PASS_DEPTH)
					m_depthShader->SetWorld(&world);
				else
					m_mainShader->SetWorld(&world);

				effect->CommitChanges();

				DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

				effect->EndPass();
			}
		}
	}

	// Draw Lara's hairs
	if (m_moveableObjects.find(ID_HAIR) != m_moveableObjects.end())
	{
		RendererObject* hairsObj = m_moveableObjects[ID_HAIR];
		D3DXMatrixIdentity(&world);

		for (int iPass = 0; iPass < cPasses; iPass++)
		{
			effect->BeginPass(iPass);

			if (pass == RENDERER_PASSES::RENDERER_PASS_DEPTH)
				m_depthShader->SetWorld(&world);
			else
				m_mainShader->SetWorld(&world);

			effect->CommitChanges();

			m_device->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, m_numHairVertices, m_numHairIndices / 3,
											 m_hairIndices, D3DFMT_INDEX32, m_hairVertices, sizeof(RendererVertex));

			effect->EndPass();
		}
	}

	// Draw gunflashes
	__int16 length = 0;
	__int16 zOffset = 0;
	__int16 rotationX = 0;

	if (Lara.weaponItem != WEAPON_FLARE && Lara.weaponItem != WEAPON_SHOTGUN && Lara.weaponItem != WEAPON_CROSSBOW)
	{
		if (Lara.weaponItem == WEAPON_REVOLVER)
		{
			length = 192;
			zOffset = 68;
			rotationX = -14560;
		}
		else if (Lara.weaponItem == WEAPON_UZI)
		{
			length = 190;
			zOffset = 50;
		}
		else if (Lara.weaponItem == WEAPON_HK)
		{
			length = 300;
			zOffset = 92;
			rotationX = -14560;
		}
		else
		{
			length = 180;
			zOffset = 40;
			rotationX = -16830;
		}

		OBJECT_INFO* flashObj = &Objects[ID_GUN_FLASH];
		RendererObject* flashMoveable = m_moveableObjects[ID_GUN_FLASH];
		RendererMesh* flashMesh = flashMoveable->ObjectMeshes[0];
		RendererBucket* flashBucket = flashMesh->GetBucket(bucketIndex);

		if (flashBucket->NumVertices != 0)
		{
			m_device->SetStreamSource(0, flashBucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
			m_device->SetIndices(flashBucket->GetIndexBuffer());

			D3DXMATRIX offset;
			D3DXMatrixTranslation(&offset, 0, length, zOffset);

			D3DXMATRIX rotation2;
			D3DXMatrixRotationX(&rotation2, TR_ANGLE_TO_RAD(rotationX));

			m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
			m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
			m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);

			if (Lara.leftArm.flash_gun)
			{
				D3DXMatrixMultiply(&world, &laraObj->AnimationTransforms[HAND_L], &m_LaraWorldMatrix);
				D3DXMatrixMultiply(&world, &offset, &world);
				D3DXMatrixMultiply(&world, &rotation2, &world);

				for (int iPass = 0; iPass < cPasses; iPass++)
				{
					effect->BeginPass(iPass);
					
					if (pass == RENDERER_PASSES::RENDERER_PASS_DEPTH)
						m_depthShader->SetWorld(&world);
					else
						m_mainShader->SetWorld(&world);
					
					effect->CommitChanges();

					DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, flashBucket->NumVertices, 0, flashBucket->NumIndices / 3);

					effect->EndPass();
				}
			}

			if (Lara.rightArm.flash_gun)
			{
				D3DXMatrixMultiply(&world, &laraObj->AnimationTransforms[HAND_R], &m_LaraWorldMatrix);
				D3DXMatrixMultiply(&world, &offset, &world);
				D3DXMatrixMultiply(&world, &rotation2, &world);

				for (int iPass = 0; iPass < cPasses; iPass++)
				{
					effect->BeginPass(iPass);

					if (pass == RENDERER_PASSES::RENDERER_PASS_DEPTH)
						m_depthShader->SetWorld(&world);
					else
						m_mainShader->SetWorld(&world);

					effect->CommitChanges();

					DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, flashBucket->NumVertices, 0, flashBucket->NumIndices / 3);

					effect->EndPass();
				}
			}

			m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		}
	}*/

	return true;
}

bool Renderer::DrawItem(__int32 itemIndex, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass)
{
	/*D3DXMATRIX world;
	UINT cPasses = 1;
	 
	ITEM_INFO* item = &Items[m_itemsToDraw[itemIndex]];
	if (item->objectNumber == ID_LARA || item->objectNumber == 434)
		return true;
	   
	OBJECT_INFO* obj = &Objects[item->objectNumber];
	RendererObject* moveableObj = m_moveableObjects[item->objectNumber];
	   
	LPD3DXEFFECT effect;
	if (pass == RENDERER_PASSES::RENDERER_PASS_DEPTH)
	{
		effect = m_depthShader->GetEffect();
		m_depthShader->SetUseSkinning(true);
		m_depthShader->SetBones(moveableObj->AnimationTransforms, moveableObj->ObjectMeshes.size());
	}
	else
	{
		effect = m_mainShader->GetEffect();
		m_mainShader->SetModelType(MODEL_TYPES::MODEL_TYPE_MOVEABLE);
		m_mainShader->SetEnableShadows(false);
		m_mainShader->SetUseSkinning(true);
		m_mainShader->SetBones(moveableObj->AnimationTransforms, moveableObj->ObjectMeshes.size());

		RendererLightInfo* light = &m_itemsLightInfo[itemIndex];
		m_mainShader->SetLightActive(light->Active);
		if (light->Active)
		{
			m_mainShader->SetLightPosition(&light->Light->Position);
			m_mainShader->SetLightDirection(&light->Light->Direction);
			m_mainShader->SetLightColor(&light->Light->Color);
			m_mainShader->SetLightIn(light->Light->In);
			m_mainShader->SetLightOut(light->Light->Out);
			m_mainShader->SetLightRange(light->Light->Range);
			m_mainShader->SetLightType(light->Light->Type);
		}
	}

	for (__int32 i = 0; i < moveableObj->ObjectMeshes.size(); i++)
	{
		RendererMesh* mesh = moveableObj->ObjectMeshes[i];
		RendererBucket* bucket = mesh->GetBucket(bucketIndex);
		if (bucket->NumVertices == 0)
			continue;


		m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
		m_device->SetIndices(bucket->GetIndexBuffer());

		for (int iPass = 0; iPass < cPasses; iPass++)
		{
			effect->BeginPass(iPass);

			if (pass == RENDERER_PASSES::RENDERER_PASS_DEPTH)
				m_depthShader->SetWorld(&m_itemsObjectMatrices[m_itemsToDraw[itemIndex]]);
			else
				m_mainShader->SetWorld(&m_itemsObjectMatrices[m_itemsToDraw[itemIndex]]);

			effect->CommitChanges();

			DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

			effect->EndPass();
		}
	}*/

	return true;
}

void Renderer::DrawSky()
{
	D3DXMATRIX world;
	D3DXMATRIX scale;
	UINT cPasses = 1;

	RendererObject* horizonObj = m_moveableObjects[ID_HORIZON];
	RendererMesh* mesh = horizonObj->ObjectMeshes[0];

	m_mainShader->SetModelType(MODEL_TYPES::MODEL_TYPE_HORIZON);

	for (__int32 i = 0; i < NUM_BUCKETS; i++)
	{
		RendererBucket* bucket = mesh->GetBucket(i);

		// Bind buffers
		m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
		m_device->SetIndices(bucket->GetIndexBuffer());

		for (int iPass = 0; iPass < cPasses; iPass++)
		{
			m_mainShader->GetEffect()->BeginPass(iPass);

			D3DXMatrixTranslation(&world, LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
			D3DXMatrixScaling(&scale, 16.0f, 16.0f, 16.0f);
			D3DXMatrixMultiply(&world, &scale, &world);

			m_mainShader->SetWorld(&world);

			m_mainShader->GetEffect()->CommitChanges();

			DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

			m_mainShader->GetEffect()->EndPass();
		}
	}

	m_device->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);
}

__int32 Renderer::DrawScene()
{

	DrawSceneLightPrePass(false);
	return 1;

	D3DXMATRIX translation;
	D3DXMATRIX rotation;
	D3DXMATRIX scale;
	D3DXMATRIX world;
	 
	m_numLines = 0;

	// Clip and collect rooms and items
	CollectRooms();
	CollectItems();

	// Update dynamic lights
	for (vector<RendererLight*>::iterator it = m_dynamicLights.begin(); it != m_dynamicLights.end(); ++it)
		delete (*it);
	m_dynamicLights.clear();
	UpdateGunFlashes();
	
	// Collect lights
	CollectLights();

	// Update all animations
	UpdateLaraAnimations();
	UpdateItemsAnimations();

	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	m_device->SetRenderState(D3DRS_ALPHATESTENABLE, false);

	PrepareShadowMaps();

	m_numVertices = 0;
	m_numTriangles = 0;
	 
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	  
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	m_device->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	 
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	m_device->BeginScene();
	  
	UINT cPasses = 1;
	m_mainShader->GetEffect()->Begin(&cPasses, 0);

	m_mainShader->SetTexture(m_textureAtlas);
	m_mainShader->SetView(&ViewMatrix);
	m_mainShader->SetProjection(&ProjectionMatrix);

	m_device->SetVertexDeclaration(m_vertexDeclaration);
	 	
	/*if (m_fadeTimer < 30)
		m_shader->SetFloat(m_shader->GetParameterByName(NULL, "FadeTimer"), m_fadeTimer++);
	
	if (UseSpotCam && SpotCam[CurrentSplineCamera].flags & 0x400)
		m_shader->SetBool(m_shader->GetParameterByName(NULL, "CinematicMode"), true);
	else
		m_shader->SetBool(m_shader->GetParameterByName(NULL, "CinematicMode"), false);*/

	// Draw skybox if needed
	if (m_moveableObjects.find(ID_HORIZON) != m_moveableObjects.end())
	{
		DrawSky();
	}
	 
	// Solid faces buckets
	for (__int32 i = 0; i < m_roomsToDraw.size(); i++)
	{
		RendererRoom* room = m_rooms[m_roomsToDraw[i]];
		RendererObject* obj = room->RoomObject;
		if (obj == NULL)
			continue;

		if (obj->ObjectMeshes.size() == 0)
			continue;

		RendererMesh* mesh = obj->ObjectMeshes[0];

		if (mesh->GetBucket(RENDERER_BUCKETS::RENDERER_BUCKET_SOLID)->NumVertices != 0)
			DrawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_DRAW);

		if (mesh->GetBucket(RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS)->NumVertices != 0)
		{
			m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			DrawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_DRAW);
			m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		}

		// Draw static objects
		ROOM_INFO* r = room->Room;
		if (r->numMeshes != 0)
		{
			for (__int32 j = 0; j < r->numMeshes; j++)
			{
				MESH_INFO* sobj = &r->mesh[j];
				RendererObject* staticObj = m_staticObjects[sobj->staticNumber];
				RendererMesh* staticMesh = staticObj->ObjectMeshes[0];

				if (staticMesh->GetBucket(RENDERER_BUCKETS::RENDERER_BUCKET_SOLID)->NumVertices != 0)
					DrawStatic(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_DRAW);

				if (staticMesh->GetBucket(RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS)->NumVertices != 0)
				{
					m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
					DrawStatic(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_DRAW);
					m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
				}
			}
		}
	}

	// Draw Lara
	DrawLara(RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_DRAW);

	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	DrawLara(RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_DRAW);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	// Draw items
	for (__int32 k = 0; k < m_itemsToDraw.size(); k++)
	{
		DrawItem(k, RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_DRAW);

		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		DrawItem(k, RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_DRAW);

		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	}

	// Transparent and alpha test faces
	for (__int32 i = 0; i < m_roomsToDraw.size(); i++)
	{
		DrawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_DRAW);
		DrawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT, RENDERER_PASSES::RENDERER_PASS_DRAW);

		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		DrawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_DRAW);
		DrawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT_DS, RENDERER_PASSES::RENDERER_PASS_DRAW);

		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

		// Draw static objects
		ROOM_INFO* room = &Rooms[m_roomsToDraw[i]];
		if (room->numMeshes != 0)
		{
			for (__int32 j = 0; j < room->numMeshes; j++)
			{
				DrawStatic(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_DRAW);
				DrawStatic(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT, RENDERER_PASSES::RENDERER_PASS_DRAW);

				m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
				DrawStatic(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_DRAW);
				DrawStatic(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT_DS, RENDERER_PASSES::RENDERER_PASS_DRAW);
				m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
			}
		}
	}

	// Draw Lara
	DrawLara(RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_DRAW);
	DrawLara(RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT, RENDERER_PASSES::RENDERER_PASS_DRAW);

	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	DrawLara(RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_DRAW);
	DrawLara(RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT_DS, RENDERER_PASSES::RENDERER_PASS_DRAW);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	
	// Draw items
	for (__int32 k = 0; k <  m_itemsToDraw.size(); k++)
	{
		/*DrawItem(k, RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_DRAW);
		DrawItem(k, RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT, RENDERER_PASSES::RENDERER_PASS_DRAW);

		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		DrawItem(k, RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_DRAW);
		DrawItem(k, RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT_DS, RENDERER_PASSES::RENDERER_PASS_DRAW);
		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);*/
	}
	
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	//DrawObjectOn2DPosition(0, 0, 349, 0, 0, 0);
	DrawAllPickups();

	m_mainShader->GetEffect()->End();

	DrawGameInfo();
	DrawDebugInfo();

	DrawAllLines2D();

	//DoPauseMenu(0);

	m_device->EndScene();
	m_device->Present(NULL, NULL, NULL, NULL);

	return 1;
}

__int32 Renderer::DumpGameScene()
{
	//BeginRenderToTexture();
	//m_renderTarget->Bind();
	DrawSceneLightPrePass(true);
	//m_renderTarget->Unbind();
	//EndRenderToTexture();
	//D3DXSaveTextureToFile("E:\\rt.png", D3DXIMAGE_FILEFORMAT::D3DXIFF_PNG, m_renderTarget->GetTexture(), NULL);

	return 1;
}

__int32	Renderer::BeginRenderToTexture()
{
	/*m_backBufferTarget = NULL;
	m_device->GetRenderTarget(0, &m_backBufferTarget);

	LPDIRECT3DSURFACE9 surface;
	m_renderTarget->GetSurfaceLevel(0, &surface);
	m_device->SetRenderTarget(0, surface);
	surface->Release();
	
	m_backBufferDepth = NULL;
	m_device->GetDepthStencilSurface(&m_backBufferDepth);
	m_device->SetDepthStencilSurface(m_renderTargetDepth);
*/
	return 1;
}

__int32 Renderer::EndRenderToTexture()
{
	/*m_device->SetRenderTarget(0, m_backBufferTarget);
	m_device->SetDepthStencilSurface(m_backBufferDepth);*/

	return 1;
}

__int32 Renderer::Draw()
{
	//umpScreen();
	//D3DXSaveTextureToFile("E:\\RenderTarget.png", D3DXIMAGE_FILEFORMAT::D3DXIFF_PNG, m_renderTarget, NULL);

	DrawScene();

	return 1;
}

void Renderer::InsertLine2D(__int32 x1, __int32 y1, __int32 x2, __int32 y2, byte r, byte g, byte b)
{
	m_lines[m_numLines].Vertices[0] = D3DXVECTOR2(x1, y1);
	m_lines[m_numLines].Vertices[1] = D3DXVECTOR2(x2, y2);
	m_lines[m_numLines].Color = D3DCOLOR_XRGB(r, g, b);
	m_numLines++;
}

void Renderer::DrawAllLines2D()
{
	m_line->SetWidth(1);
	m_line->SetPattern(0xffffffff);
	m_line->Begin();

	for (__int32 i = 0; i < m_numLines; i++)
	{
		m_line->Draw(m_lines[i].Vertices, 2, m_lines[i].Color);
	}

	m_line->End();
}

void Renderer::DrawBar(__int32 x, __int32 y, __int32 w, __int32 h, __int32 percent, __int32 color1, __int32 color2)
{
	byte r1 = (color1 >> 16) & 0xFF;
	byte g1 = (color1 >> 8) & 0xFF;
	byte b1 = (color1 >> 0) & 0xFF;

	byte r2 = (color2 >> 16) & 0xFF;
	byte g2 = (color2 >> 8) & 0xFF;
	byte b2 = (color2 >> 0) & 0xFF;

	float factor = ScreenWidth / 800.0f;

	__int32 realX = x * factor;
	__int32 realY = y * factor;
	__int32 realW = w * factor;
	__int32 realH = h * factor;

	__int32 realPercent = percent / 100.0f * realW;

	for (__int32 i = 0; i < realH; i++)
		InsertLine2D(realX, realY + i, realX + realW, realY + i, 0, 0, 0);

	for (__int32 i = 0; i < realH; i++)
		InsertLine2D(realX, realY + i, realX + realPercent, realY + i, r1, g1, b1);

	InsertLine2D(realX, realY, realX + realW, realY, 255, 255, 255);
	InsertLine2D(realX, realY + realH, realX + realW, realY + realH, 255, 255, 255);
	InsertLine2D(realX, realY, realX, realY + realH, 255, 255, 255);
	InsertLine2D(realX + realW, realY, realX + realW, realY + realH + 1, 255, 255, 255);
}

void Renderer::DrawGameInfo()
{
	__int32 flashState = FlashIt();

	UpdateHealtBar(flashState);
	UpdateAirBar(flashState);
	DrawDashBar();
}

void Renderer::DrawDashBar()
{
	if (DashTimer < 120)
		DrawBar(460, 32, 150, 12, 100 * (unsigned __int16)DashTimer / 120, 0xA0A000, 0xA000);
}

void Renderer::DrawAirBar(__int32 percentual)
{
	//if (CurrentLevel)
	//{
	DrawBar(450, 10, 150, 12, percentual, 0x0000A0, 0x0050A0);
	//}
}

void Renderer::DrawHealthBar(__int32 percentual)
{
	//if (CurrentLevel)
	//{
		__int32 color2 = 0xA00000;
		if (Lara.poisoned || Lara.gassed)
			color2 = 0xA0A000;
		DrawBar(10, 10, 150, 12, percentual, 0xA00000, color2);
	//}
}

void Renderer::DrawDebugInfo()
{
	if (UseSpotCam)
	{
		sprintf_s(&m_message[0], 255, "Spotcam.camera = %d    Spotcam.flags = %d     Current: %d", SpotCam[CurrentSplineCamera].camera, SpotCam[CurrentSplineCamera].flags, CurrentSplineCamera);
		PrintDebugMessage(10, 70, 255, 255, 255, 255, m_message);
	}

	__int32 y = 100;

	sprintf_s(&m_message[0], 255, "TR5Main Alpha");
	PrintDebugMessage(10, 80+y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "LaraItem.currentAnimState = %d", LaraItem->currentAnimState);
	PrintDebugMessage(10, 90 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "LaraItem.animNumber = %d", LaraItem->animNumber);
	PrintDebugMessage(10, 100 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "LaraItem.frameNumber = %d", LaraItem->frameNumber);
	PrintDebugMessage(10, 110 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "LaraItem.roomNumber = %d", LaraItem->roomNumber);
	PrintDebugMessage(10, 120 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "LaraItem.pos = < %d %d %d >", LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
	PrintDebugMessage(10, 130 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "NumVertices = %d", m_numVertices);
	PrintDebugMessage(10, 140 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "NumTriangles = %d", m_numTriangles);
	PrintDebugMessage(10, 150 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Camera.pos = < %d %d %d >", Camera.pos.x, Camera.pos.y, Camera.pos.z);
	PrintDebugMessage(10, 160 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Camera.target = < %d %d %d >", Camera.target.x, Camera.target.y, Camera.target.z);
	PrintDebugMessage(10, 170 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "CurrentLevel: %d", CurrentLevel);
	PrintDebugMessage(10, 180 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.backGun: %d", Lara.backGun);
	PrintDebugMessage(10, 190 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.leftArm.animNumber: %d", Lara.leftArm.animNumber);
	PrintDebugMessage(10, 200 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.leftArm.frameNumber: %d", Lara.leftArm.frameNumber);
	PrintDebugMessage(10, 210 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.rightArm.animNumber: %d", Lara.rightArm.animNumber);
	PrintDebugMessage(10, 220 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.rightArm.frameNumber: %d", Lara.rightArm.frameNumber);
	PrintDebugMessage(10, 230 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.weaponItem: %d", Lara.weaponItem);
	PrintDebugMessage(10, 240 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.gunType: %d", Lara.gunType);
	PrintDebugMessage(10, 250 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.gunStatus: %d", Lara.gunStatus);
	PrintDebugMessage(10, 260 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.crowbar: %d", Lara.crowbar);
	PrintDebugMessage(10, 270 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.leftArm.rot: %d %d %d", Lara.leftArm.xRot, Lara.leftArm.yRot, Lara.leftArm.zRot);
	PrintDebugMessage(10, 280 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.rightArm.rot: %d %d %d", Lara.rightArm.xRot, Lara.rightArm.yRot, Lara.rightArm.zRot);
	PrintDebugMessage(10, 290 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.torsoRot: < %d %d %d >", Lara.torsoXrot, Lara.torsoYrot, Lara.torsoZrot);
	PrintDebugMessage(10, 300 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.headRot: < %d %d %d >", Lara.headXrot, Lara.headYrot, Lara.headZrot);
	PrintDebugMessage(10, 310 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.leftArm.lock: %d", Lara.leftArm.lock);
	PrintDebugMessage(10, 320 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.rightArm.lock: %d", Lara.rightArm.lock);
	PrintDebugMessage(10, 330 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Autotarget: %d", OptionAutoTarget);
	PrintDebugMessage(10, 340 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "NumberDrawnRooms: %d", m_roomsToDraw.size());
	PrintDebugMessage(10, 350 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Camera.pos.roomNumber = %d", Camera.pos.roomNumber);
	PrintDebugMessage(10, 360 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "NumberDrawnItems = %d", m_itemsToDraw.size());
	PrintDebugMessage(10, 370 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Update = %d, ShadowMap = %d, Clear = %d, Fill = %d, Light = %d, Final = %d, Z-Buffer = %d", 
			  m_timeUpdate, m_timePrepareShadowMap, m_timeClearGBuffer, m_timeFillGBuffer, m_timeLight, m_timeCombine, m_timeReconstructZBuffer);
	PrintDebugMessage(10, 380 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "DrawCalls = %d", m_numDrawCalls);
	PrintDebugMessage(10, 390 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "NumLights = %d", m_lights.size());
	PrintDebugMessage(10, 400 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "DrawSceneTime = %d", m_timeDrawScene);
	PrintDebugMessage(10, 410 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "SkyPos = %d", SkyPos1);
	PrintDebugMessage(10, 420 + y, 255, 255, 255, 255, m_message);

//	sprintf_s(&m_message[0], 255, "Lara.lightActive = %d", m_itemsLightInfo[0].Active);
//	PrintDebugMessage(10, 380, 255, 255, 255, 255, m_message);

	m_sprite->Begin(0);

	D3DXMATRIX scale;
	D3DXMatrixScaling(&scale, 150.0f / 800.0f, 150.0f / 800.0f, 150.0f / 800.0f);
	  
	m_sprite->SetTransform(&scale);
	m_sprite->Draw(m_colorBuffer->GetTexture(), NULL, &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(0, 0, 0), 0xFFFFFFFF);
	m_sprite->SetTransform(&scale);
	m_sprite->Draw(m_normalBuffer->GetTexture(), NULL, &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(800, 0, 0), 0xFFFFFFFF);
	m_sprite->SetTransform(&scale);
	m_sprite->Draw(m_shadowBuffer->GetTexture(), NULL, &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(1600, 0, 0), 0xFFFFFFFF);
	m_sprite->SetTransform(&scale);
	m_sprite->Draw(m_vertexLightBuffer->GetTexture(), NULL, &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(2400, 0, 0), 0xFFFFFFFF);
	m_sprite->SetTransform(&scale);
	m_sprite->Draw(m_lightBuffer->GetTexture(), NULL, &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(3200, 0, 0), 0xFFFFFFFF);

	m_sprite->End();

}

__int32	Renderer::DrawPauseMenu(__int32 selectedIndex, bool resetBlink)
{
	if (resetBlink)
		ResetBlink();

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = ScreenWidth;
	rect.bottom = ScreenHeight;

	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	m_device->BeginScene();
	
	m_sprite->Begin(0);
	m_sprite->Draw(m_renderTarget->GetTexture(), &rect, &D3DXVECTOR3(0.0f, 0.0f, 0.0f), &D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DCOLOR_ARGB(255, 128, 128, 128));
	m_sprite->End();

	PrintString(400, 200, (char*)"Paused", D3DCOLOR_ARGB(255, 216, 117, 49), PRINTSTRING_CENTER);
	PrintString(400, 230, (char*)"Statistics", D3DCOLOR_ARGB(255, 255, 255, 255), PRINTSTRING_CENTER | (selectedIndex == 0 ? PRINTSTRING_BLINK : 0));
	PrintString(400, 260, (char*)"Settings", D3DCOLOR_ARGB(255, 255, 255, 255), PRINTSTRING_CENTER | (selectedIndex == 1 ? PRINTSTRING_BLINK : 0));
	PrintString(400, 290, (char*)"Exit to Title", D3DCOLOR_ARGB(255, 255, 255, 255), PRINTSTRING_CENTER | (selectedIndex == 2 ? PRINTSTRING_BLINK : 0));
	
	m_device->EndScene();
	m_device->Present(NULL, NULL, NULL, NULL);

	return 1;
}

__int32	Renderer::DrawStatisticsMenu()
{
	char buffer[255];

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = ScreenWidth;
	rect.bottom = ScreenHeight;

	m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	m_device->BeginScene();

	m_sprite->Begin(0);
	m_sprite->Draw(m_renderTarget->GetTexture(), &rect, &D3DXVECTOR3(0.0f, 0.0f, 0.0f), &D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DCOLOR_ARGB(255, 64, 64, 64));
	m_sprite->End();

	PrintString(400, 140, (char*)"Statistics", D3DCOLOR_ARGB(255, 216, 117, 49), PRINTSTRING_CENTER);

	PrintString(100, 170, (char*)"Time elapsed", D3DCOLOR_ARGB(255, 255, 255, 255), 0);
	PrintString(100, 200, (char*)"Distance traveled", D3DCOLOR_ARGB(255, 255, 255, 255), 0);
	PrintString(100, 230, (char*)"Ammo used", D3DCOLOR_ARGB(255, 255, 255, 255), 0);
	PrintString(100, 260, (char*)"Medipacks used", D3DCOLOR_ARGB(255, 255, 255, 255), 0);
	PrintString(100, 290, (char*)"Secrets found", D3DCOLOR_ARGB(255, 255, 255, 255), 0);

	sprintf_s(&buffer[0], 255, "%d", GameTimer / 30);
	PrintString(500, 170, buffer, D3DCOLOR_ARGB(255, 255, 255, 255), 0);
	sprintf_s(&buffer[0], 255, "%d", Savegame.Game.Distance / 419);
	PrintString(500, 200, buffer, D3DCOLOR_ARGB(255, 255, 255, 255), 0);
	sprintf_s(&buffer[0], 255, "%d", Savegame.Game.AmmoUsed);
	PrintString(500, 230, buffer, D3DCOLOR_ARGB(255, 255, 255, 255), 0);
	sprintf_s(&buffer[0], 255, "%d", Savegame.Game.HealthUsed);
	PrintString(500, 260, buffer, D3DCOLOR_ARGB(255, 255, 255, 255), 0);
	sprintf_s(&buffer[0], 255, "%d", Savegame.Game.Secrets);
	PrintString(500, 290, buffer, D3DCOLOR_ARGB(255, 255, 255, 255), 0);

	m_device->EndScene();
	m_device->Present(NULL, NULL, NULL, NULL);

	return 1;
}

__int32 Renderer::DrawSettingsMenu(__int32 selectedIndex, bool resetBlink)
{
	return 1;
}

__int32 Renderer::DrawInventory()
{
	return DrawInventoryScene();
}

__int32	Renderer::DrawObjectOn2DPosition(__int16 x, __int16 y, __int16 objectNum, __int16 rotX, __int16 rotY, __int16 rotZ)
{
	/*D3DXMATRIX translation;
	D3DXMATRIX rotation;
	D3DXMATRIX world;
	D3DXMATRIX view;
	D3DXMATRIX projection;
	D3DXMATRIX scale;

	UINT cPasses = 1;

	D3DXMatrixLookAtRH(&view, &D3DXVECTOR3(0.0f, 0.0f, 2048.0f), &D3DXVECTOR3(0.0f, 0.0f, 0.0f), &D3DXVECTOR3(0.0f, -1.0f, 0.0f));
	D3DXMatrixOrthoRH(&projection, ScreenWidth, ScreenHeight, -1024.0f, 1024.0f);

	OBJECT_INFO* obj = &Objects[objectNum];
	RendererObject* moveableObj = m_moveableObjects[objectNum];
	
	if (obj->animIndex != -1)
	{
		BuildAnimationPose(moveableObj, &Anims[obj->animIndex].framePtr, 0, 0, 0xFFFFFFFF);
	}

	D3DXVECTOR3 pos = D3DXVECTOR3(x, y, 1);
	D3DXMatrixIdentity(&world);

	D3DVIEWPORT9 viewport;
	m_device->GetViewport(&viewport);

	D3DXVec3Unproject(&pos, &pos, &viewport, &projection, &view, &world);
	
	m_shader->SetMatrix(m_shader->GetParameterByName(NULL, "View"), &view);
	m_shader->SetMatrix(m_shader->GetParameterByName(NULL, "Projection"), &projection);

	m_device->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);
	 
	for (__int32 i = 0; i < moveableObj->ObjectMeshes.size(); i++)
	{
		RendererMesh* mesh = moveableObj->ObjectMeshes[i];

		for (__int32 j = 0; j < NUM_BUCKETS; j++)
		{
			RendererBucket* bucket = mesh->Buckets[j];
			if (bucket->NumVertices == 0)
				continue;

			D3DXMatrixTranslation(&translation, pos.x, pos.y, pos.z + 1024);
			D3DXMatrixRotationYawPitchRoll(&rotation, TR_ANGLE_TO_RAD(rotY), TR_ANGLE_TO_RAD(rotX), TR_ANGLE_TO_RAD(rotZ));
			D3DXMatrixScaling(&scale, 0.5f, 0.5f, 0.5f);
			D3DXMatrixMultiply(&world, &scale, &rotation);
			D3DXMatrixMultiply(&world, &world, &translation);
			if (obj->animIndex != -1)
				D3DXMatrixMultiply(&world, &moveableObj->AnimationTransforms[i], &world);
			else
				D3DXMatrixMultiply(&world, &moveableObj->BindPoseTransforms[i], &world);

			m_device->SetStreamSource(0, bucket->VertexBuffer, 0, sizeof(RendererVertex));
			m_device->SetIndices(bucket->IndexBuffer);

			for (int iPass = 0; iPass < cPasses; iPass++)
			{
				m_shader->BeginPass(iPass);
				m_shader->SetMatrix(m_shader->GetParameterByName(NULL, "World"), &world);
				m_shader->CommitChanges();

				DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0,
					bucket->NumVertices, 0, bucket->NumIndices / 3);

				m_shader->EndPass();
			}
		}
	}*/

	return 1;
}

__int32 Renderer::DrawPickup(__int16 objectNum)
{
	DrawObjectOn2DPosition(600 + PickupX, 450, objectNum, 0, m_pickupRotation, 0);
	m_pickupRotation += 45 * 360 / 30;
	return 0;
}

__int32 Renderer::SyncRenderer()
{
	// Sync the renderer
	__int32 nf = Sync();
	if (nf < 2)
	{
		__int32 i = 2 - nf;
		nf = 2;
		do
		{
			while (!Sync());
			i--;
		} while (i);
	}

	GnFrameCounter++;
	return nf;
}

__int32	Renderer::DrawLoadGameMenu(__int32 selectedIndex, bool resetBlink)
{
	char buffer[128];

	// Load all savegames
	LoadSavegameInfos();
	
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = ScreenWidth;
	rect.bottom = ScreenHeight;

	m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	m_device->BeginScene();

	m_sprite->Begin(0);
	m_sprite->Draw(m_renderTarget->GetTexture(), &rect, &D3DXVECTOR3(0.0f, 0.0f, 0.0f), &D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DCOLOR_ARGB(255, 64, 64, 64));
	m_sprite->End();

	PrintString(400, 20, (char*)"Load game", D3DCOLOR_ARGB(255, 216, 117, 49), PRINTSTRING_CENTER);

	__int16 lastY = 44;

	for (__int32 i = 0; i < MAX_SAVEGAMES; i++)
	{
		if (!g_SavegameInfos[i].present)
			PrintString(400, lastY, (char*)"Not saved", D3DCOLOR_ARGB(255, 255, 255, 255), 
						PRINTSTRING_CENTER | (selectedIndex == i ? PRINTSTRING_BLINK : 0));
		else
		{
			sprintf(buffer, "%05d", g_SavegameInfos[i].saveNumber);
			PrintString(20, lastY, buffer, D3DCOLOR_ARGB(255, 255, 255, 255), (selectedIndex == i ? PRINTSTRING_BLINK | PRINTSTRING_DONT_UPDATE_BLINK : 0));

			PrintString(100, lastY, g_SavegameInfos[i].levelName, D3DCOLOR_ARGB(255, 255, 255, 255), (selectedIndex == i ? PRINTSTRING_BLINK | PRINTSTRING_DONT_UPDATE_BLINK : 0));

			sprintf(buffer, "%02d days %02d:%02d:%02d", g_SavegameInfos[i].days, g_SavegameInfos[i].hours, g_SavegameInfos[i].minutes, g_SavegameInfos[i].seconds);
			PrintString(600, lastY, buffer, D3DCOLOR_ARGB(255, 255, 255, 255), (selectedIndex == i ? PRINTSTRING_BLINK : 0));
		}

		lastY += 24;
	}

	m_device->EndScene();
	m_device->Present(NULL, NULL, NULL, NULL);

	return 0;
}

__int32	Renderer::DrawSaveGameMenu(__int32 selectedIndex, bool resetBlink)
{
	return 0;
}

__int32 Renderer::DrawInventoryScene()
{
	char stringBuffer[255];

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = ScreenWidth;
	rect.bottom = ScreenHeight;

	// Clear screen
	m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	m_device->BeginScene();

	// Set basic render states
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	m_device->SetRenderState(D3DRS_ALPHATESTENABLE, false);
	
	byte colorComponent = 1.0f; // (m_fadeTimer < 15.0f ? 255.0f / (15.0f - min(m_fadeTimer, 15.0f)) : 255);

	// Draw the full screen background
	if (g_Inventory->GetType() == INV_TYPE_TITLE)
	{
		// Scale matrix for drawing full screen background
		D3DXMatrixScaling(&m_tempScale, ScreenWidth / 640.0f, ScreenHeight / 480.0f, 0.0f);

		m_sprite->Begin(0);
		m_sprite->SetTransform(&m_tempScale);
		m_sprite->Draw(m_titleScreen, &rect, &D3DXVECTOR3(0.0f, 0.0f, 0.0f), &D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DCOLOR_ARGB(255, colorComponent, colorComponent, colorComponent));
		m_sprite->End();
	}
	else
	{
		D3DXMatrixScaling(&m_tempScale, 1.0f, 1.0f, 1.0f);

		m_sprite->Begin(0);
		m_sprite->SetTransform(&m_tempScale);
		m_sprite->Draw(m_renderTarget->GetTexture(), &rect, &D3DXVECTOR3(0.0f, 0.0f, 0.0f), &D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DCOLOR_ARGB(255, 64, 64, 64));
		m_sprite->End();
	} 

	// Clear the Z-Buffer after drawing the background
	m_device->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);

	UINT cPasses = 1;
	LPD3DXEFFECT effect = m_shaderBasic->GetEffect();
	effect->Begin(&cPasses, 0);

	effect->SetTexture(effect->GetParameterByName(NULL, "ModelTexture"), m_textureAtlas);
	effect->SetMatrix(effect->GetParameterByName(NULL, "View"), &m_tempView);
	effect->SetMatrix(effect->GetParameterByName(NULL, "Projection"), &m_tempProjection);
	
	// Setup a nice directional light
	effect->SetVector(effect->GetParameterByName(NULL, "LightDirection"), &D3DXVECTOR4(-1.0f, 0.707f, -1.0f, 1.0f));
	effect->SetVector(effect->GetParameterByName(NULL, "LightColor"), &D3DXVECTOR4(1.0f, 1.0f, 0.0f, 1.0f));
	effect->SetFloat(effect->GetParameterByName(NULL, "LightIntensity"), 0.8f);
	effect->SetVector(effect->GetParameterByName(NULL, "AmbientLight"), &D3DXVECTOR4(0.5f, 0.5f, 0.5f, 1.0f));
	
	__int32 activeRing = g_Inventory->GetActiveRing();
	__int32 lastRing = 0;
	for (__int32 k = 0; k < 3; k++)
	{
		InventoryRing* ring = g_Inventory->GetRing(k);
		if (!ring->draw || ring->numObjects == 0)
			continue;

		// Inventory camera
		if (k == g_Inventory->GetActiveRing())
		{
			float cameraY = -384.0f + g_Inventory->GetVerticalOffset() + lastRing * INV_RINGS_OFFSET;
			float targetY = g_Inventory->GetVerticalOffset() + lastRing * INV_RINGS_OFFSET;

			D3DXMatrixLookAtRH(&m_tempView, &D3DXVECTOR3(3072.0f, cameraY, 0.0f), &D3DXVECTOR3(0.0f, targetY, 0.0f), &D3DXVECTOR3(0.0f, -1.0f, 0.0f));
			D3DXMatrixPerspectiveFovRH(&m_tempProjection, 80.0f * RADIAN, g_Renderer->ScreenWidth / (float)g_Renderer->ScreenHeight, 1.0f, 200000.0f);
		}

		// Setup the GPU
		m_device->SetVertexDeclaration(m_vertexDeclaration);
	
		// Fade timer
		//if (m_fadeTimer < 15)
		//	m_shader->SetFloat(m_shader->GetParameterByName(NULL, "FadeTimer"), m_fadeTimer++);

		__int16 numObjects = ring->numObjects;
		float deltaAngle = 360.0f / numObjects;
		__int32 objectIndex = 0;
		objectIndex = ring->currentObject;

		for (__int32 i = 0; i < numObjects; i++)
		{
			__int16 objectNumber = g_Inventory->GetInventoryObject(ring->objects[objectIndex].inventoryObject)->objectNumber;

			// Calculate the inventory object position and rotation
			float currentAngle = 0.0f;
			__int16 steps = -objectIndex + ring->currentObject;
			if (steps < 0) steps += numObjects;
			currentAngle = steps * deltaAngle;
			currentAngle += ring->movement;

			if (ring->focusState == INV_FOCUS_STATE_NONE && k == g_Inventory->GetActiveRing())
			{
				if (objectIndex == ring->currentObject)
					ring->objects[objectIndex].rotation += 45 * 360 / 30;
				else if (ring->objects[objectIndex].rotation != 0)
					ring->objects[objectIndex].rotation += 45 * 360 / 30;
			}
			else if (ring->focusState != INV_FOCUS_STATE_POPUP && ring->focusState != INV_FOCUS_STATE_POPOVER)
				g_Inventory->GetRing(k)->objects[objectIndex].rotation = 0;

			if (ring->objects[objectIndex].rotation > 65536.0f)
				ring->objects[objectIndex].rotation = 0;

			__int32 x = 2048.0f * cos(currentAngle * RADIAN);
			__int32 z = 2048.0f * sin(currentAngle * RADIAN);
			__int32 y = lastRing * INV_RINGS_OFFSET;

			// Prepare the object transform
			D3DXMatrixScaling(&m_tempScale, ring->objects[objectIndex].scale, ring->objects[objectIndex].scale, ring->objects[objectIndex].scale);
			D3DXMatrixTranslation(&m_tempTranslation, x, y, z);
			D3DXMatrixRotationY(&m_tempRotation, TR_ANGLE_TO_RAD(ring->objects[objectIndex].rotation + 16384));
			D3DXMatrixMultiply(&m_tempTransform, &m_tempScale, &m_tempRotation);
			D3DXMatrixMultiply(&m_tempTransform, &m_tempTransform, &m_tempTranslation);

			OBJECT_INFO* obj = &Objects[objectNumber];
			RendererObject* moveableObj = m_moveableObjects[objectNumber];

			// Build the object animation matrices
			if (ring->focusState == INV_FOCUS_STATE_FOCUSED && obj->animIndex != -1 && 
				objectIndex == ring->currentObject && k == g_Inventory->GetActiveRing())
			{
				__int16* framePtr[2];
				__int32 rate = 0;
				GetFrame(obj->animIndex, ring->frameIndex, framePtr, &rate);
				BuildAnimationPose(moveableObj, framePtr, 0, 1, 0xFFFFFFFF);
			}
			else
			{
				if (obj->animIndex != -1)
					BuildAnimationPose(moveableObj, &Anims[obj->animIndex].framePtr, 0, 1, 0xFFFFFFFF);
			}

			for (__int32 n = 0; n < moveableObj->ObjectMeshes.size(); n++)
			{
				RendererMesh* mesh = moveableObj->ObjectMeshes[n];

				// Finish the world matrix
				if (obj->animIndex != -1)
					D3DXMatrixMultiply(&m_tempWorld, &moveableObj->AnimationTransforms[n], &m_tempTransform);
				else
					D3DXMatrixMultiply(&m_tempWorld, &moveableObj->BindPoseTransforms[n], &m_tempTransform);
				effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &m_tempWorld);

				for (__int32 m = 0; m < NUM_BUCKETS; m++)
				{
					RendererBucket* bucket = mesh->GetBucket(m);
					if (bucket->NumVertices == 0)
						continue;

					m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
					m_device->SetIndices(bucket->GetIndexBuffer());

					for (int iPass = 0; iPass < cPasses; iPass++)
					{
						effect->BeginPass(iPass);
						effect->CommitChanges();

						DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0,
							bucket->NumVertices, 0, bucket->NumIndices / 3);

						effect->EndPass();
					}
				}
			}

			__int16 inventoryItem = ring->objects[objectIndex].inventoryObject;

			// Draw special stuff if needed
			if (objectIndex == ring->currentObject && k == g_Inventory->GetActiveRing())
			{
				if (g_Inventory->GetActiveRing() == INV_RING_OPTIONS)
				{
					if (inventoryItem == INV_OBJECT_PASSAPORT && ring->focusState == INV_FOCUS_STATE_FOCUSED)
					{
						// Draw savegames menu
						if (ring->passportAction == INV_WHAT_PASSPORT_LOAD_GAME || ring->passportAction == INV_WHAT_PASSPORT_SAVE_GAME)
						{
							__int16 lastY = 44;

							for (__int32 n = 0; n < MAX_SAVEGAMES; n++)
							{
								if (!g_SavegameInfos[i].present)
									PrintString(400, lastY, (char*)"Not saved", D3DCOLOR_ARGB(255, 255, 255, 255),
										PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == n ? PRINTSTRING_BLINK : 0));
								else
								{
									sprintf(stringBuffer, "%05d", g_SavegameInfos[n].saveNumber);
									PrintString(20, lastY, stringBuffer, D3DCOLOR_ARGB(255, 255, 255, 255), PRINTSTRING_OUTLINE |
										(ring->selectedIndex == n ? PRINTSTRING_BLINK | PRINTSTRING_DONT_UPDATE_BLINK : 0));

									PrintString(100, lastY, g_SavegameInfos[n].levelName, D3DCOLOR_ARGB(255, 255, 255, 255), PRINTSTRING_OUTLINE |
										(ring->selectedIndex == n ? PRINTSTRING_BLINK | PRINTSTRING_DONT_UPDATE_BLINK : 0));

									sprintf(stringBuffer, "%02d days %02d:%02d:%02d", g_SavegameInfos[n].days, g_SavegameInfos[n].hours, g_SavegameInfos[n].minutes, g_SavegameInfos[n].seconds);
									PrintString(600, lastY, stringBuffer, D3DCOLOR_ARGB(255, 255, 255, 255),
										PRINTSTRING_OUTLINE | (ring->selectedIndex == n ? PRINTSTRING_BLINK : 0));
								}

								lastY += 24;
							}
						}
						char* string = (char*)g_NewStrings[0].c_str();
						switch (ring->passportAction)
						{
						case INV_WHAT_PASSPORT_NEW_GAME:
							string = (char*)g_NewStrings[STRING_INV_NEW_GAME].c_str();
							break;
						case INV_WHAT_PASSPORT_LOAD_GAME:
							string = (char*)g_NewStrings[STRING_INV_LOAD_GAME].c_str();
							break;
						case INV_WHAT_PASSPORT_SAVE_GAME:
							string = (char*)g_NewStrings[STRING_INV_SAVE_GAME].c_str();
							break;
						case INV_WHAT_PASSPORT_EXIT_GAME:
							string = (char*)g_NewStrings[STRING_INV_EXIT_GAME].c_str();
							break;
						case INV_WHAT_PASSPORT_EXIT_TO_TITLE:
							string = (char*)g_NewStrings[STRING_INV_EXIT_TO_TITLE].c_str();
							break;
						}

						PrintString(400, 550, string, PRINTSTRING_COLOR_ORANGE, PRINTSTRING_CENTER | PRINTSTRING_OUTLINE);
					}
					else
					{
						// Draw the description below the object
						char* string = (char*)g_NewStrings[g_Inventory->GetInventoryObject(inventoryItem)->objectName].c_str(); // &AllStrings[AllStringsOffsets[g_Inventory->GetInventoryObject(inventoryItem)->objectName]];
						PrintString(400, 550, string, PRINTSTRING_COLOR_ORANGE, PRINTSTRING_CENTER | PRINTSTRING_OUTLINE);
					}
				}
				else
				{
					__int16 inventoryItem = g_Inventory->GetRing(k)->objects[objectIndex].inventoryObject;
					char* string = &AllStrings[AllStringsOffsets[InventoryObjectsList[inventoryItem].objectName]];

					__int32 quantity = -1;
					switch (objectNumber)
					{
					case ID_BIGMEDI_ITEM:
						quantity = Lara.numLargeMedipack;
						break;
					case ID_SMALLMEDI_ITEM:
						quantity = Lara.numSmallMedipack;
						break;
					case ID_FLARE_INV_ITEM:
						quantity = Lara.numFlares;
						break;
					case ID_SHOTGUN_AMMO1_ITEM:
						quantity = Lara.numShotgunAmmo1;
						if (quantity != -1)
							quantity /= 6;
						break;
					case ID_SHOTGUN_AMMO2_ITEM:
						quantity = Lara.numShotgunAmmo2;
						if (quantity != -1)
							quantity /= 6;
						break;
					case ID_HK_AMMO_ITEM:
						quantity = Lara.numHKammo1;
						break;
					case ID_CROSSBOW_AMMO1_ITEM:
						quantity = Lara.numCrossbowAmmo1;
						break;
					case ID_CROSSBOW_AMMO2_ITEM:
						quantity = Lara.numCrossbowAmmo2;
						break;
					case ID_REVOLVER_AMMO_ITEM:
						quantity = Lara.numRevolverAmmo;
						break;
					case ID_UZI_AMMO_ITEM:
						quantity = Lara.numUziAmmo;
						break;
					case ID_BOTTLE:
						quantity = Lara.bottle;
						break;
					case ID_PICKUP_ITEM4:
						quantity = Savegame.Level.Secrets;
						break;
					default:
						if (objectNumber >= ID_PUZZLE_ITEM1 && objectNumber <= ID_PUZZLE_ITEM8)
							quantity = Lara.puzzleItems[objectNumber - ID_PUZZLE_ITEM1];
						else if (objectNumber >= ID_PUZZLE_ITEM1_COMBO1 && objectNumber <= ID_PUZZLE_ITEM8_COMBO2)
							quantity = (Lara.puzzleItemsCombo >> (objectNumber - ID_PUZZLE_ITEM1_COMBO1)) & 1;
						else if (objectNumber >= ID_KEY_ITEM1 && objectNumber <= ID_KEY_ITEM8)
							quantity = (Lara.keyItems >> (objectNumber - ID_KEY_ITEM1)) & 1;
						else if (objectNumber >= ID_KEY_ITEM1_COMBO1 && objectNumber <= ID_KEY_ITEM8_COMBO2)
							quantity = (Lara.keyItemsCombo >> (objectNumber - ID_KEY_ITEM1_COMBO1)) & 1;
						else if (objectNumber >= ID_PICKUP_ITEM1 && objectNumber <= ID_PICKUP_ITEM3)
							quantity = (Lara.pickupItems >> (objectNumber - ID_PICKUP_ITEM1)) & 1;
						else if (objectNumber >= ID_PICKUP_ITEM1_COMBO1 && objectNumber <= ID_PICKUP_ITEM3_COMBO2)
							quantity = (Lara.pickupItemsCombo >> (objectNumber - ID_PICKUP_ITEM1_COMBO1)) & 1;
						else if (objectNumber == ID_EXAMINE1)
							quantity = Lara.examine1;
						else if (objectNumber == ID_EXAMINE2)
							quantity = Lara.examine2;
						else if (objectNumber == ID_EXAMINE3)
							quantity = Lara.examine3;
					}

					if (quantity < 1)
						PrintString(400, 550, string, D3DCOLOR_ARGB(255, 216, 117, 49), PRINTSTRING_CENTER);
					else
					{
						sprintf(stringBuffer, "%d x %s", quantity, string);
						PrintString(400, 550, stringBuffer, D3DCOLOR_ARGB(255, 216, 117, 49), PRINTSTRING_CENTER);
					}
				}
			}

			objectIndex++;
			if (objectIndex == numObjects) objectIndex = 0;
		}

		lastRing++;
	}

	effect->End();

	m_device->EndScene();
	m_device->Present(NULL, NULL, NULL, NULL);

	return 0;
}

__int32 Renderer::GetFrame(__int16 animation, __int16 frame, __int16** framePtr, __int32* rate)
{
	ITEM_INFO item;
	item.animNumber = animation;
	item.frameNumber = frame;

	return GetFrame_D2(&item, framePtr, rate);
}

void Renderer::UpdateGunFlashes()
{
	/*RendererObject* laraObj = m_moveableObjects[ID_LARA];

	__int32 numArms = 0;
	D3DXVECTOR4 flashPosition = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f);

	if (Lara.leftArm.flash_gun)
	{
		numArms++;

		// Get Lara left joint absolute position
		D3DXVECTOR4 handPos;
		D3DXMATRIX world;
		D3DXMatrixMultiply(&world, &laraObj->AnimationTransforms[HAND_L], &m_LaraWorldMatrix);
		D3DXVec3Transform(&handPos, &laraObj->LinearizedBones[HAND_L]->GlobalTranslation, &world);

		flashPosition += handPos;
	}

	if (Lara.rightArm.flash_gun)
	{
		numArms++;

		// Get Lara right joint absolute position
		D3DXVECTOR4 handPos;
		D3DXMATRIX world;
		D3DXMatrixMultiply(&world, &laraObj->AnimationTransforms[HAND_R], &m_LaraWorldMatrix);
		D3DXVec3Transform(&handPos, &laraObj->LinearizedBones[HAND_R]->GlobalTranslation, &world);

		flashPosition += handPos;
	}

	if (numArms != 0)
	{
		RendererLight* flashLight = new RendererLight();

		flashLight->Position = flashPosition / numArms;
		flashLight->Color = D3DXVECTOR4(1.0f, 1.0f, 0.0f, 1.0f);
		flashLight->Out = 3072.0f;
		flashLight->Type = LIGHT_TYPES::LIGHT_TYPE_POINT;
		flashLight->Dynamic = true;
		flashLight->Intensity = 2.0f;

		m_dynamicLights.push_back(flashLight);
	}*/
}

void Renderer::UpdateFlares()
{
	// Flare in Lara's hand
	/*if (Lara.flareAge > 0)
	{
		RendererObject* laraObj = m_moveableObjects[ID_LARA];

		D3DXVECTOR4 handPos;
		D3DXMATRIX world;
		D3DXMatrixMultiply(&world, &laraObj->AnimationTransforms[HAND_L], &m_LaraWorldMatrix);
		D3DXVec3Transform(&handPos, &laraObj->LinearizedBones[HAND_L]->GlobalTranslation, &world);

		RendererLight* flareLight = new RendererLight();

		flareLight->Position = handPos;
		flareLight->Color = D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f);
		flareLight->Out = 3072.0f;
		flareLight->Type = LIGHT_TYPES::LIGHT_TYPE_POINT;
		flareLight->Dynamic = true;
		flareLight->Intensity = 2.0f;

		m_dynamicLights.push_back(flareLight);
	}

	// Flares throwed away
	for (__int32 i = 0; i < m_itemsToDraw.size(); i++)
	{
		RendererItemToDraw* itemToDraw = m_itemsToDraw[i];
		ITEM_INFO* item = itemToDraw->Item;
		if (item->objectNumber != ID_FLARE_ITEM)
			continue;

		RendererLight* flareLight = new RendererLight();

		flareLight->Position = D3DXVECTOR4(item->pos.xPos, item->pos.yPos - 128.0f, item->pos.zPos, 1.0f);
		flareLight->Color = D3DXVECTOR4(0.0f, 1.0f, 0.0f, 1.0f);
		flareLight->Out = 3072.0f;
		flareLight->Type = LIGHT_TYPES::LIGHT_TYPE_POINT;
		flareLight->Dynamic = true;
		flareLight->Intensity = 2.0f;

		m_dynamicLights.push_back(flareLight);
	}*/
}

void Renderer::UpdateFires()
{
	/*for (__int32 i = 0; i < 32; i++)
	{
		FIRE_LIST* fire = &Fires[i];

		// Is the fire active and visible?
		if (!fire->on)
			continue;

		RendererLight* flareLight = new RendererLight();

		flareLight->Position = D3DXVECTOR4(fire->x, fire->y, fire->z, 1.0f);
		flareLight->Color = D3DXVECTOR4(1.0f, 1.0f, 0.0f, 1.0f);
		flareLight->Out = 2048.0f;
		flareLight->Type = LIGHT_TYPES::LIGHT_TYPE_POINT;
		flareLight->Dynamic = true;
		flareLight->Intensity = 2.0f;

		m_dynamicLights.push_back(flareLight);		
	}*/

	for (__int32 i = 0; i < 64; i++)
	{
		DYNAMIC* dynamic = &Dynamics[i];
		if (dynamic->on)
		{
			RendererLight* flareLight = new RendererLight();

			flareLight->Position = D3DXVECTOR4(dynamic->x, dynamic->y, dynamic->z, 1.0f);
			flareLight->Color = D3DXVECTOR4(dynamic->r / 255.0f, dynamic->g / 255.0f, dynamic->b / 255.0f, 1.0f);
			flareLight->Out = dynamic->falloff;
			flareLight->Type = LIGHT_TYPES::LIGHT_TYPE_POINT;
			flareLight->Dynamic = true;
			flareLight->Intensity = 1.0f;

			m_dynamicLights.push_back(flareLight);
		}
	}
}

bool Renderer::DrawSceneLightPrePass(bool dump)
{
	// HACK:
	LaraItem->hitPoints = 1000;

	m_timeClearGBuffer = 0;
	m_timePrepareShadowMap = 0;
	m_timeFillGBuffer = 0;
	m_timeLight = 0;
	m_timeCombine = 0;
	m_timeUpdate = 0;
	m_numDrawCalls = 0;
	m_numTriangles = 0;
	m_numVertices = 0;
	m_timeDrawScene = 0;
	m_timeReconstructZBuffer = 0;

	LPD3DXEFFECT effect;
	UINT cPasses = 1;

	auto time1 = chrono::high_resolution_clock::now();
	auto timeScene1 = chrono::high_resolution_clock::now();

	D3DXMatrixMultiply(&m_viewProjection, &ViewMatrix, &ProjectionMatrix);
	D3DXMatrixInverse(&m_inverseViewProjection, NULL, &m_viewProjection);

	CollectRooms();
	CollectItems();
	CollectLightsLPP();

	UpdateLaraAnimations();
	UpdateItemsAnimations();

	auto time2 = chrono::high_resolution_clock::now();
	m_timeUpdate = (chrono::duration_cast<ns>(time2 - time1)).count();
	time1 = time2;

	PrepareShadowMaps();

	time2 = chrono::high_resolution_clock::now();
	m_timePrepareShadowMap = (chrono::duration_cast<ns>(time2 - time1)).count();
	time1 = time2;

	// Clear the G-Buffer
	BindRenderTargets(m_colorBuffer, m_normalBuffer, m_depthBuffer, m_vertexLightBuffer);

	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	m_device->SetVertexDeclaration(m_vertexDeclaration);

	effect = m_shaderClearGBuffer->GetEffect();
	m_device->BeginScene();
	effect->Begin(&cPasses, 0);
	effect->BeginPass(0);
	effect->CommitChanges();
	m_device->DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 6, 2, m_quadIndices, D3DFORMAT::D3DFMT_INDEX32,
		m_quadVertices, sizeof(RendererVertex));
	effect->EndPass();
	effect->End();
	m_device->EndScene();

	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	time2 = chrono::high_resolution_clock::now();
	m_timeClearGBuffer = (chrono::duration_cast<ns>(time2 - time1)).count();
	time1 = time2;

	// Fill the G-Buffer
	effect = m_shaderFillGBuffer->GetEffect();
	m_device->BeginScene();
	effect->Begin(&cPasses, 0);

	effect->SetMatrix(effect->GetParameterByName(NULL, "View"), &ViewMatrix);
	effect->SetMatrix(effect->GetParameterByName(NULL, "Projection"), &ProjectionMatrix);
	effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPES::MODEL_TYPE_ROOM);
	effect->SetTexture(effect->GetParameterByName(NULL, "TextureAtlas"), m_textureAtlas);
	effect->SetBool(effect->GetParameterByName(NULL, "UseSkinning"), false);
	effect->SetVector(effect->GetParameterByName(NULL, "Color"), &D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));

	D3DXMATRIX world;

	// Draw opaque geometry
	DrawSkyLPP();

	for (__int32 i = 0; i < m_roomsToDraw.size(); i++)
	{
		RendererRoom* room = m_rooms[m_roomsToDraw[i]];
		if (room == NULL)
			continue;

		DrawRoomLPP(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
		DrawRoomLPP(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
	
		// Draw static objects
		ROOM_INFO* r = room->Room;
		if (r->numMeshes != 0)
		{
			for (__int32 j = 0; j < r->numMeshes; j++)
			{
				MESH_INFO* sobj = &r->mesh[j];
				RendererObject* staticObj = m_staticObjects[sobj->staticNumber];
				RendererMesh* staticMesh = staticObj->ObjectMeshes[0];

				DrawStaticLPP(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
				DrawStaticLPP(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
			}
		}
	}

	DrawLaraLPP(RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
	DrawLaraLPP(RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
	
	for (__int32 i = 0; i < m_itemsToDraw.size(); i++)
	{
		RendererObject* obj = m_moveableObjects[Items[m_itemsToDraw[i]->Id].objectNumber];
		DrawItemLPP(m_itemsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
		DrawItemLPP(m_itemsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
	}

	DrawGunshells(RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
	DrawGunshells(RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER);

	// Draw alpha tested geometry
	for (__int32 i = 0; i < m_roomsToDraw.size(); i++)
	{
		RendererRoom* room = m_rooms[m_roomsToDraw[i]];
		if (room == NULL)
			continue;

		DrawRoomLPP(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
		DrawRoomLPP(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
	
		// Draw static objects
		ROOM_INFO* r = room->Room;
		if (r->numMeshes != 0)
		{
			for (__int32 j = 0; j < r->numMeshes; j++)
			{
				MESH_INFO* sobj = &r->mesh[j];
				RendererObject* staticObj = m_staticObjects[sobj->staticNumber];
				RendererMesh* staticMesh = staticObj->ObjectMeshes[0];

				DrawStaticLPP(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
				DrawStaticLPP(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
			}
		}
	}

	DrawLaraLPP(RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
	DrawLaraLPP(RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
	 
	for (__int32 i = 0; i < m_itemsToDraw.size(); i++)
	{
		DrawItemLPP(m_itemsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
		DrawItemLPP(m_itemsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
	}

	DrawGunshells(RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
	DrawGunshells(RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER);

	DrawGunFlashes(RENDERER_PASSES::RENDERER_PASS_GBUFFER);

	effect->EndPass();
	effect->End();
	m_device->EndScene();

	time2 = chrono::high_resolution_clock::now();
	m_timeFillGBuffer = (chrono::duration_cast<ns>(time2 - time1)).count();
	time1 = time2;

	// Reset the back-buffer
	RestoreBackBuffer();
	   
	// Bind the light target
	RestoreBackBuffer();
	BindRenderTargets(m_shadowBuffer, NULL, NULL, NULL);
	m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f), 1.0f, 0);
	m_device->BeginScene();
	m_device->EndScene();

	RestoreBackBuffer();
	BindRenderTargets(m_lightBuffer, NULL, NULL, NULL);
	
	effect = m_shaderLight->GetEffect();

	// Setup additive blending
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	m_device->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
	m_device->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
	m_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	m_device->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	  
	// Clear the screen and disable Z write
	m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, false);
	  
	m_device->BeginScene();
  	effect->Begin(&cPasses, 0); 

	effect->SetMatrix(effect->GetParameterByName(NULL, "View"), &ViewMatrix);
	effect->SetMatrix(effect->GetParameterByName(NULL, "Projection"), &ProjectionMatrix);
	effect->SetMatrix(effect->GetParameterByName(NULL, "ViewProjectionInverse"), &m_inverseViewProjection);
	          
	effect->SetTexture(effect->GetParameterByName(NULL, "ColorMap"), m_colorBuffer->GetTexture());
	effect->SetTexture(effect->GetParameterByName(NULL, "NormalMap"), m_normalBuffer->GetTexture());
	effect->SetTexture(effect->GetParameterByName(NULL, "DepthMap"), m_depthBuffer->GetTexture());
	  
	effect->SetBool(effect->GetParameterByName(NULL, "AmbientPass"), false);
	effect->SetVector(effect->GetParameterByName(NULL, "CameraPosition"), &D3DXVECTOR4(Camera.pos.x, Camera.pos.y, Camera.pos.z, 1.0f));
	effect->SetFloat(effect->GetParameterByName(NULL, "HalfPixelX"), m_halfPixelX);
	effect->SetFloat(effect->GetParameterByName(NULL, "HalfPixelY"), m_halfPixelY);
	  
	effect->SetInt(effect->GetParameterByName(NULL, "LightType"), LIGHT_TYPES::LIGHT_TYPE_POINT);
	 
	m_device->SetStreamSource(0, m_sphereMesh->VertexBuffer, 0, sizeof(RendererVertex));
	m_device->SetIndices(m_sphereMesh->IndexBuffer);
	 
	for (int j = 0; j < m_lights.size(); j++)
	{ 
		RendererLight* light = m_lights[j];

		for (int iPass = 0; iPass < cPasses; iPass++)
		{
			effect->BeginPass(iPass);
			  
			D3DXMATRIX translation;
			D3DXMATRIX scale;

			/*if (light == m_shadowLight)
			{
				effect->SetBool(effect->GetParameterByName(NULL, "CastShadows"), true);
				effect->SetMatrix(effect->GetParameterByName(NULL, "LightView"), &m_lightView);
				effect->SetMatrix(effect->GetParameterByName(NULL, "LightProjection"), &m_lightProjection);
				//effect->SetTexture(effect->GetParameterByName(NULL, "ShadowMapCube"), m_shadowMapCube->GetTexture());
				effect->SetTexture(effect->GetParameterByName(NULL, "ShadowMap"), m_shadowMap->GetTexture());
			}
			else
			{*/
				effect->SetBool(effect->GetParameterByName(NULL, "CastShadows"), false);
			//}

			effect->SetBool(effect->GetParameterByName(NULL, "AmbientPass"), false);
			effect->SetBool(effect->GetParameterByName(NULL, "LightDynamic"), light->Dynamic);
			effect->SetVector(effect->GetParameterByName(NULL, "LightPosition"), &light->Position);
			effect->SetFloat(effect->GetParameterByName(NULL, "LightIntensity"), light->Intensity);
			effect->SetFloat(effect->GetParameterByName(NULL, "LightIn"), light->In);
			effect->SetVector(effect->GetParameterByName(NULL, "LightColor"), &light->Color);
			effect->SetFloat(effect->GetParameterByName(NULL, "LightOut"), light->Out);
			D3DXMatrixTranslation(&translation, light->Position.x, light->Position.y, light->Position.z);
			D3DXMatrixScaling(&scale, light->Out / 1024.0f, light->Out / 1024.0f, light->Out / 1024.0f);
			D3DXMatrixMultiply(&world, &scale, &translation);

			effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &world);
			effect->CommitChanges();

			DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, m_sphereMesh->NumVertices, 0, m_sphereMesh->NumIndices / 3);

			effect->EndPass();
		}
	}

	effect->End();
	m_device->EndScene();

	time2 = chrono::high_resolution_clock::now();
	m_timeLight = (chrono::duration_cast<ns>(time2 - time1)).count();
	time1 = time2;

	RestoreBackBuffer();
	         
	if (dump)
		BindRenderTargets(m_renderTarget, NULL, NULL, NULL);

	// Combine stage
	//BindRenderTargets(m_postprocessBuffer, NULL, NULL, NULL);

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_device->SetRenderState(D3DRS_ZWRITEENABLE, true);

	effect = m_shaderCombine->GetEffect();
	m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	m_device->BeginScene();
	effect->Begin(&cPasses, 0);

	effect->SetTexture(effect->GetParameterByName(NULL, "ColorMap"), m_colorBuffer->GetTexture());
	effect->SetTexture(effect->GetParameterByName(NULL, "LightMap"), m_lightBuffer->GetTexture());
	effect->SetTexture(effect->GetParameterByName(NULL, "VertexColorMap"), m_vertexLightBuffer->GetTexture());
	effect->SetTexture(effect->GetParameterByName(NULL, "NormalMap"), m_normalBuffer->GetTexture());
	effect->SetFloat(effect->GetParameterByName(NULL, "HalfPixelX"), m_halfPixelX);
	effect->SetFloat(effect->GetParameterByName(NULL, "HalfPixelY"), m_halfPixelY);
	     
	if (m_shadowLight != NULL)
	{ 
		effect->SetMatrix(effect->GetParameterByName(NULL, "ViewProjectionInverse"), &m_inverseViewProjection);
		effect->SetBool(effect->GetParameterByName(NULL, "CastShadows"), true);
		effect->SetMatrix(effect->GetParameterByName(NULL, "LightView"), &m_lightView);
		effect->SetMatrix(effect->GetParameterByName(NULL, "LightProjection"), &m_lightProjection);
		effect->SetVector(effect->GetParameterByName(NULL, "LightPosition"), &m_shadowLight->Position);
		effect->SetFloat(effect->GetParameterByName(NULL, "LightOut"), m_shadowLight->Out);
		effect->SetTexture(effect->GetParameterByName(NULL, "ShadowMap"), m_shadowMap->GetTexture());
		effect->SetTexture(effect->GetParameterByName(NULL, "DepthMap"), m_depthBuffer->GetTexture());
	}
	else
	{
		effect->SetBool(effect->GetParameterByName(NULL, "CastShadows"), false);
	}

	effect->BeginPass(0);
	effect->CommitChanges();
	m_device->DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 6, 2, m_quadIndices, D3DFORMAT::D3DFMT_INDEX32,
		m_quadVertices, sizeof(RendererVertex));
	effect->EndPass();
	effect->End();
	m_device->EndScene();

	time2 = chrono::high_resolution_clock::now();
	m_timeCombine = (chrono::duration_cast<ns>(time2 - time1)).count();
	time1 = time2;

	// Clear depth and start reconstructing Z-Buffer
	m_device->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0xFFFFFFFF, 1.0f, 0);

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	m_device->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
	m_device->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
	m_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	m_device->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	 
	effect = m_shaderReconstructZBuffer->GetEffect();
	m_device->BeginScene();
	effect->Begin(&cPasses, 0);

	effect->SetMatrix(effect->GetParameterByName(NULL, "View"), &ViewMatrix);
	effect->SetMatrix(effect->GetParameterByName(NULL, "Projection"), &ProjectionMatrix);

	for (__int32 i = 0; i < m_roomsToDraw.size(); i++)
	{
		RendererRoom* room = m_rooms[m_roomsToDraw[i]];
		if (room == NULL)
			continue;

		DrawRoomLPP(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
		DrawRoomLPP(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);

		// Draw static objects
		ROOM_INFO* r = room->Room;
		if (r->numMeshes != 0)
		{
			for (__int32 j = 0; j < r->numMeshes; j++)
			{
				MESH_INFO* sobj = &r->mesh[j];
				RendererObject* staticObj = m_staticObjects[sobj->staticNumber];
				RendererMesh* staticMesh = staticObj->ObjectMeshes[0];

				DrawStaticLPP(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
				DrawStaticLPP(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
			}
		}
	}

	DrawLaraLPP(RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
	DrawLaraLPP(RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);

	for (__int32 i = 0; i < m_itemsToDraw.size(); i++)
	{
		DrawItemLPP(m_itemsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
		DrawItemLPP(m_itemsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
	}

	// Draw alpha tested geometry
	for (__int32 i = 0; i < m_roomsToDraw.size(); i++)
	{
		RendererRoom* room = m_rooms[m_roomsToDraw[i]];
		if (room == NULL)
			continue;

		DrawRoomLPP(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
		DrawRoomLPP(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);

		// Draw static objects
		ROOM_INFO* r = room->Room;
		if (r->numMeshes != 0)
		{
			for (__int32 j = 0; j < r->numMeshes; j++)
			{
				MESH_INFO* sobj = &r->mesh[j];
				RendererObject* staticObj = m_staticObjects[sobj->staticNumber];
				RendererMesh* staticMesh = staticObj->ObjectMeshes[0];

				DrawStaticLPP(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
				DrawStaticLPP(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
			}
		}
	}

	DrawLaraLPP(RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
	DrawLaraLPP(RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);

	for (__int32 i = 0; i < m_itemsToDraw.size(); i++)
	{
		DrawItemLPP(m_itemsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
		DrawItemLPP(m_itemsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
	}

	m_device->EndScene();
	effect->End();

	/*m_device->SetRenderState(D3DRS_COLORWRITEENABLE, true);
	m_device->SetRenderState(D3DRS_COLORWRITEENABLE1, true);
	m_device->SetRenderState(D3DRS_COLORWRITEENABLE2, true);
	m_device->SetRenderState(D3DRS_COLORWRITEENABLE3, true);*/

	// Draw sprites
	m_spritesVertices.clear();
	m_spritesIndices.clear();
	
	DrawFires();
	DrawSmokes();
	DrawBlood();

	if (WeatherType == WEATHER_TYPES::WEATHER_RAIN)
	//	DoSnow(); 
		DoRain();

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
	m_device->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
	m_device->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
	m_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	m_device->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	effect = m_shaderSprites->GetEffect();
	m_device->BeginScene();
	effect->Begin(&cPasses, 0);

	effect->SetMatrix(effect->GetParameterByName(NULL, "View"), &ViewMatrix);
	effect->SetMatrix(effect->GetParameterByName(NULL, "Projection"), &ProjectionMatrix);
	effect->SetTexture(effect->GetParameterByName(NULL, "TextureAtlas"), m_textureAtlas);
	effect->BeginPass(0);
	effect->CommitChanges();

	__int32 numSpriteBuckets = m_spritesIndices.size() / 288;
	if (m_spritesIndices.size() % 288 != 0) numSpriteBuckets++;

	/*for (__int32 i = 0; i < numSpriteBuckets; i++)
	{
		__int32 numSprites = 48;
		__int32 numSpriteVertices = (i == numSpriteBuckets - 1 ? m_spritesVertices.size() - 192 * i : 192);
		__int32 numSpriteIndices = (i == numSpriteBuckets - 1 ? m_spritesIndices.size() - 288 * i : 288);
		*/
		m_device->DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, m_spritesVertices.size(),
			m_spritesIndices.size() / 3, m_spritesIndices.data(),
			D3DFORMAT::D3DFMT_INDEX32, m_spritesVertices.data(), sizeof(RendererVertex));
	//}

	effect->EndPass();
	effect->End();
	m_device->EndScene();
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

	time2 = chrono::high_resolution_clock::now();
	m_timeReconstructZBuffer = (chrono::duration_cast<ns>(time2 - time1)).count();
	time1 = time2;

	auto timeScene2 = chrono::high_resolution_clock::now();
	m_timeDrawScene = (chrono::duration_cast<ns>(timeScene2 - timeScene1)).count() / 1000;

	if (!dump)
	{
		DrawDebugInfo();
		m_device->Present(NULL, NULL, NULL, NULL);
	}
	else
		RestoreBackBuffer();

	/*m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFFFFFFFF, 1.0f, 0);
	m_device->BeginScene();

	m_sprite->Begin(0);
	m_sprite->Draw(m_depthBuffer->GetTexture(), NULL, &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(0, 0, 0), 0xFFFFFFFF);
	m_sprite->End();

	m_device->EndScene();
	m_device->Present(NULL, NULL, NULL, NULL);*/

	return true;
}

bool Renderer::BindRenderTargets(RenderTarget2D* rt1, RenderTarget2D* rt2, RenderTarget2D* rt3, RenderTarget2D* rt4)
{
	HRESULT res;

	m_backBufferTarget = NULL;
	res = m_device->GetRenderTarget(0, &m_backBufferTarget);
	if (res != S_OK)
		return false;

	m_backBufferDepth = NULL;
	res = m_device->GetDepthStencilSurface(&m_backBufferDepth);
	if (res != S_OK)
		return false;

	if (rt1 != NULL)
	{
		LPDIRECT3DSURFACE9 surface;
		res = rt1->GetTexture()->GetSurfaceLevel(0, &surface);
		if (res != S_OK)
			return false;

		res = m_device->SetRenderTarget(0, surface);
		if (res != S_OK)
			return false;

		surface->Release();
	}

	if (rt2 != NULL)
	{
		LPDIRECT3DSURFACE9 surface;
		res = rt2->GetTexture()->GetSurfaceLevel(0, &surface);
		if (res != S_OK)
			return false;

		res = m_device->SetRenderTarget(1, surface);
		if (res != S_OK)
			return false;

		surface->Release();
	}

	if (rt3 != NULL)
	{
		LPDIRECT3DSURFACE9 surface;
		res = rt3->GetTexture()->GetSurfaceLevel(0, &surface);
		if (res != S_OK)
			return false;

		res = m_device->SetRenderTarget(2, surface);
		if (res != S_OK)
			return false;

		surface->Release();
	}

	if (rt4 != NULL)
	{
		LPDIRECT3DSURFACE9 surface;
		res = rt4->GetTexture()->GetSurfaceLevel(0, &surface);
		if (res != S_OK)
			return false;

		res = m_device->SetRenderTarget(3, surface);
		if (res != S_OK)
			return false;

		surface->Release();
	}

	return true;
}

bool Renderer::RestoreBackBuffer()
{
	m_device->SetRenderTarget(0, m_backBufferTarget);
	m_device->SetRenderTarget(1, NULL);
	m_device->SetRenderTarget(2, NULL);
	m_device->SetRenderTarget(3, NULL);

	m_device->SetDepthStencilSurface(m_backBufferDepth);

	return true;
}

bool Renderer::DrawLaraLPP(RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass)
{
	D3DXMATRIX world;
	D3DXMATRIX translation;
	D3DXMATRIX rotation;
	UINT cPasses = 1;

	RendererObject* laraObj = m_moveableObjects[ID_LARA];
	RendererObject* laraSkin = m_moveableObjects[ID_LARA_SKIN];

	ITEM_INFO* item = LaraItem;
	OBJECT_INFO* obj = &Objects[0];

	RendererLightInfo* light = &m_itemsLightInfo[0];
	LPD3DXEFFECT effect;
	if (pass == RENDERER_PASSES::RENDERER_PASS_SHADOW_MAP)
		effect = m_depthShader->GetEffect();
	else if (pass == RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH)
		effect = m_shaderReconstructZBuffer->GetEffect();
	else  
		effect = m_shaderFillGBuffer->GetEffect();

	effect->SetBool(effect->GetParameterByName(NULL, "UseSkinning"), true);
	effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPES::MODEL_TYPE_LARA);
	effect->SetMatrixArray(effect->GetParameterByName(NULL, "Bones"), laraObj->AnimationTransforms, laraObj->ObjectMeshes.size());
	effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &m_LaraWorldMatrix);

	if (bucketIndex == RENDERER_BUCKETS::RENDERER_BUCKET_SOLID || bucketIndex == RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS)
		effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLEND_MODES::BLENDMODE_OPAQUE);
	else
		effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLEND_MODES::BLENDMODE_ALPHATEST);

	for (__int32 i = 0; i < laraObj->ObjectMeshes.size(); i++)
	{
		// Lara has meshes overriden by weapons, crowbar, etc
		RendererMesh* mesh = MeshPointersToMesh[Lara.meshPtrs[i]];
		RendererBucket* bucket = mesh->GetBucket(bucketIndex);

		if (bucket->NumVertices != 0)
		{
			// Bind buffers
			m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
			m_device->SetIndices(bucket->GetIndexBuffer());

			for (int iPass = 0; iPass < cPasses; iPass++)
			{
				effect->BeginPass(iPass);
				effect->CommitChanges();

				DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

				effect->EndPass();
			}
		}

		// Draw joints, if present
		if (m_moveableObjects.find(ID_LARA_SKIN_JOINTS) != m_moveableObjects.end())
		{
			RendererObject* laraSkinJoints = m_moveableObjects[ID_LARA_SKIN_JOINTS];
			RendererMesh* jointMesh = laraSkinJoints->ObjectMeshes[i];
			bucket = jointMesh->GetBucket(bucketIndex);

			if (bucket->NumVertices != 0)
			{
				// Bind buffers
				m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
				m_device->SetIndices(bucket->GetIndexBuffer());

				for (int iPass = 0; iPass < cPasses; iPass++)
				{
					effect->BeginPass(iPass);
					effect->CommitChanges();

					DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

					effect->EndPass();
				}
			}
		}
	}

	// Disable skinning, following will use the old way
	effect->SetBool(effect->GetParameterByName(NULL, "UseSkinning"), false);

	if (m_moveableObjects.find(Lara.holster) != m_moveableObjects.end())
	{
		// Draw holsters
		OBJECT_INFO* objHolsters = &Objects[Lara.holster];
		RendererObject* modelHolster = m_moveableObjects[Lara.holster];

		RendererMesh* leftHolster = modelHolster->ObjectMeshes[4];
		RendererMesh* rightHolster = modelHolster->ObjectMeshes[8];

		RendererBucket* bucket = leftHolster->GetBucket(bucketIndex);
		if (bucket->NumVertices != 0)
		{
			m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
			m_device->SetIndices(bucket->GetIndexBuffer());

			D3DXMatrixMultiply(&world, &laraObj->AnimationTransforms[THIGH_L], &m_LaraWorldMatrix);

			for (int iPass = 0; iPass < cPasses; iPass++)
			{
				effect->BeginPass(iPass);
				effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &world);
				effect->CommitChanges();

				DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

				effect->EndPass();
			}

			bucket = rightHolster->GetBucket(bucketIndex);

			m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
			m_device->SetIndices(bucket->GetIndexBuffer());

			D3DXMatrixMultiply(&world, &laraObj->AnimationTransforms[THIGH_R], &m_LaraWorldMatrix);

			for (int iPass = 0; iPass < cPasses; iPass++)
			{
				effect->BeginPass(iPass);
				effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &world);
				effect->CommitChanges();

				DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

				effect->EndPass();
			}
		}

		// Draw back gun
		if (Lara.backGun)
		{
			OBJECT_INFO* backGunObject = &Objects[Lara.backGun];
			RendererObject* modelBackGun = m_moveableObjects[Lara.backGun];
			RendererMesh* backGunMesh = modelBackGun->ObjectMeshes[HEAD];

			RendererBucket* bucket = backGunMesh->GetBucket(bucketIndex);
			if (bucket->NumVertices != 0)
			{
				m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
				m_device->SetIndices(bucket->GetIndexBuffer());

				D3DXMatrixMultiply(&world, &laraObj->AnimationTransforms[HEAD], &m_LaraWorldMatrix);

				for (int iPass = 0; iPass < cPasses; iPass++)
				{
					effect->BeginPass(iPass);
					effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &world);
					effect->CommitChanges();

					DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

					effect->EndPass();
				}
			}
		}
	}

	// Draw Lara's hairs
	if (bucketIndex == 0)
	{
		if (m_moveableObjects.find(ID_HAIR) != m_moveableObjects.end())
		{
			RendererObject* hairsObj = m_moveableObjects[ID_HAIR];
			D3DXMatrixIdentity(&world);

			for (int iPass = 0; iPass < cPasses; iPass++)
			{
				effect->BeginPass(iPass);
				effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &world);
				effect->CommitChanges();

				m_device->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, m_numHairVertices, m_numHairIndices / 3,
					m_hairIndices, D3DFMT_INDEX32, m_hairVertices, sizeof(RendererVertex));

				effect->EndPass();
			}
		}
	}

	return true;
}

bool Renderer::DrawItemLPP(RendererItemToDraw* itemToDraw, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass)
{ 
	D3DXMATRIX world;
	UINT cPasses = 1;

	ITEM_INFO* item = itemToDraw->Item;
	if (item->objectNumber == ID_LARA || item->objectNumber == 434)
		return true;

	OBJECT_INFO* obj = &Objects[item->objectNumber];
	RendererObject* moveableObj = m_moveableObjects[item->objectNumber];
	if (moveableObj->DoNotDraw)
		return true;

	if (!moveableObj->HasDataInBucket[bucketIndex])
		return true;

	LPD3DXEFFECT effect;
	if (pass == RENDERER_PASSES::RENDERER_PASS_SHADOW_MAP)
		effect = m_depthShader->GetEffect();
	else if (pass == RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH)
		effect = m_shaderReconstructZBuffer->GetEffect();
	else
		effect = m_shaderFillGBuffer->GetEffect();

	effect->SetBool(effect->GetParameterByName(NULL, "UseSkinning"), true);
	effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPES::MODEL_TYPE_MOVEABLE);
	effect->SetMatrixArray(effect->GetParameterByName(NULL, "Bones"), moveableObj->AnimationTransforms, moveableObj->ObjectMeshes.size());
	effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &itemToDraw->World);
	
	if (bucketIndex == RENDERER_BUCKETS::RENDERER_BUCKET_SOLID || bucketIndex == RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS)
		effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLEND_MODES::BLENDMODE_OPAQUE);
	else
		effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLEND_MODES::BLENDMODE_ALPHATEST);
	 
	for (__int32 i = 0; i < moveableObj->ObjectMeshes.size(); i++)
	{
		RendererMesh* mesh = moveableObj->ObjectMeshes[i];
		RendererBucket* bucket = mesh->GetBucket(bucketIndex);
		if (bucket->NumVertices == 0)
			continue;

		m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
		m_device->SetIndices(bucket->GetIndexBuffer());

		for (int iPass = 0; iPass < cPasses; iPass++)
		{
			effect->BeginPass(iPass);
			effect->CommitChanges();

			DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

			effect->EndPass();
		}
	}

	return true;
}

bool Renderer::DrawRoomLPP(__int32 roomIndex, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass)
{
	D3DXMATRIX world;
	UINT cPasses = 1;

	RendererRoom* room = m_rooms[roomIndex];
	ROOM_INFO* r = room->Room;
	RendererObject* roomObj = room->RoomObject;
	
	if (!roomObj->HasDataInBucket[bucketIndex])
		return true;

	if (roomObj->ObjectMeshes.size() == 0)
		return true;

	RendererMesh* mesh = roomObj->ObjectMeshes[0];
	RendererBucket* bucket = mesh->GetBucket(bucketIndex);

	LPD3DXEFFECT effect;
	if (pass == RENDERER_PASSES::RENDERER_PASS_SHADOW_MAP)
		effect = m_depthShader->GetEffect();
	else if (pass == RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH)
		effect = m_shaderReconstructZBuffer->GetEffect();
	else
		effect = m_shaderFillGBuffer->GetEffect();
	  
	D3DXMatrixTranslation(&world, r->x, r->y, r->z);
	   
	effect->SetBool(effect->GetParameterByName(NULL, "UseSkinning"), false);
	effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPES::MODEL_TYPE_ROOM);
	effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &world);

	if (bucketIndex == RENDERER_BUCKETS::RENDERER_BUCKET_SOLID || bucketIndex == RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS)
		effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLEND_MODES::BLENDMODE_OPAQUE);
	else
		effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLEND_MODES::BLENDMODE_ALPHATEST);

	m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
	m_device->SetIndices(bucket->GetIndexBuffer());

	for (int iPass = 0; iPass < cPasses; iPass++)
	{
		effect->BeginPass(iPass);
		effect->CommitChanges();
		  
		DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

		effect->EndPass();
	}

	return true;
}

bool Renderer::DrawStaticLPP(__int32 roomIndex, __int32 staticIndex, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass)
{
	D3DXMATRIX world;
	D3DXMATRIX rotation;
	UINT cPasses = 1;

	ROOM_INFO* room = &Rooms[roomIndex];
	MESH_INFO* sobj = &room->mesh[staticIndex];
	RendererObject* obj = m_staticObjects[sobj->staticNumber];
	if (!obj->HasDataInBucket[bucketIndex])
		return true;

	RendererMesh* mesh = obj->ObjectMeshes[0];
	RendererBucket* bucket = mesh->GetBucket(bucketIndex);
	if (bucket->NumVertices == 0)
		return true;

	LPD3DXEFFECT effect;
	if (pass == RENDERER_PASSES::RENDERER_PASS_SHADOW_MAP)
		effect = m_depthShader->GetEffect();
	else if (pass == RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH)
		effect = m_shaderReconstructZBuffer->GetEffect();
	else
		effect = m_shaderFillGBuffer->GetEffect();

	D3DXMatrixTranslation(&world, sobj->x, sobj->y, sobj->z);
	D3DXMatrixRotationY(&rotation, TR_ANGLE_TO_RAD(sobj->yRot));
	D3DXMatrixMultiply(&world, &rotation, &world);

	effect->SetBool(effect->GetParameterByName(NULL, "UseSkinning"), false);
	effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPES::MODEL_TYPE_STATIC);
	effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &world);

	if (bucketIndex == RENDERER_BUCKETS::RENDERER_BUCKET_SOLID || bucketIndex == RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS)
		effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLEND_MODES::BLENDMODE_OPAQUE);
	else
		effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLEND_MODES::BLENDMODE_ALPHATEST);

	m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
	m_device->SetIndices(bucket->GetIndexBuffer());

	for (int iPass = 0; iPass < cPasses; iPass++)
	{
		effect->BeginPass(iPass);
		effect->CommitChanges();

		DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

		effect->EndPass();
	}

	return true;
}

bool Renderer::DrawGunFlashes(RENDERER_PASSES pass)
{
	D3DXMATRIX world;
	D3DXMATRIX translation;
	D3DXMATRIX rotation;
	UINT cPasses = 1;

	RendererObject* laraObj = m_moveableObjects[ID_LARA];
	RendererObject* laraSkin = m_moveableObjects[ID_LARA_SKIN];

	ITEM_INFO* item = LaraItem;
	OBJECT_INFO* obj = &Objects[0];

	RendererLightInfo* light = &m_itemsLightInfo[0];
	LPD3DXEFFECT effect;
	if (pass == RENDERER_PASSES::RENDERER_PASS_SHADOW_MAP)
		effect = m_depthShader->GetEffect();
	else if (pass == RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH)
		effect = m_shaderReconstructZBuffer->GetEffect();
	else
		effect = m_shaderFillGBuffer->GetEffect();

	effect->SetBool(effect->GetParameterByName(NULL, "UseSkinning"), false);
	effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPES::MODEL_TYPE_MOVEABLE);
	effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLEND_MODES::BLENDMODE_ALPHATEST);

	__int16 length = 0;
	__int16 zOffset = 0;
	__int16 rotationX = 0;

	if (Lara.weaponItem != WEAPON_FLARE && Lara.weaponItem != WEAPON_SHOTGUN && Lara.weaponItem != WEAPON_CROSSBOW)
	{
		if (Lara.weaponItem == WEAPON_REVOLVER)
		{
			length = 192;
			zOffset = 68;
			rotationX = -14560;
		}
		else if (Lara.weaponItem == WEAPON_UZI)
		{
			length = 190;
			zOffset = 50;
		}
		else if (Lara.weaponItem == WEAPON_HK)
		{
			length = 300;
			zOffset = 92;
			rotationX = -14560;
		}
		else
		{
			length = 180;
			zOffset = 40;
			rotationX = -16830;
		}

		OBJECT_INFO* flashObj = &Objects[ID_GUN_FLASH];
		RendererObject* flashMoveable = m_moveableObjects[ID_GUN_FLASH];
		RendererMesh* flashMesh = flashMoveable->ObjectMeshes[0];
		RendererBucket* flashBucket = flashMesh->GetBucket(RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST);

		if (flashBucket->NumVertices != 0)
		{
			m_device->SetStreamSource(0, flashBucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
			m_device->SetIndices(flashBucket->GetIndexBuffer());

			D3DXMATRIX offset;
			D3DXMatrixTranslation(&offset, 0, length, zOffset);

			D3DXMATRIX rotation2;
			D3DXMatrixRotationX(&rotation2, TR_ANGLE_TO_RAD(rotationX));

			//m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
			//m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
			//m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);

			m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
			m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

			if (Lara.leftArm.flash_gun)
			{
				D3DXMatrixMultiply(&world, &laraObj->AnimationTransforms[HAND_L], &m_LaraWorldMatrix);
				D3DXMatrixMultiply(&world, &offset, &world);
				D3DXMatrixMultiply(&world, &rotation2, &world);

				for (int iPass = 0; iPass < cPasses; iPass++)
				{
					effect->BeginPass(iPass);
					effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &world);
					effect->CommitChanges();

					DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, flashBucket->NumVertices, 0, flashBucket->NumIndices / 3);

					effect->EndPass();
				}
			}

			if (Lara.rightArm.flash_gun)
			{
				D3DXMatrixMultiply(&world, &laraObj->AnimationTransforms[HAND_R], &m_LaraWorldMatrix);
				D3DXMatrixMultiply(&world, &offset, &world);
				D3DXMatrixMultiply(&world, &rotation2, &world);

				for (int iPass = 0; iPass < cPasses; iPass++)
				{
					effect->BeginPass(iPass);
					effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &world);
					effect->CommitChanges();

					DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, flashBucket->NumVertices, 0, flashBucket->NumIndices / 3);

					effect->EndPass();
				}
			}

			m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		}
	}

	return true;
}

void Renderer::CollectLightsLPP()
{ 
	m_lights.clear();

	// Add rooms lights
	for (__int32 i = 0; i < m_roomsToDraw.size(); i++)
	{
		RendererRoom* room = m_rooms[m_roomsToDraw[i]];
		if (room == NULL)
			continue;

		for (__int32 j = 0; j < room->Lights.size(); j++)
		{
			D3DXVECTOR3 laraPos = D3DXVECTOR3(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
			D3DXVECTOR3 lightVector = D3DXVECTOR3(room->Lights[j]->Position.x,
				room->Lights[j]->Position.y, room->Lights[j]->Position.z);

			if (D3DXVec3Length(&(laraPos-lightVector)) >= 1024.0f * 32.0f)
				continue;

			m_lights.push_back(room->Lights[j]);
		}
	}

	// Add dynamic lights
	for (__int32 i = 0; i < m_dynamicLights.size(); i++)
		m_lights.push_back(m_dynamicLights[i]);

	// Now try to search for a shadow caster, using Lara as reference
	RendererRoom* room = m_rooms[LaraItem->roomNumber];

	// Search for the brightest light. We do a simple version of the classic calculation done in pixel shader.
	RendererLight* brightestLight = NULL;
	float brightest = 0.0f;

	// Try room lights
	if (room->Lights.size() != 0)
	{
		for (__int32 j = 0; j < room->Lights.size(); j++)
		{
			RendererLight* light = room->Lights[j];

			D3DXVECTOR4 itemPos = D3DXVECTOR4(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, 1.0f);
			D3DXVECTOR4 lightVector = itemPos - light->Position;

			float distance = D3DXVec4Length(&lightVector);
			D3DXVec4Normalize(&lightVector, &lightVector);

			float intensity;
			float attenuation;
			float angle;
			float d;
			float attenuationRange;
			float attenuationAngle;

			switch (light->Type)
			{
			case LIGHT_TYPES::LIGHT_TYPE_POINT:
				if (distance > light->Out || light->Out < 2048.0f)
					continue;

				attenuation = 1.0f - distance / light->Out;
				intensity = max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

				if (intensity >= brightest)
				{
					brightest = intensity;
					brightestLight = light;
				}

				break;

			case  LIGHT_TYPES::LIGHT_TYPE_SPOT:
				if (distance > light->Range)
					continue;

				attenuation = 1.0f - distance / light->Range;
				intensity = max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

				if (intensity >= brightest)
				{
					brightest = intensity;
					brightestLight = light;
				}

				break;
			}
		}
	}

	// If the brightest light is found, then fill the data structure. We ignore for now dynamic lights for shadows.
	m_shadowLight = brightestLight;
}

bool Renderer::DrawSkyLPP()
{
	D3DXVECTOR4 color = D3DXVECTOR4(SkyColor1.r / 255.0f, SkyColor1.g / 255.0f, SkyColor1.b / 255.0f, 1.0f);

	// First update the sky in the case of storm
	if (gfLevelFlags & 0x40 || true)
	{
		if (Unk_00E6D74C || Unk_00E6D73C)
		{
			UpdateStorm();
			if (StormTimer > -1)
				StormTimer--;
			if (!StormTimer)
				SoundEffect(182, 0, 0);
		}
		else if (!(rand() & 0x7F))
		{
			Unk_00E6D74C = (rand() & 0x1F) + 16;
			Unk_00E6E4DC = rand() + 256;
			StormTimer = (rand() & 3) + 12;
		}

		color = D3DXVECTOR4((SkyStormColor.r + 44) / 255.0f, SkyStormColor.g / 255.0f, SkyStormColor.b / 255.0f, 1.0f);
	}

	D3DXMATRIX world;
	D3DXMATRIX translation;
	D3DXMATRIX rotation;
	D3DXMATRIX scale;
	UINT cPasses = 1;

	LPD3DXEFFECT effect = m_shaderFillGBuffer->GetEffect();
	
	effect->SetTexture(effect->GetParameterByName(NULL, "TextureAtlas"), m_skyTexture);
	effect->SetVector(effect->GetParameterByName(NULL, "Color"), &color);
	effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPES::MODEL_TYPE_SKY);

	m_device->SetStreamSource(0, m_skyQuad->VertexBuffer, 0, sizeof(RendererVertex));
	m_device->SetIndices(m_skyQuad->IndexBuffer);

	// Quads have normal up, so I must rotate the plane by X (or Z)
	D3DXMatrixRotationX(&m_tempRotation, PI);

	// Hardcoded kingdom :)
	for (__int32 i = 0; i < 2; i++)
	{
		D3DXMatrixTranslation(&m_tempTranslation, Camera.pos.x + SkyPos1 - i * 9728.0f, Camera.pos.y - 1536.0f, Camera.pos.z);
		D3DXMatrixMultiply(&world, &m_tempRotation, &m_tempTranslation);
		effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &world);

		for (int iPass = 0; iPass < cPasses; iPass++)
		{
			effect->BeginPass(iPass);
			effect->CommitChanges();

			DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, m_quad->NumVertices, 0, m_quad->NumIndices / 3);

			effect->EndPass();
		}
	}

	// Draw the horizon
	RendererObject* horizonObj = m_moveableObjects[ID_HORIZON];
	RendererMesh* mesh = horizonObj->ObjectMeshes[0];
	effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPES::MODEL_TYPE_HORIZON);

	effect->SetTexture(effect->GetParameterByName(NULL, "TextureAtlas"), m_textureAtlas);
	effect->SetVector(effect->GetParameterByName(NULL, "Color"), &D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));
	effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLEND_MODES::BLENDMODE_ALPHATEST);

	D3DXMatrixTranslation(&world, Camera.pos.x, Camera.pos.y, Camera.pos.z);
	effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &world);


	for (__int32 i = 0; i < NUM_BUCKETS; i++)
	{
		RendererBucket* bucket = mesh->GetBucket(i);

		// Bind buffers
		m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
		m_device->SetIndices(bucket->GetIndexBuffer());

		for (int iPass = 0; iPass < cPasses; iPass++)
		{
			effect->BeginPass(iPass);
			effect->CommitChanges();

			DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

			effect->EndPass();
		}
	}

	effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLEND_MODES::BLENDMODE_OPAQUE);

	m_device->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);

	return true;
}
 
void Renderer::AddDynamicLight(__int32 x, __int32 y, __int32 z, __int16 falloff, byte r, byte g, byte b)
{
	RendererLight* dynamicLight = new RendererLight();

	dynamicLight->Position = D3DXVECTOR4(x, y, z, 1.0f);
	dynamicLight->Color = D3DXVECTOR4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
	dynamicLight->Out = falloff * 256.0f;
	dynamicLight->Type = LIGHT_TYPES::LIGHT_TYPE_POINT;
	dynamicLight->Dynamic = true;
	dynamicLight->Intensity = 2.0f;

	m_dynamicLights.push_back(dynamicLight);
	NumDynamics++;
}

void Renderer::ClearDynamicLights()
{
	for (vector<RendererLight*>::iterator it = m_dynamicLights.begin(); it != m_dynamicLights.end(); ++it)
		delete (*it);
	m_dynamicLights.clear();
	NumDynamics = 0;
}

void Renderer::CreateBillboardMatrix(D3DXMATRIX* out, D3DXVECTOR3* particlePos, D3DXVECTOR3* cameraPos)
{
	D3DXVECTOR3 look = *particlePos;
	look = look - *cameraPos;
	D3DXVec3Normalize(&look, &look);

	D3DXVECTOR3 cameraUp = D3DXVECTOR3(0.0f, -1.0f, 0.0f);

	D3DXVECTOR3 right;
	D3DXVec3Cross(&right, &cameraUp, &look);
	D3DXVec3Normalize(&right, &right);

	D3DXVECTOR3 up;
	D3DXVec3Cross(&up, &look, &right);
	D3DXVec3Normalize(&up, &up);

	D3DXMatrixIdentity(out);
	
	out->_11 = right.x;
	out->_12 = right.y;
	out->_13 = right.z;

	out->_21 = up.x;
	out->_22 = up.y;
	out->_23 = up.z;

	out->_31 = look.x;
	out->_32 = look.y;
	out->_33 = look.z;

	out->_41 = particlePos->x;
	out->_42 = particlePos->y;
	out->_43 = particlePos->z;
}

void Renderer::AddSprite(RendererSprite* sprite, __int32 x, __int32 y, __int32 z, byte r, byte g, byte b, float rotation, float scale, float width, float height)
{
	scale = 1.0f;

	width *= scale;
	height *= scale;

	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;

	D3DXMATRIX billboardMatrix;
	CreateBillboardMatrix(&billboardMatrix, &D3DXVECTOR3(x, y, z), &D3DXVECTOR3(Camera.pos.x, Camera.pos.y, Camera.pos.z));

	D3DXVECTOR3 p0 = D3DXVECTOR3(-halfWidth, -halfHeight, 0);
	D3DXVECTOR3 p1 = D3DXVECTOR3(halfWidth, -halfHeight, 0);
	D3DXVECTOR3 p2 = D3DXVECTOR3(halfWidth, halfHeight, 0);
	D3DXVECTOR3 p3 = D3DXVECTOR3(-halfWidth, halfHeight, 0);

	D3DXVECTOR4 p0t;
	D3DXVECTOR4 p1t;
	D3DXVECTOR4 p2t;
	D3DXVECTOR4 p3t;

	D3DXVec3Transform(&p0t, &p0, &billboardMatrix);
	D3DXVec3Transform(&p1t, &p1, &billboardMatrix);
	D3DXVec3Transform(&p2t, &p2, &billboardMatrix);
	D3DXVec3Transform(&p3t, &p3, &billboardMatrix);

	RendererVertex v;
	__int32 baseVertex = m_spritesVertices.size();

	v.x = p0t.x;
	v.y = p0t.y;
	v.z = p0t.z;
	v.u = sprite->UV[0].x;
	v.v = sprite->UV[0].y;
	v.r = r / 255.0f;
	v.g = g / 255.0f;
	v.b = b / 255.0f;
	v.a = 1.0f;
	m_spritesVertices.push_back(v);

	v.x = p1t.x;
	v.y = p1t.y;
	v.z = p1t.z;
	v.u = sprite->UV[1].x;
	v.v = sprite->UV[1].y;
	v.r = r / 255.0f;
	v.g = g / 255.0f;
	v.b = b / 255.0f;
	v.a = 1.0f;
	m_spritesVertices.push_back(v);

	v.x = p2t.x;
	v.y = p2t.y;
	v.z = p2t.z;
	v.u = sprite->UV[2].x;
	v.v = sprite->UV[2].y;
	v.r = r / 255.0f;
	v.g = g / 255.0f;
	v.b = b / 255.0f;
	v.a = 1.0f;
	m_spritesVertices.push_back(v);

	v.x = p3t.x;
	v.y = p3t.y;
	v.z = p3t.z;
	v.u = sprite->UV[3].x;
	v.v = sprite->UV[3].y;
	v.r = r / 255.0f;
	v.g = g / 255.0f;
	v.b = b / 255.0f;
	v.a = 1.0f;
	m_spritesVertices.push_back(v);

	m_spritesIndices.push_back(baseVertex + 0);
	m_spritesIndices.push_back(baseVertex + 1);
	m_spritesIndices.push_back(baseVertex + 2);
	m_spritesIndices.push_back(baseVertex + 0);
	m_spritesIndices.push_back(baseVertex + 2);
	m_spritesIndices.push_back(baseVertex + 3);
}

void Renderer::DrawFires()
{
	for (__int32 k = 0; k < 32; k++)
	{
		FIRE_LIST* fire = &Fires[k];
		if (fire->on)
		{ 
			for (__int32 i = 0; i < 20; i++)
			{
				FIRE_SPARKS* spark = &FireSparks[i];
				if (spark->on)
				{
					AddSprite(m_sprites[spark->def], 
								fire->x + spark->x, fire->y + spark->y, fire->z + spark->z, 
								spark->r, spark->g, spark->b, 
								TR_ANGLE_TO_RAD(spark->rotAng), spark->scalar, spark->size * 4.0f, spark->size * 4.0f);
				}
			}
		}
	}
}

void Renderer::DrawSmokes()
{
	for (__int32 i = 0; i < 32; i++)
	{
		SMOKE_SPARKS* spark = &SmokeSparks[i];
		if (spark->On)
		{
			AddSprite(m_sprites[spark->Def],
				spark->x, spark->y, spark->z,
				spark->Shade, spark->Shade, spark->Shade,
				TR_ANGLE_TO_RAD(spark->RotAng), spark->Scalar, spark->Size * 4.0f, spark->Size * 4.0f);
		}
	}
}

void Renderer::DrawBlood()
{
	for (__int32 i = 0; i < 32; i++)
	{
		BLOOD_STRUCT* blood = &Blood[i];
		if (blood->On)
		{
			AddSprite(m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + 15],
				blood->x, blood->y, blood->z,
				blood->Shade * 244, blood->Shade * 0, blood->Shade * 0,
				TR_ANGLE_TO_RAD(blood->RotAng), 1.0f, blood->Size * 8.0f, blood->Size * 8.0f);
		}
	}
}

bool Renderer::DrawGunshells(RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass)
{
	D3DXMATRIX world;
	UINT cPasses = 1;

	LPD3DXEFFECT effect;
	if (pass == RENDERER_PASSES::RENDERER_PASS_SHADOW_MAP)
		effect = m_depthShader->GetEffect();
	else
		effect = m_shaderFillGBuffer->GetEffect();

	for (__int32 i = 0; i < 24; i++)
	{
		GUNSHELL_STRUCT* gunshell = &GunShells[i];
		if (gunshell->counter > 0)
		{
			OBJECT_INFO* obj = &Objects[gunshell->objectNumber];
			RendererObject* moveableObj = m_moveableObjects[gunshell->objectNumber];
			
			if (!moveableObj->HasDataInBucket[bucketIndex])
				return true;
			
			effect->SetBool(effect->GetParameterByName(NULL, "UseSkinning"), false);
			effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPES::MODEL_TYPE_MOVEABLE);
			
			D3DXMatrixTranslation(&m_tempTranslation, gunshell->pos.xPos, gunshell->pos.yPos, gunshell->pos.zPos);
			D3DXMatrixRotationYawPitchRoll(&m_tempRotation, TR_ANGLE_TO_RAD(gunshell->pos.yRot),
				TR_ANGLE_TO_RAD(gunshell->pos.xRot),
				TR_ANGLE_TO_RAD(gunshell->pos.zRot));
			D3DXMatrixMultiply(&world, &m_tempRotation, &m_tempTranslation);
			effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &world);

			if (bucketIndex == RENDERER_BUCKETS::RENDERER_BUCKET_SOLID || bucketIndex == RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS)
				effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLEND_MODES::BLENDMODE_OPAQUE);
			else
				effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLEND_MODES::BLENDMODE_ALPHATEST);

			for (__int32 i = 0; i < moveableObj->ObjectMeshes.size(); i++)
			{
				RendererMesh* mesh = moveableObj->ObjectMeshes[i];
				RendererBucket* bucket = mesh->GetBucket(bucketIndex);
				if (bucket->NumVertices == 0)
					continue;

				m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
				m_device->SetIndices(bucket->GetIndexBuffer());

				for (int iPass = 0; iPass < cPasses; iPass++)
				{
					effect->BeginPass(iPass);
					effect->CommitChanges();

					DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

					effect->EndPass();
				}
			}
		}

	}

	return true;
}

bool Renderer::DoRain()
{
	RendererVertex vertices[NUM_RAIN_DROPS * 2];

	if (m_firstWeather)
	{
		for (__int32 i = 0; i < NUM_RAIN_DROPS; i++)
			m_rain[i].Reset = true;
	}

	for (__int32 i = 0; i < NUM_RAIN_DROPS; i++)
	{
		RendererWeatherParticle* drop = &m_rain[i];

		if (drop->Reset)
		{ 
			drop->X = LaraItem->pos.xPos + rand() % WEATHER_RADIUS - WEATHER_RADIUS / 2.0f;
			drop->Y = LaraItem->pos.yPos - (m_firstWeather ? rand() % WEATHER_HEIGHT : WEATHER_HEIGHT) + (rand() % 512);
			drop->Z = LaraItem->pos.zPos + rand() % WEATHER_RADIUS - WEATHER_RADIUS / 2.0f;

			__int16 roomNumber = Camera.pos.roomNumber;
			FLOOR_INFO* floor = GetFloor(drop->X, drop->Y, drop->Z, &roomNumber);
			ROOM_INFO* room = &Rooms[roomNumber];
			if (!(room->flags & 32))
				continue;

			drop->Size = RAIN_SIZE + (rand() % 64);
			drop->AngleH = (rand() % RAIN_MAX_ANGLE_H) * RADIAN;
			drop->AngleV = (rand() % RAIN_MAX_ANGLE_V) * RADIAN;
			drop->Reset = false;
		}

		RendererVertex* vertex = &vertices[2 * i];
		
		vertex->x = drop->X;
		vertex->y = drop->Y;
		vertex->z = drop->Z;
		vertex->r = RAIN_COLOR;
		vertex->g = RAIN_COLOR;
		vertex->b = RAIN_COLOR;

		vertex = &vertices[2 * i + 1];

		float radius = drop->Size * sin(drop->AngleV);
		
		float dx = sin(drop->AngleH) * radius;
		float dy = drop->Size * cos(drop->AngleH);
		float dz = cos(drop->AngleH) * radius;
		
		drop->X += dx;
		drop->Y += RAIN_DELTA_Y;
		drop->Z += dz;

		vertex->x = drop->X;
		vertex->y = drop->Y;
		vertex->z = drop->Z;
		vertex->r = RAIN_COLOR;
		vertex->g = RAIN_COLOR;
		vertex->b = RAIN_COLOR;

		__int16 roomNumber = Camera.pos.roomNumber;
		FLOOR_INFO* floor = GetFloor(drop->X, drop->Y, drop->Z, &roomNumber);
		ROOM_INFO* room = &Rooms[roomNumber];
		if (drop->Y >= room->y + room->minfloor)
			drop->Reset = true;
	}

	LPD3DXEFFECT effect = m_shaderRain->GetEffect();
	UINT cPasses = 1;

	m_device->BeginScene();
	effect->Begin(&cPasses, 0);

	effect->SetMatrix(effect->GetParameterByName(NULL, "View"), &ViewMatrix);
	effect->SetMatrix(effect->GetParameterByName(NULL, "Projection"), &ProjectionMatrix);

	__int32 numBuckets = 2;
	for (int iPass = 0; iPass < cPasses; iPass++)
	{
		effect->BeginPass(iPass);
		effect->CommitChanges();
		for (__int32 i = 0; i < numBuckets; i++)
			m_device->DrawPrimitiveUP(D3DPRIMITIVETYPE::D3DPT_LINELIST, NUM_RAIN_DROPS / 2, &vertices[0], sizeof(RendererVertex));
		effect->EndPass();
	}

	effect->End();
	m_device->EndScene();

	m_firstWeather = false;

	return true;
}

bool Renderer::DoSnow()
{
	if (m_firstWeather)
		for (__int32 i = 0; i < NUM_SNOW_PARTICLES; i++)
			m_snow[i].Reset = true;

	for (__int32 i = 0; i < NUM_SNOW_PARTICLES; i++)
	{
		RendererWeatherParticle* snow = &m_snow[i];

		if (snow->Reset)
		{
			snow->X = LaraItem->pos.xPos + rand() % WEATHER_RADIUS - WEATHER_RADIUS / 2.0f;
			snow->Y = LaraItem->pos.yPos - (m_firstWeather ? rand() % WEATHER_HEIGHT : WEATHER_HEIGHT) + (rand() % 512);
			snow->Z = LaraItem->pos.zPos + rand() % WEATHER_RADIUS - WEATHER_RADIUS / 2.0f;
			snow->Size = SNOW_DELTA_Y + (rand() % 64);
			snow->AngleH = (rand() % SNOW_MAX_ANGLE_H) * RADIAN;
			snow->AngleV = (rand() % SNOW_MAX_ANGLE_V) * RADIAN;
			snow->Reset = false;
		}

		float radius = snow->Size * sin(snow->AngleV);

		float dx = sin(snow->AngleH) * radius;
		float dy = snow->Size * cos(snow->AngleH);
		float dz = cos(snow->AngleH) * radius;

		snow->X += dx;
		snow->Y += SNOW_DELTA_Y;
		snow->Z += dz;

		if (snow->X <= 0 || snow->Z <= 0 || snow->X >= 100 * 1024.0f || snow->Z >= 100 * 1024.0f)
		{
			snow->Reset = true;
			continue;
		}

		AddSprite(m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + 14], snow->X, snow->Y, snow->Z, 255, 255, 255,
			0.0f, 1.0f, SNOW_SIZE, SNOW_SIZE);

		__int16 roomNumber = Camera.pos.roomNumber;
		FLOOR_INFO* floor = GetFloor(snow->X, snow->Y, snow->Z, &roomNumber);
		ROOM_INFO* room = &Rooms[roomNumber];
		if (snow->Y >= room->y + room->minfloor)
			snow->Reset = true;
	}

	m_firstWeather = false;

	return true;
}