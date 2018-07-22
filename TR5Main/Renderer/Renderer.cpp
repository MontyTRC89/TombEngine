#include "Renderer.h"
#include "..\Global\global.h"
#include <stdio.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "..\Specific\input.h"
#include "..\Specific\winmain.h"
#include "..\Specific\roomload.h"
#include <DxErr.h>
#pragma comment(lib, "dxerr.lib")
#include <stack> 
#include "..\Game\draw.h"
#include "..\Specific\roomload.h"
#include "..\Specific\game.h"
#include "..\Game\healt.h"

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
}

Renderer::~Renderer()
{
	if (m_device != NULL) m_device->Release();
	if (m_d3D != NULL) m_d3D->Release();
	if (m_font != NULL) m_font->Release();
	if (m_effect != NULL) m_effect->Release();
	delete m_lines;
}

void Renderer::InitialiseBucketBuffer(RendererBucket* bucket)
{
	if (bucket == NULL)
		return;
	
	if (bucket->VertexBuffer != NULL)
		bucket->VertexBuffer->Release();

	if (bucket->NumVertices == 0)
		return;

	m_device->CreateVertexBuffer(bucket->Vertices.size() * sizeof(RendererVertex), D3DUSAGE_WRITEONLY,
		0, D3DPOOL_MANAGED, &bucket->VertexBuffer, NULL);

	void* vertices;

	bucket->VertexBuffer->Lock(0, 0, &vertices, 0);
	memcpy(vertices, bucket->Vertices.data(), bucket->Vertices.size() * sizeof(RendererVertex));
	bucket->VertexBuffer->Unlock();

	if (bucket->IndexBuffer != NULL)
		bucket->IndexBuffer->Release();

	if (bucket->NumIndices == 0)
		return;

	m_device->CreateIndexBuffer(bucket->Indices.size() * 4, D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_MANAGED,
		&bucket->IndexBuffer, NULL);

	void* indices;

	bucket->IndexBuffer->Lock(0, 0, &indices, 0);
	memcpy(indices, bucket->Indices.data(), bucket->Indices.size() * 4);
	bucket->IndexBuffer->Unlock();
}

bool Renderer::Initialise(__int32 w, __int32 h, bool windowed, HWND handle)
{
	DB_Log(2, "Renderer::Initialise - DLL");
	printf("Initialising DX\n");

	m_d3D = Direct3DCreate9(D3D_SDK_VERSION); //printf("%d\n", m_d3D);
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
	d3dpp.MultiSampleType = D3DMULTISAMPLE_8_SAMPLES;

	m_d3D->CreateDevice(D3DADAPTER_DEFAULT,
						D3DDEVTYPE_HAL,
						handle,
						D3DCREATE_HARDWARE_VERTEXPROCESSING,
						&d3dpp,
						&m_device);

	// Load the white sprite 
	/*D3DXCreateTextureFromFileEx(m_device, "WhiteSprite.png", D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0,
		D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
		D3DCOLOR_XRGB(255, 0, 255), NULL, NULL, &whiteSprite);*/

	DWORD dwShaderFlags = 0;
	dwShaderFlags |= D3DXSHADER_ENABLE_BACKWARDS_COMPATIBILITY;

	LPD3DXBUFFER ppCompilationErrors = NULL;
	HRESULT res = D3DXCreateEffectFromFile(
		m_device,
		"Basic.fx",
		NULL, // CONST D3DXMACRO* pDefines,
		NULL, // LPD3DXINCLUDE pInclude,
		dwShaderFlags,
		NULL, // LPD3DXEFFECTPOOL pPool,
		&m_effect,
		&ppCompilationErrors);

	//char* pText = (char*)ppCompilationErrors->GetBufferPointer();
//	printf("%s\n", pText);

	res = D3DXCreateEffectFromFile(
		m_device,
		"Lines.fx",
		NULL, // CONST D3DXMACRO* pDefines,
		NULL, // LPD3DXINCLUDE pInclude,
		dwShaderFlags,
		NULL, // LPD3DXEFFECTPOOL pPool,
		&m_effectLines2D,
		&ppCompilationErrors);

	res = D3DXCreateEffectFromFile(
		m_device,
		"menu_inventory_background.fx",
		NULL, // CONST D3DXMACRO* pDefines,
		NULL, // LPD3DXINCLUDE pInclude,
		dwShaderFlags,
		NULL, // LPD3DXEFFECTPOOL pPool,
		&m_menuInventoryBackground,
		&ppCompilationErrors);

	

	//printf("Error: %s error description: %s\n",
	//DXGetErrorString(res), DXGetErrorDescription(res));

	

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
	m_device->CreateVertexDeclaration(roomVertexElements, &m_vertexDeclaration);

	D3DVERTEXELEMENT9 roomVertexElementsLines2D[] =
	{
		{ 0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		D3DDECL_END()
	};

	m_vertexDeclarationLines2D = NULL;
	m_device->CreateVertexDeclaration(roomVertexElements, &m_vertexDeclarationLines2D);

	D3DXCreateFont(m_device, 13, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &m_font);
	D3DXCreateFont(m_device, 28, 0, FW_NORMAL, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Ondine"), &m_gameFont);

	D3DXCreateSprite(m_device, &m_sprite);

	m_lines = (RendererLine2D*)malloc(MAX_LINES_2D * sizeof(RendererLine2D));
	m_line = NULL;
	D3DXCreateLine(m_device, &m_line);

	printf("DX initialised\n");

	m_fadeTimer = 0;

	res = m_device->CreateTexture(ScreenWidth, ScreenHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, 
		D3DPOOL_DEFAULT, &m_renderTarget, NULL);

//	res=D3DXCreateTexture(m_device, ScreenWidth, ScreenHeight, 1, D3DUSAGE_RENDERTARGET | D3DX_DEFAULT_NONPOW2,
	//	D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_renderTarget);

	printf("Error: %s error description: %s\n",
	DXGetErrorString(res), DXGetErrorDescription(res));

	//res=D3DXCreateTexture(m_device, ScreenWidth, ScreenHeight, 0, D3DUSAGE_RENDERTARGET,
	//	D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_textureAtlas);

	m_device->CreateDepthStencilSurface(ScreenWidth, ScreenHeight, D3DFMT_D24S8, D3DMULTISAMPLE_NONE,
				0, true, &m_renderTargetDepth, NULL);

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

	if (flags & PRINTSTRING_BLINK)
	{
		color = D3DCOLOR_ARGB(255, m_blinkColorValue, m_blinkColorValue, m_blinkColorValue);
		m_blinkColorValue += m_blinkColorDirection * 8;
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

	m_gameFont->DrawTextA(NULL, string, -1, &rect, 0, color);
}

bool Renderer::DrawMessage(LPD3DXFONT font, unsigned int x, unsigned int y, 
						   int alpha, unsigned char r, unsigned char g, unsigned char b, LPCSTR Message)
{
	D3DCOLOR fontColor = D3DCOLOR_ARGB(alpha, r, g, b);
	RECT rct;
	rct.left = x;
	rct.right = 512;
	rct.top = y;
	rct.bottom = rct.top + 16;
	font->DrawTextA(NULL, Message, -1, &rct, 0, fontColor);

	return true;
}

RendererMesh* Renderer::GetRendererMeshFromTrMesh(__int16* meshPtr, __int16 boneIndex, __int32 isJoints, __int32 isHairs)
{
	RendererMesh* mesh = new RendererMesh();

	__int16 cx = *meshPtr++;
	__int16 cy = *meshPtr++;
	__int16 cz = *meshPtr++;
	__int16 r1 = *meshPtr++;
	__int16 r2 = *meshPtr++;

	__int16 numVertices = *meshPtr++;
	//if (numVertices > 255)
	//	numVertices = numVertices >> 8;

	VECTOR* vertices = (VECTOR*)malloc(sizeof(VECTOR) * numVertices); 
	//printf("numVertices: %d\n", numVertices);
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
	//printf("numNormals: %d\n", numNormals);
	if (numNormals > 0)
		meshPtr += numNormals * 3;
	else
		meshPtr += (-numNormals);

	__int16 numRectangles = *meshPtr++;
	//printf("numRectangles: %d\n", numRectangles);

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

		if (!doubleSided)
		{
			if (texture->attribute == 2 || (effects & 1))
				bucket = mesh->Buckets[RENDERBUCKET_TRANSPARENT];
			else if (texture->attribute == 0)
				bucket = mesh->Buckets[RENDERBUCKET_SOLID];
			else
				bucket = mesh->Buckets[RENDERBUCKET_ALPHA_TEST];
		}
		else
		{
			if (texture->attribute == 2 || (effects & 1))
				bucket = mesh->Buckets[RENDERBUCKET_TRANSPARENT_DS];
			else if (texture->attribute == 0)
				bucket = mesh->Buckets[RENDERBUCKET_SOLID_DS];
			else
				bucket = mesh->Buckets[RENDERBUCKET_ALPHA_TEST_DS];
		}

		__int32 baseVertices = bucket->NumVertices;
		for (__int32 v = 0; v < 4; v++)
		{
			RendererVertex vertex;

			vertex.x = vertices[indices[v]].vx;
			vertex.y = vertices[indices[v]].vy;
			vertex.z = vertices[indices[v]].vz;

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

		if (!doubleSided)
		{
			if (texture->attribute == 2 || (effects & 1))
				bucket = mesh->Buckets[RENDERBUCKET_TRANSPARENT];
			else if (texture->attribute == 0)
				bucket = mesh->Buckets[RENDERBUCKET_SOLID];
			else
				bucket = mesh->Buckets[RENDERBUCKET_ALPHA_TEST];
		}
		else
		{
			if (texture->attribute == 2 || (effects & 1))
				bucket = mesh->Buckets[RENDERBUCKET_TRANSPARENT_DS];
			else if (texture->attribute == 0)
				bucket = mesh->Buckets[RENDERBUCKET_SOLID_DS];
			else
				bucket = mesh->Buckets[RENDERBUCKET_ALPHA_TEST_DS];
		}

		__int32 baseVertices = bucket->NumVertices;
		for (__int32 v = 0; v < 3; v++)
		{
			RendererVertex vertex;

			vertex.x = vertices[indices[v]].vx;
			vertex.y = vertices[indices[v]].vy;
			vertex.z = vertices[indices[v]].vz;

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

	return mesh;
}

bool Renderer::PrepareDataForTheRenderer()
{
	// Step 0: free all previus resources
	for (map<__int32, RendererObject*>::iterator it = m_roomObjects.begin(); it != m_roomObjects.end(); ++it)
		delete (it->second);
	m_roomObjects.clear();

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

	// Release the temp buffer
	free(buffer);

	// Release the eventually old texture
	if (m_fontAndMiscTexture != NULL)
		m_fontAndMiscTexture->Release();

	// Create the new texture atlas
	D3DXCreateTexture(m_device, 256, 768, 0, D3DX_DEFAULT,
		D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_fontAndMiscTexture);

	// Lock the texture pixels and copy them from the buffer
	m_fontAndMiscTexture->LockRect(0, &rect, NULL, NULL);
	dest = static_cast<unsigned char*>(rect.pBits);
	memcpy(dest, MiscTextures, 256 * 768 * 4);
	m_fontAndMiscTexture->UnlockRect(0);

	D3DXSaveTextureToFile("E:\\Misc.png", D3DXIMAGE_FILEFORMAT::D3DXIFF_PNG, m_fontAndMiscTexture, NULL);
	
	// Step 2: prepare rooms
	for (__int32 i = 0; i < NumberRooms; i++)
	{
		ROOM_INFO* room = &Rooms[i];
		if (room->NumVertices == 0)
			continue;

		RendererObject* roomObject = new RendererObject(1, -1);
		RendererMesh* mesh = new RendererMesh();

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

					if (!doubleSided)
					{
						if (texture->attribute == 0)
							bucket = mesh->Buckets[RENDERBUCKET_SOLID];
						else if (texture->attribute == 1)
							bucket = mesh->Buckets[RENDERBUCKET_ALPHA_TEST];
						else
							bucket = mesh->Buckets[RENDERBUCKET_TRANSPARENT];
					}
					else
					{
						if (texture->attribute == 0)
							bucket = mesh->Buckets[RENDERBUCKET_SOLID_DS];
						else if (texture->attribute == 1)
							bucket = mesh->Buckets[RENDERBUCKET_ALPHA_TEST_DS];
						else
							bucket = mesh->Buckets[RENDERBUCKET_TRANSPARENT_DS];
					}

					__int32 baseVertices = bucket->NumVertices;
					for (__int32 v = 0; v < 4; v++)
					{
						RendererVertex vertex; // = new RendererVertex();

						vertex.x = vertices[poly->Vertices[v]].Vertex.x;
						vertex.y = vertices[poly->Vertices[v]].Vertex.y;
						vertex.z = vertices[poly->Vertices[v]].Vertex.z;

						vertex.nx = vertices[poly->Vertices[v]].Normal.x;
						vertex.ny = vertices[poly->Vertices[v]].Normal.y;
						vertex.nz = vertices[poly->Vertices[v]].Normal.z;

						vertex.u = (texture->vertices[v].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
						vertex.v = (texture->vertices[v].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;

						vertex.r = ((vertices[poly->Vertices[v]].Colour >> 16) & 0xFF) / 255.0f;
						vertex.g = ((vertices[poly->Vertices[v]].Colour >> 8) & 0xFF) / 255.0f;
						vertex.b = ((vertices[poly->Vertices[v]].Colour >> 0) & 0xFF) / 255.0f;
						vertex.a = 1.0f;

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

					if (!doubleSided)
					{
						if (texture->attribute == 0)
							bucket = mesh->Buckets[RENDERBUCKET_SOLID];
						else if (texture->attribute == 1)
							bucket = mesh->Buckets[RENDERBUCKET_ALPHA_TEST];
						else
							bucket = mesh->Buckets[RENDERBUCKET_TRANSPARENT];
					}
					else
					{
						if (texture->attribute == 0)
							bucket = mesh->Buckets[RENDERBUCKET_SOLID_DS];
						else if (texture->attribute == 1)
							bucket = mesh->Buckets[RENDERBUCKET_ALPHA_TEST_DS];
						else
							bucket = mesh->Buckets[RENDERBUCKET_TRANSPARENT_DS];
					}

					__int32 baseVertices = bucket->NumVertices;
					for (__int32 v = 0; v < 3; v++)
					{
						RendererVertex vertex; // = new RendererVertex();

						vertex.x = vertices[poly->Vertices[v]].Vertex.x;
						vertex.y = vertices[poly->Vertices[v]].Vertex.y;
						vertex.z = vertices[poly->Vertices[v]].Vertex.z;

						vertex.nx = vertices[poly->Vertices[v]].Normal.x;
						vertex.ny = vertices[poly->Vertices[v]].Normal.y;
						vertex.nz = vertices[poly->Vertices[v]].Normal.z;

						vertex.u = (texture->vertices[v].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
						vertex.v = (texture->vertices[v].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;

						vertex.r = ((vertices[poly->Vertices[v]].Colour >> 16) & 0xFF) / 255.0f;
						vertex.g = ((vertices[poly->Vertices[v]].Colour >> 8) & 0xFF) / 255.0f;
						vertex.b = ((vertices[poly->Vertices[v]].Colour >> 0) & 0xFF) / 255.0f;
						vertex.a = 1.0f;

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

					polygons += sizeof(tr4_mesh_face3);
				}
			}
		}

		for (__int32 i = 0; i < NUM_BUCKETS; i++)
			InitialiseBucketBuffer(mesh->Buckets[i]);

		printf("Buckets OK\n");

		roomObject->ObjectMeshes.push_back(mesh);
		m_roomObjects.insert(pair<__int32, RendererObject*>(i, roomObject));
	}

	m_numHairVertices = 0;
	m_numHairIndices = 0;

	// Step 3: prepare moveables
	for (__int32 i = 0; i < MoveablesIds.size(); i++)
	{
		OBJECT_INFO* obj = &Objects[MoveablesIds[i]];

		RendererObject* moveable = new RendererObject(obj->nmeshes, MoveablesIds[i]);
		if (obj->nmeshes != 0)
		{
			for (__int32 j = 0; j < obj->nmeshes; j++)
			{
				__int16* meshPtr = &RawMeshData[RawMeshPointers[obj->meshIndex / 2 + j] / 2];
				RendererMesh* mesh = GetRendererMeshFromTrMesh(meshPtr, j, MoveablesIds[i] == ID_LARA_SKIN_JOINTS,
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
						RendererBucket* jointBucket = jointMesh->Buckets[b1];
						for (__int32 v1 = 0; v1 < jointBucket->Vertices.size(); v1++)
						{
							RendererVertex* jointVertex = &jointBucket->Vertices[v1];
							if (jointVertex->bone != j)
							{
								RendererMesh* skinMesh = objSkin->ObjectMeshes[jointVertex->bone];
								RendererBone* skinBone = objSkin->LinearizedBones[jointVertex->bone];

								for (__int32 b2 = 0; b2 < NUM_BUCKETS; b2++)
								{
									RendererBucket* skinBucket = skinMesh->Buckets[b2];
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
						InitialiseBucketBuffer(mesh->Buckets[n]);
				}
			}

			if (MoveablesIds[i] == ID_HAIR)
			{
				for (__int32 j = 0; j < moveable->ObjectMeshes.size(); j++)
				{
					RendererMesh* mesh = moveable->ObjectMeshes[j];
					for (__int32 n = 0; n < NUM_BUCKETS; n++)
					{
						m_numHairVertices += mesh->Buckets[n]->NumVertices;
						m_numHairIndices += mesh->Buckets[n]->NumIndices;
					}
				}

				m_hairVertices = (RendererVertex*)malloc(m_numHairVertices * sizeof(RendererVertex));
				m_hairIndices = (__int32*)malloc(m_numHairIndices * 4);
			}

			if (MoveablesIds[i] == ID_HAIR && false)
			{
				RendererObject* objSkin = m_moveableObjects[ID_LARA_SKIN];

				for (__int32 j = 0; j < 6; j++)
				{
					RendererMesh* hairMesh = moveable->ObjectMeshes[j];
					RendererBone* hairBone = moveable->LinearizedBones[j];

					for (__int32 b1 = 0; b1 < NUM_BUCKETS; b1++)
					{
						RendererBucket* hairBucket = hairMesh->Buckets[b1];
						for (__int32 v1 = 0; v1 < hairBucket->Vertices.size(); v1++)
						{
							RendererVertex* hairVertex = &hairBucket->Vertices[v1];
							if (hairVertex->bone != j)
							{
								RendererMesh* otherMesh = (j == 0 ? objSkin->ObjectMeshes[14] : moveable->ObjectMeshes[hairVertex->bone]);
								RendererBone* otherBone = (j == 0 ? objSkin->LinearizedBones[14] : moveable->LinearizedBones[hairVertex->bone]);

								for (__int32 b2 = 0; b2 < NUM_BUCKETS; b2++)
								{
									RendererBucket* otherBucket = otherMesh->Buckets[b2];
									for (__int32 v2 = 0; v2 < otherBucket->Vertices.size(); v2++)
									{
										RendererVertex* otherVertex = &otherBucket->Vertices[v2];

										__int32 x1 = hairBucket->Vertices[v1].x + hairBone->GlobalTranslation.x;
										__int32 y1 = hairBucket->Vertices[v1].y + hairBone->GlobalTranslation.y;
										__int32 z1 = hairBucket->Vertices[v1].z + hairBone->GlobalTranslation.z;

										__int32 x2 = otherBucket->Vertices[v2].x + otherBone->GlobalTranslation.x;
										__int32 y2 = otherBucket->Vertices[v2].y + otherBone->GlobalTranslation.y;
										__int32 z2 = otherBucket->Vertices[v2].z + otherBone->GlobalTranslation.z;

										if (abs(x1 - x2) < 2 && abs(y1 - y2) < 2 && abs(z1 - z2) < 2)
										{
											hairVertex->x = otherVertex->x;
											hairVertex->y = otherVertex->y;
											hairVertex->z = otherVertex->z;
										}
									}
								}
							}
						}
					}
				}
			}

			// Initialise buffers
			for (__int32 j = 0; j < obj->nmeshes; j++)
			{
				RendererMesh* mesh = moveable->ObjectMeshes[j];
				for (__int32 n = 0; n < NUM_BUCKETS; n++)
					InitialiseBucketBuffer(mesh->Buckets[n]);
			}
		}

		m_moveableObjects.insert(pair<__int32, RendererObject*>(MoveablesIds[i], moveable));
	}

	// Step 4: prepare static meshes
	for (__int32 i = 0; i < StaticObjectsIds.size(); i++)
	{
		STATIC_INFO* obj = &StaticObjects[StaticObjectsIds[i]];
		RendererObject* staticObject = new RendererObject(1, StaticObjectsIds[i]);

		__int16* meshPtr = &RawMeshData[RawMeshPointers[obj->meshNumber / 2] / 2];
		RendererMesh* mesh = GetRendererMeshFromTrMesh(meshPtr, 0, false, false);

		for (__int32 i = 0; i < NUM_BUCKETS; i++)
			InitialiseBucketBuffer(mesh->Buckets[i]);

		staticObject->ObjectMeshes.push_back(mesh);

		m_staticObjects.insert(pair<__int32, RendererObject*>(StaticObjectsIds[i], staticObject));
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
			rotX* (360.0f / 1024.0f)*0.0174533,
			rotZ* (360.0f / 1024.0f)*0.0174533);
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

void Renderer::BuildAnimationPoseRecursive(RendererObject* obj, __int16** frmptr, D3DXMATRIX* parentTransform, __int16 frac, __int16 rate, RendererBone* bone)
{
	D3DXMATRIX rotation;
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

	D3DXMatrixMultiply(&obj->AnimationTransforms[bone->Index], &rotation, &bone->Transform);
	D3DXMatrixMultiply(&obj->AnimationTransforms[bone->Index], &obj->AnimationTransforms[bone->Index], parentTransform);

	for (int j = 0; j < bone->Children.size(); j++)
	{
		BuildAnimationPoseRecursive(obj, frmptr, &obj->AnimationTransforms[bone->Index], frac, rate, bone->Children[j]);
	}
}

void Renderer::BuildAnimationPose(RendererObject* obj, __int16** frmptr, __int16 frac, __int16 rate)
{
	D3DXVECTOR3 p = D3DXVECTOR3((int)*(frmptr[0] + 6), (int)*(frmptr[0] + 7), (int)*(frmptr[0] + 8));

	D3DXMATRIX rotation;
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

	D3DXMatrixMultiply(&obj->AnimationTransforms[obj->Skeleton->Index], &rotation, &translation);

	for (int j = 0; j < obj->Skeleton->Children.size(); j++)
	{
		BuildAnimationPoseRecursive(obj, frmptr, &obj->AnimationTransforms[obj->Skeleton->Index], frac, rate, obj->Skeleton->Children[j]);
	}
}

void Renderer::CollectRoomsToDraw()
{
	m_roomsToDraw.clear();
	for (__int32 i = 0; i < NumberRooms; i++)
		if (Rooms[i].NumVertices != 0)
			m_roomsToDraw.push_back(i);
}

/*void Renderer::DrawRoom(__int16 index)
{
	RendererObject* obj = m_roomObjects[index];
	RendererMesh* mesh = obj->ObjectMeshes[0];
	ROOM_INFO* room = &Rooms[index];

	m_device->SetStreamSource(0, vertexBuffer, 0, sizeof(RendererVertex));
	m_device->SetIndices(indexBuffer);

}

void Renderer::DrawStaticObject(__int16 roomIndex, __int16 staticIndex)
{

}*/

void Renderer::DrawPrimitives(D3DPRIMITIVETYPE primitiveType, UINT baseVertexIndex, UINT minVertexIndex, UINT numVertices, UINT baseIndex, UINT primitiveCount)
{
	m_numVertices += numVertices;
	m_numTriangles += primitiveCount;

	m_device->DrawIndexedPrimitive(primitiveType, baseVertexIndex, minVertexIndex, numVertices, baseIndex, primitiveCount);
}

void Renderer::DrawRoom(__int32 roomIndex, __int32 bucketIndex)
{
	D3DXMATRIX world;
	UINT cPasses = 1;

	RendererObject* obj = m_roomObjects[roomIndex];
	RendererMesh* mesh = obj->ObjectMeshes[0];
	ROOM_INFO* room = &Rooms[roomIndex];
	RendererBucket* bucket;

	// Draw room geometry first
	bucket = mesh->Buckets[bucketIndex];
	if (bucket->NumVertices == 0)
		return;

	m_device->SetStreamSource(0, bucket->VertexBuffer, 0, sizeof(RendererVertex));
	m_device->SetIndices(bucket->IndexBuffer);

	m_effect->SetBool(m_effect->GetParameterByName(NULL, "EnableVertexColors"), true);

	for (int iPass = 0; iPass < cPasses; iPass++)
	{
		m_effect->BeginPass(iPass);
		D3DXMatrixTranslation(&world, room->x, room->y, room->z);
		m_effect->SetMatrix(m_effect->GetParameterByName(NULL, "World"), &world);
		m_effect->CommitChanges();

		DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0,
			bucket->NumVertices, 0, bucket->NumIndices / 3);

		m_effect->EndPass();
	}

	m_effect->SetBool(m_effect->GetParameterByName(NULL, "EnableVertexColors"), false);
}

void Renderer::DrawStatic(__int32 roomIndex, __int32 staticIndex, __int32 bucketIndex)
{
	D3DXMATRIX world;
	D3DXMATRIX rotation;
	UINT cPasses = 1;

	ROOM_INFO* room = &Rooms[roomIndex];
	MESH_INFO* sobj = &room->mesh[staticIndex];
	RendererObject* obj = m_staticObjects[sobj->staticNumber];
	RendererMesh* mesh = obj->ObjectMeshes[0];
	RendererBucket* bucket = mesh->Buckets[bucketIndex];
	if (bucket->NumVertices == 0)
		return;

	m_device->SetStreamSource(0, bucket->VertexBuffer, 0, sizeof(RendererVertex));
	m_device->SetIndices(bucket->IndexBuffer);

	for (int iPass = 0; iPass < cPasses; iPass++)
	{
		m_effect->BeginPass(iPass);
		D3DXMatrixTranslation(&world, sobj->x, sobj->y, sobj->z);
		D3DXMatrixRotationY(&rotation, TR_ANGLE_TO_RAD(sobj->yRot));
		D3DXMatrixMultiply(&world, &rotation, &world);
		m_effect->SetMatrix(m_effect->GetParameterByName(NULL, "World"), &world);
		m_effect->CommitChanges();

		DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0,
			bucket->NumVertices, 0, bucket->NumIndices / 3);

		m_effect->EndPass();
	}
}

void Renderer::DrawLara(__int32 bucketIndex)
{
	D3DXMATRIX world;
	D3DXMATRIX translation;
	D3DXMATRIX rotation;
	UINT cPasses = 1;

	RendererObject* laraObj = m_moveableObjects[ID_LARA];
	RendererObject* laraSkin = m_moveableObjects[ID_LARA_SKIN];

	ITEM_INFO* item = LaraItem;
	OBJECT_INFO* obj = &Objects[0];

	if (obj->animIndex != -1)
	{
		__int16	*framePtr[2];
		__int32 rate;
		__int32 frac = GetFrame_D2(item, framePtr, &rate);
		BuildAnimationPose(laraObj, framePtr, frac, rate);
	}

	for (__int32 i = 0; i < laraObj->ObjectMeshes.size(); i++)
	{
		RendererMesh* mesh = laraSkin->ObjectMeshes[i];
		RendererBucket* bucket = mesh->Buckets[bucketIndex];

		if (bucket->NumVertices == 0)
			continue;

		D3DXMatrixTranslation(&translation, LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
		D3DXMatrixRotationY(&rotation, TR_ANGLE_TO_RAD(LaraItem->pos.yRot));
		D3DXMatrixMultiply(&world, &rotation, &translation);
		//D3DXMatrixMultiply(&world, &laraObj->AnimationTransforms[i], &world);

		m_effect->SetBool(m_effect->GetParameterByName(NULL, "UseSkinning"), true);
		m_effect->SetMatrixArray(m_effect->GetParameterByName(NULL, "Bones"), laraObj->AnimationTransforms, laraObj->ObjectMeshes.size());

		m_device->SetStreamSource(0, bucket->VertexBuffer, 0, sizeof(RendererVertex));
		m_device->SetIndices(bucket->IndexBuffer);

		for (int iPass = 0; iPass < cPasses; iPass++)
		{
			m_effect->BeginPass(iPass);
			m_effect->SetMatrix(m_effect->GetParameterByName(NULL, "World"), &world);
			m_effect->CommitChanges();

			DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0,
				bucket->NumVertices, 0, bucket->NumIndices / 3);

			m_effect->EndPass();
		}

		if (m_moveableObjects.find(ID_LARA_SKIN_JOINTS) != m_moveableObjects.end())
		{
			RendererObject* laraSkinJoints = m_moveableObjects[ID_LARA_SKIN_JOINTS];
			RendererMesh* jointMesh = laraSkinJoints->ObjectMeshes[i];
			bucket = jointMesh->Buckets[bucketIndex];

			m_device->SetStreamSource(0, bucket->VertexBuffer, 0, sizeof(RendererVertex));
			m_device->SetIndices(bucket->IndexBuffer);

			for (int iPass = 0; iPass < cPasses; iPass++)
			{
				m_effect->BeginPass(iPass);
				m_effect->SetMatrix(m_effect->GetParameterByName(NULL, "World"), &world);
				m_effect->CommitChanges();

				DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0,
					bucket->NumVertices, 0, bucket->NumIndices / 3);

				m_effect->EndPass();
			}
		}

		m_effect->SetBool(m_effect->GetParameterByName(NULL, "UseSkinning"), false);
	}

	// Draw Lara's hairs
	if (m_moveableObjects.find(ID_HAIR) != m_moveableObjects.end())
	{
		RendererObject* hairsObj = m_moveableObjects[ID_HAIR];

		D3DXMATRIX lastMatrix;
		D3DXMATRIX hairMatrix;
		D3DXMATRIX identity;

		D3DXMatrixIdentity(&lastMatrix);
		D3DXMatrixIdentity(&identity);

		/*D3DXMATRIX matrices[7];
		for (__int32 i = 0; i < 6; i++)
			D3DXMatrixTranslation(&matrices[0], Hairs[i].pos.xPos, Hairs[i].pos.yPos, Hairs[i].pos.zPos);
		matrices[6] = m_moveableObjects[ID_LARA]->AnimationTransforms[14];*/

		D3DXVECTOR3 parentVertices[6][4];
		D3DXMATRIX headMatrix;
		RendererObject* objSkin = m_moveableObjects[ID_LARA_SKIN];
		RendererObject* objLara = m_moveableObjects[ID_LARA];
		RendererMesh* parentMesh = objSkin->ObjectMeshes[14];
		RendererBone* parentBone = objSkin->LinearizedBones[14];

		D3DXMatrixTranslation(&translation, LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
		D3DXMatrixRotationY(&rotation, TR_ANGLE_TO_RAD(LaraItem->pos.yRot));
		D3DXMatrixMultiply(&world, &rotation, &translation);
		D3DXMatrixMultiply(&world, &objLara->AnimationTransforms[14], &world);

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
			RendererBucket* bucket = mesh->Buckets[bucketIndex];

			D3DXMatrixTranslation(&translation, Hairs[i].pos.xPos, Hairs[i].pos.yPos, Hairs[i].pos.zPos);
			D3DXMatrixRotationYawPitchRoll(&rotation, TR_ANGLE_TO_RAD(Hairs[i].pos.yRot),
				TR_ANGLE_TO_RAD(Hairs[i].pos.xRot), TR_ANGLE_TO_RAD(Hairs[i].pos.zRot));
			D3DXMatrixMultiply(&world, &rotation, &translation);

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

					D3DXVec3Transform(&out, &in, &world);

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
				m_hairIndices[lastIndex] = baseVertex+bucket->Indices[j];
				lastIndex++;
			}
		}

		D3DXMatrixIdentity(&world);

		//for (__int32 i = 0; i < lastVertex; i++)
		//	printf("%d %d %d\n", (__int32)m_hairVertices[i].x, (__int32)m_hairVertices[i].y, (__int32)m_hairVertices[i].z);

		//for (__int32 i = 0; i < lastVertex; i++)
		//	printf("%d\n", m_hairIndices[i]);

		for (int iPass = 0; iPass < cPasses; iPass++)
		{
			m_effect->BeginPass(iPass);
			m_effect->SetMatrix(m_effect->GetParameterByName(NULL, "World"), &world);
			m_effect->CommitChanges();

			HRESULT res = m_device->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0, lastVertex, lastIndex / 3,
				m_hairIndices, D3DFMT_INDEX32, m_hairVertices, sizeof(RendererVertex));

			m_effect->EndPass();
		}

		/*for (__int32 i = 0; i < 6; i++)
		{
			RendererMesh* mesh = hairsObj->ObjectMeshes[i];
			RendererBucket* bucket = mesh->Buckets[bucketIndex];

			D3DXMatrixTranslation(&translation, Hairs[i].pos.xPos, Hairs[i].pos.yPos, Hairs[i].pos.zPos);
			D3DXMatrixRotationYawPitchRoll(&rotation, TR_ANGLE_TO_RAD(Hairs[i].pos.yRot), 
				TR_ANGLE_TO_RAD(Hairs[i].pos.xRot), TR_ANGLE_TO_RAD(Hairs[i].pos.zRot));
			D3DXMatrixMultiply(&world, &rotation, &translation);

			m_device->SetStreamSource(0, bucket->VertexBuffer, 0, sizeof(RendererVertex));
			m_device->SetIndices(bucket->IndexBuffer);

			for (int iPass = 0; iPass < cPasses; iPass++)
			{
				m_effect->BeginPass(iPass);
				m_effect->SetMatrix(m_effect->GetParameterByName(NULL, "World"), &world);
				m_effect->CommitChanges();

				DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0,
					bucket->NumVertices, 0, bucket->NumIndices / 3);

				m_effect->EndPass();
			}
		}*/

		m_effect->SetBool(m_effect->GetParameterByName(NULL, "UseSkinning"), false);
	}
}

void Renderer::DrawItem(__int32 itemIndex, __int32 bucketIndex)
{
	D3DXMATRIX world;
	D3DXMATRIX translation;
	D3DXMATRIX rotation;
	UINT cPasses = 1;

	ITEM_INFO* item = &Items[itemIndex];
	if (item->objectNumber == ID_LARA)
		return;

	if (itemIndex == 100)
		printf("%d %d %d\n", item->pos.xPos, item->pos.yPos, item->pos.zPos);

	OBJECT_INFO* obj = &Objects[item->objectNumber];

	RendererObject* moveableObj = m_moveableObjects[item->objectNumber];

	if (obj->animIndex != -1)
	{
		__int16	*framePtr[2];
		__int32 rate;
		__int32 frac = GetFrame_D2(item, framePtr, &rate);
		BuildAnimationPose(moveableObj, framePtr, frac, rate);
	}

	for (__int32 i = 0; i < moveableObj->ObjectMeshes.size(); i++)
	{
		RendererMesh* mesh = moveableObj->ObjectMeshes[i];
		RendererBucket* bucket = mesh->Buckets[bucketIndex];
		if (bucket->NumVertices == 0)
			continue;

		D3DXMatrixTranslation(&translation, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		D3DXMatrixRotationY(&rotation, TR_ANGLE_TO_RAD(item->pos.yRot));
		D3DXMatrixMultiply(&world, &rotation, &translation);
		if (obj->animIndex != -1)
			D3DXMatrixMultiply(&world, &moveableObj->AnimationTransforms[i], &world);
		else
			D3DXMatrixMultiply(&world, &moveableObj->BindPoseTransforms[i], &world);

		m_device->SetStreamSource(0, bucket->VertexBuffer, 0, sizeof(RendererVertex));
		m_device->SetIndices(bucket->IndexBuffer);

		for (int iPass = 0; iPass < cPasses; iPass++)
		{
			m_effect->BeginPass(iPass);
			m_effect->SetMatrix(m_effect->GetParameterByName(NULL, "World"), &world);
			m_effect->CommitChanges();

			DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0,
				bucket->NumVertices, 0, bucket->NumIndices / 3);

			m_effect->EndPass();
		}
	}
}

void Renderer::DrawSky()
{
	D3DXMATRIX world;
	D3DXMATRIX scale;
	UINT cPasses = 1;

	RendererObject* horizonObj = m_moveableObjects[ID_HORIZON];
	RendererMesh* mesh = horizonObj->ObjectMeshes[0];

	for (__int32 i = 0; i < NUM_BUCKETS; i++)
	{
		RendererBucket* bucket = mesh->Buckets[i];

		m_device->SetStreamSource(0, bucket->VertexBuffer, 0, sizeof(RendererVertex));
		m_device->SetIndices(bucket->IndexBuffer);

		for (int iPass = 0; iPass < cPasses; iPass++)
		{
			m_effect->BeginPass(iPass);

			D3DXMatrixTranslation(&world, LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
			D3DXMatrixScaling(&scale, 16.0f, 16.0f, 16.0f);
			D3DXMatrixMultiply(&world, &scale, &world);

			m_effect->SetMatrix(m_effect->GetParameterByName(NULL, "World"), &world);
			m_effect->CommitChanges();

			DrawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0,
				bucket->NumVertices, 0, bucket->NumIndices / 3);

			m_effect->EndPass();
		}
	}

	m_device->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);
}

__int32 Renderer::DrawScene()
{
	D3DXMATRIX translation;
	D3DXMATRIX rotation;
	D3DXMATRIX scale;
	D3DXMATRIX world;

	m_numLines = 0;

	// Clip and collect rooms
	CollectRoomsToDraw();

	// Update all animations
	/*for (__int32 i = 0; i < NumItems; i++)
	{
	ITEM_INFO* item = &Items[i];
	OBJECT_INFO* obj = &Objects[item->objectNumber];
	RendererObject* moveableObj = m_moveableObjects[item->objectNumber];

	if (obj->animIndex != -1)
	{
	__int16	*framePtr[2];
	__int32 rate;
	__int32 frac = GetFrame_D2(item, framePtr, &rate);
	BuildAnimationPose(moveableObj, framePtr, frac, rate);
	}
	}*/

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

	m_effect->SetTexture(m_effect->GetParameterByName(NULL, "ModelTexture"), m_textureAtlas);
	m_effect->SetMatrix(m_effect->GetParameterByName(NULL, "View"), &ViewMatrix);
	m_effect->SetMatrix(m_effect->GetParameterByName(NULL, "Projection"), &ProjectionMatrix);

	m_device->SetVertexDeclaration(m_vertexDeclaration);

	UINT cPasses = 1;
	m_effect->Begin(&cPasses, 0);

	if (m_fadeTimer < 30)
		m_effect->SetFloat(m_effect->GetParameterByName(NULL, "FadeTimer"), m_fadeTimer++);
	
	if (UseSpotCam && SpotCam[CurrentSplineCamera].flags & 0x400)
		m_effect->SetBool(m_effect->GetParameterByName(NULL, "CinematicMode"), true);
	else
		m_effect->SetBool(m_effect->GetParameterByName(NULL, "CinematicMode"), false);

	// Draw skybox if needed
	if (m_moveableObjects.find(ID_HORIZON) != m_moveableObjects.end())
	{
		DrawSky();
	}

	// Solid faces buckets
	for (__int32 i = 0; i < m_roomsToDraw.size(); i++)
	{
		RendererObject* obj = m_roomObjects[m_roomsToDraw[i]];
		RendererMesh* mesh = obj->ObjectMeshes[0];

		if (mesh->Buckets[RENDERBUCKET_SOLID]->NumVertices != 0)
			DrawRoom(m_roomsToDraw[i], RENDERBUCKET_SOLID);

		if (mesh->Buckets[RENDERBUCKET_SOLID_DS]->NumVertices != 0)
		{
			m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			DrawRoom(m_roomsToDraw[i], RENDERBUCKET_SOLID_DS);
			m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		}

		// Draw static objects
		ROOM_INFO* room = &Rooms[m_roomsToDraw[i]];
		if (room->numMeshes != 0)
		{
			for (__int32 j = 0; j < room->numMeshes; j++)
			{
				MESH_INFO* sobj = &room->mesh[j];
				RendererObject* staticObj = m_staticObjects[sobj->staticNumber];
				RendererMesh* staticMesh = obj->ObjectMeshes[0];

				//if (staticMesh->Buckets[RENDERBUCKET_SOLID]->NumVertices != 0)
				//	DrawStatic(m_roomsToDraw[i], j, RENDERBUCKET_SOLID);

				if (staticMesh->Buckets[RENDERBUCKET_SOLID_DS]->NumVertices != 0)
				{
					m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
					//DrawStatic(m_roomsToDraw[i], j, RENDERBUCKET_SOLID_DS);
					m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
				}
			}
		}
	}

	// Draw Lara
	DrawLara(RENDERBUCKET_SOLID);

	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	DrawLara(RENDERBUCKET_SOLID_DS);

	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	// Draw items
	for (__int32 k = 0; k < NumItems; k++)
	{
		DrawItem(k, RENDERBUCKET_SOLID);

		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		DrawItem(k, RENDERBUCKET_SOLID_DS);

		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	}

	// Transparent and alpha test faces
	for (__int32 i = 0; i < m_roomsToDraw.size(); i++)
	{
		DrawRoom(m_roomsToDraw[i], RENDERBUCKET_ALPHA_TEST);
		DrawRoom(m_roomsToDraw[i], RENDERBUCKET_TRANSPARENT);

		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		DrawRoom(m_roomsToDraw[i], RENDERBUCKET_SOLID_DS);
		DrawRoom(m_roomsToDraw[i], RENDERBUCKET_TRANSPARENT_DS);

		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

		// Draw static objects
		ROOM_INFO* room = &Rooms[m_roomsToDraw[i]];
		if (room->numMeshes != 0)
		{
			for (__int32 j = 0; j < room->numMeshes; j++)
			{
				DrawStatic(m_roomsToDraw[i], j, RENDERBUCKET_ALPHA_TEST);


				DrawStatic(m_roomsToDraw[i], j, RENDERBUCKET_TRANSPARENT);

				m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
				DrawStatic(m_roomsToDraw[i], j, RENDERBUCKET_ALPHA_TEST_DS);
				DrawStatic(m_roomsToDraw[i], j, RENDERBUCKET_TRANSPARENT_DS);

				m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
			}
		}
	}

	// Draw Lara
	DrawLara(RENDERBUCKET_ALPHA_TEST);
	DrawLara(RENDERBUCKET_TRANSPARENT);

	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	DrawLara(RENDERBUCKET_ALPHA_TEST_DS);
	DrawLara(RENDERBUCKET_TRANSPARENT_DS);

	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	// Draw items
	for (__int32 k = 0; k < NumItems; k++)
	{
		//m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCCOLOR);
		//m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVDESTCOLOR);
		//m_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

		DrawItem(k, RENDERBUCKET_ALPHA_TEST);
		DrawItem(k, RENDERBUCKET_TRANSPARENT);

		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		DrawItem(k, RENDERBUCKET_ALPHA_TEST_DS);
		DrawItem(k, RENDERBUCKET_TRANSPARENT_DS);

		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	}

	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	m_effect->End();

	DrawGameInfo();
	DrawDebugInfo();

	DrawAllLines2D();

	//DoPauseMenu(0);

	m_device->EndScene();
	m_device->Present(NULL, NULL, NULL, NULL);

	return 1;
}

__int32 Renderer::DumpScreen()
{
	LPDIRECT3DSURFACE9 oldTarget = NULL;
	m_device->GetRenderTarget(0, &oldTarget);

	LPDIRECT3DSURFACE9 surface;
	m_renderTarget->GetSurfaceLevel(0, &surface);
	m_device->SetRenderTarget(0, surface);
	surface->Release();

	LPDIRECT3DSURFACE9 oldDepth = NULL;
	m_device->GetDepthStencilSurface(&oldDepth);
	m_device->SetDepthStencilSurface(m_renderTargetDepth);

	DrawScene();

	m_device->SetRenderTarget(0, oldTarget);
	m_device->SetDepthStencilSurface(oldDepth);

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

	float factor = ScreenWidth / 640.0f;

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

	PrintString(400, 500, (char*)"This is a text message for testing the new font system", 0xFFFFFFFF, 
				PRINTSTRING_CENTER | PRINTSTRING_BLINK);
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
	sprintf_s(&m_message[0], 255, "LaraItem.currentAnimState = %d", LaraItem->currentAnimState);
	DrawMessage(m_font, 10, 80, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "LaraItem.animNumber = %d", LaraItem->animNumber);
	DrawMessage(m_font, 10, 90, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "LaraItem.frameNumber = %d", LaraItem->frameNumber);
	DrawMessage(m_font, 10, 100, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "LaraItem.roomNumber = %d", LaraItem->roomNumber);
	DrawMessage(m_font, 10, 110, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "LaraItem.pos = < %d %d %d >", LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
	DrawMessage(m_font, 10, 120, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "NumVertices = %d", m_numVertices);
	DrawMessage(m_font, 10, 130, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "NumTriangles = %d", m_numTriangles);
	DrawMessage(m_font, 10, 140, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Camera.pos = < %d %d %d >", Camera.pos.x, Camera.pos.y, Camera.pos.z);
	DrawMessage(m_font, 10, 150, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Camera.target = < %d %d %d >", Camera.target.x, Camera.target.y, Camera.target.z);
	DrawMessage(m_font, 10, 160, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "CurrentLevel: %d", CurrentLevel);
	DrawMessage(m_font, 10, 170, 255, 255, 255, 255, m_message);

	if (UseSpotCam)
	{
		sprintf_s(&m_message[0], 255, "Spotcam.camera = %d    Spotcam.flags = %d     Current: %d", SpotCam[CurrentSplineCamera].camera, SpotCam[CurrentSplineCamera].flags, CurrentSplineCamera);
		DrawMessage(m_font, 10, 180, 255, 255, 255, 255, m_message);
	}
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
	m_sprite->Draw(m_renderTarget, &rect, &D3DXVECTOR3(0.0f, 0.0f, 0.0f), &D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DCOLOR_ARGB(255, 128, 128, 128));
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
	m_sprite->Draw(m_renderTarget, &rect, &D3DXVECTOR3(0.0f, 0.0f, 0.0f), &D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DCOLOR_ARGB(255, 128, 128, 128));
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