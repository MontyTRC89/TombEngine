#include "Renderer.h"

#include "Structs.h"
#include "Enums.h"
#include "RendererBucket.h"
#include "RendererMesh.h"
#include "RendererObject.h"

#include <stdio.h>
#include <stack> 
#include <chrono> 
#include <thread>

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
#include "..\Game\effect2.h"
 
using ns = chrono::nanoseconds;
using get_time = chrono::steady_clock;

extern GameScript* g_Script;

Renderer::Renderer()
{
	memset(m_normalLaraSkinJointRemap, -1, 15 * 32 * 2);
	memset(m_youngLaraSkinJointRemap, -1, 15 * 32 * 2);

	// Normal Lara
	m_normalLaraSkinJointRemap[1][0] = 0;
	m_normalLaraSkinJointRemap[1][1] = 0;
	m_normalLaraSkinJointRemap[1][2] = 0;
	m_normalLaraSkinJointRemap[1][3] = 0;
	m_normalLaraSkinJointRemap[1][4] = 0;
	m_normalLaraSkinJointRemap[1][5] = 0;

	m_normalLaraSkinJointRemap[2][0] = 1;
	m_normalLaraSkinJointRemap[2][1] = 1;
	m_normalLaraSkinJointRemap[2][2] = 1;
	m_normalLaraSkinJointRemap[2][3] = 1;
	m_normalLaraSkinJointRemap[2][4] = 1;

	m_normalLaraSkinJointRemap[3][4] = 2;
	m_normalLaraSkinJointRemap[3][5] = 2;
	m_normalLaraSkinJointRemap[3][6] = 2;
	m_normalLaraSkinJointRemap[3][7] = 2;

	m_normalLaraSkinJointRemap[4][0] = 0;
	m_normalLaraSkinJointRemap[4][1] = 0;
	m_normalLaraSkinJointRemap[4][2] = 0;
	m_normalLaraSkinJointRemap[4][3] = 0;
	m_normalLaraSkinJointRemap[4][4] = 0;
	m_normalLaraSkinJointRemap[4][5] = 0;

	m_normalLaraSkinJointRemap[5][0] = 4;
	m_normalLaraSkinJointRemap[5][1] = 4;
	m_normalLaraSkinJointRemap[5][2] = 4;
	m_normalLaraSkinJointRemap[5][3] = 4;
	m_normalLaraSkinJointRemap[5][4] = 4;

	m_normalLaraSkinJointRemap[6][4] = 5;
	m_normalLaraSkinJointRemap[6][5] = 5;
	m_normalLaraSkinJointRemap[6][6] = 5;
	m_normalLaraSkinJointRemap[6][7] = 5;

	m_normalLaraSkinJointRemap[7][0] = 0;
	m_normalLaraSkinJointRemap[7][1] = 0;
	m_normalLaraSkinJointRemap[7][2] = 0;
	m_normalLaraSkinJointRemap[7][3] = 0;
	m_normalLaraSkinJointRemap[7][4] = 0;
	m_normalLaraSkinJointRemap[7][5] = 0;

	m_normalLaraSkinJointRemap[8][6] = 7;
	m_normalLaraSkinJointRemap[8][7] = 7;
	m_normalLaraSkinJointRemap[8][8] = 7;
	m_normalLaraSkinJointRemap[8][9] = 7;
	m_normalLaraSkinJointRemap[8][10] = 7;
	m_normalLaraSkinJointRemap[8][11] = 7;

	m_normalLaraSkinJointRemap[9][5] = 8;
	m_normalLaraSkinJointRemap[9][6] = 8;
	m_normalLaraSkinJointRemap[9][7] = 8;
	m_normalLaraSkinJointRemap[9][8] = 8;
	m_normalLaraSkinJointRemap[9][9] = 8;

	m_normalLaraSkinJointRemap[10][0] = 9;
	m_normalLaraSkinJointRemap[10][1] = 9;
	m_normalLaraSkinJointRemap[10][2] = 9;
	m_normalLaraSkinJointRemap[10][3] = 9;
	m_normalLaraSkinJointRemap[10][4] = 9;

	m_normalLaraSkinJointRemap[11][6] = 7;
	m_normalLaraSkinJointRemap[11][7] = 7;
	m_normalLaraSkinJointRemap[11][8] = 7;
	m_normalLaraSkinJointRemap[11][9] = 7;
	m_normalLaraSkinJointRemap[11][10] = 7;
	m_normalLaraSkinJointRemap[11][11] = 7;

	m_normalLaraSkinJointRemap[12][5] = 11;
	m_normalLaraSkinJointRemap[12][6] = 11;
	m_normalLaraSkinJointRemap[12][7] = 11;
	m_normalLaraSkinJointRemap[12][8] = 11;
	m_normalLaraSkinJointRemap[12][9] = 11;

	m_normalLaraSkinJointRemap[13][0] = 12;
	m_normalLaraSkinJointRemap[13][1] = 12;
	m_normalLaraSkinJointRemap[13][2] = 12;
	m_normalLaraSkinJointRemap[13][3] = 12;
	m_normalLaraSkinJointRemap[13][4] = 12;

	m_normalLaraSkinJointRemap[14][6] = 7;
	m_normalLaraSkinJointRemap[14][7] = 7;
	m_normalLaraSkinJointRemap[14][8] = 7;
	m_normalLaraSkinJointRemap[14][9] = 7;
	m_normalLaraSkinJointRemap[14][10] = 7;
	m_normalLaraSkinJointRemap[14][11] = 7;

	// Young Lara
	m_youngLaraSkinJointRemap[1][0] = 0; // Left up leg
	m_youngLaraSkinJointRemap[1][1] = 0;
	m_youngLaraSkinJointRemap[1][2] = 0;
	m_youngLaraSkinJointRemap[1][3] = 0;
	m_youngLaraSkinJointRemap[1][4] = 0;
	m_youngLaraSkinJointRemap[1][5] = 0;

	m_youngLaraSkinJointRemap[2][0] = 1; // Bottom left leg
	m_youngLaraSkinJointRemap[2][1] = 1;
	m_youngLaraSkinJointRemap[2][2] = 1;
	m_youngLaraSkinJointRemap[2][3] = 1;
	m_youngLaraSkinJointRemap[2][4] = 1;

	m_youngLaraSkinJointRemap[3][0] = 2; // Left foot
	m_youngLaraSkinJointRemap[3][1] = 2;
	m_youngLaraSkinJointRemap[3][2] = 2;
	m_youngLaraSkinJointRemap[3][3] = 2;

	m_youngLaraSkinJointRemap[4][6] = 0; // Right upper leg
	m_youngLaraSkinJointRemap[4][7] = 0;
	m_youngLaraSkinJointRemap[4][8] = 0;
	m_youngLaraSkinJointRemap[4][9] = 0;
	m_youngLaraSkinJointRemap[4][10] = 0;
	m_youngLaraSkinJointRemap[4][11] = 0;

	m_youngLaraSkinJointRemap[5][0] = 4; // Right bottom leg
	m_youngLaraSkinJointRemap[5][1] = 4;
	m_youngLaraSkinJointRemap[5][2] = 4;
	m_youngLaraSkinJointRemap[5][3] = 4;
	m_youngLaraSkinJointRemap[5][4] = 4;

	m_youngLaraSkinJointRemap[6][0] = 5; // Right foot
	m_youngLaraSkinJointRemap[6][1] = 5;
	m_youngLaraSkinJointRemap[6][2] = 5;
	m_youngLaraSkinJointRemap[6][3] = 5;

	m_youngLaraSkinJointRemap[7][0] = 0; // Torso
	m_youngLaraSkinJointRemap[7][1] = 0;
	m_youngLaraSkinJointRemap[7][2] = 0;
	m_youngLaraSkinJointRemap[7][3] = 0;
	m_youngLaraSkinJointRemap[7][4] = 0;
	m_youngLaraSkinJointRemap[7][5] = 0;

	m_youngLaraSkinJointRemap[8][0] = 7; // Left upper arm
	m_youngLaraSkinJointRemap[8][1] = 7;
	m_youngLaraSkinJointRemap[8][2] = 7;
	m_youngLaraSkinJointRemap[8][3] = 7;
	m_youngLaraSkinJointRemap[8][4] = 7;
	m_youngLaraSkinJointRemap[8][5] = 7;

	m_youngLaraSkinJointRemap[9][5] = 8; // Left bottom arm
	m_youngLaraSkinJointRemap[9][6] = 8;
	m_youngLaraSkinJointRemap[9][7] = 8;
	m_youngLaraSkinJointRemap[9][8] = 8;
	m_youngLaraSkinJointRemap[9][9] = 8;

	m_youngLaraSkinJointRemap[10][0] = 9; // Left hand
	m_youngLaraSkinJointRemap[10][1] = 9;
	m_youngLaraSkinJointRemap[10][2] = 9;
	m_youngLaraSkinJointRemap[10][3] = 9;
	m_youngLaraSkinJointRemap[10][4] = 9;

	m_youngLaraSkinJointRemap[11][0] = 7; // Right upper arm
	m_youngLaraSkinJointRemap[11][1] = 7;
	m_youngLaraSkinJointRemap[11][2] = 7;
	m_youngLaraSkinJointRemap[11][3] = 7;
	m_youngLaraSkinJointRemap[11][4] = 7;
	m_youngLaraSkinJointRemap[11][5] = 7;

	m_youngLaraSkinJointRemap[12][5] = 11; // Right low arm
	m_youngLaraSkinJointRemap[12][6] = 11;
	m_youngLaraSkinJointRemap[12][7] = 11;
	m_youngLaraSkinJointRemap[12][8] = 11;
	m_youngLaraSkinJointRemap[12][9] = 11;

	m_youngLaraSkinJointRemap[13][0] = 12; // Right arm
	m_youngLaraSkinJointRemap[13][1] = 12;
	m_youngLaraSkinJointRemap[13][2] = 12;
	m_youngLaraSkinJointRemap[13][3] = 12;
	m_youngLaraSkinJointRemap[13][4] = 12;

	m_youngLaraSkinJointRemap[14][0] = 7; // Head
	m_youngLaraSkinJointRemap[14][1] = 7;
	m_youngLaraSkinJointRemap[14][2] = 7;
	m_youngLaraSkinJointRemap[14][3] = 7;
	m_youngLaraSkinJointRemap[14][4] = 7;
	m_youngLaraSkinJointRemap[14][5] = 7;

	m_fullscreenQuadVertices[0].x = -1;
	m_fullscreenQuadVertices[0].y = 1;
	m_fullscreenQuadVertices[0].z = 1;
	m_fullscreenQuadVertices[0].u = 0;
	m_fullscreenQuadVertices[0].v = 0;

	m_fullscreenQuadVertices[1].x = 1;
	m_fullscreenQuadVertices[1].y = 1;
	m_fullscreenQuadVertices[1].z = 1;
	m_fullscreenQuadVertices[1].u = 1;
	m_fullscreenQuadVertices[1].v = 0;

	m_fullscreenQuadVertices[2].x = -1;
	m_fullscreenQuadVertices[2].y = -1;
	m_fullscreenQuadVertices[2].z = 1;
	m_fullscreenQuadVertices[2].u = 0;
	m_fullscreenQuadVertices[2].v = 1;

	m_fullscreenQuadVertices[3].x = 1;
	m_fullscreenQuadVertices[3].y = -1;
	m_fullscreenQuadVertices[3].z = 1;
	m_fullscreenQuadVertices[3].u = 1;
	m_fullscreenQuadVertices[3].v = 1;

	m_fullscreenQuadIndices[0] = 0;
	m_fullscreenQuadIndices[1] = 3;
	m_fullscreenQuadIndices[2] = 2;
	m_fullscreenQuadIndices[3] = 0;
	m_fullscreenQuadIndices[4] = 1;
	m_fullscreenQuadIndices[5] = 3;
}

Renderer::~Renderer()
{
	DX_RELEASE(m_d3D);
	DX_RELEASE(m_device);
	DX_RELEASE(m_debugFont);
	DX_RELEASE(m_gameFont);

	FreeRendererData();

	delete m_lines2D;
	delete m_lightBuffer;
	delete m_normalBuffer;
	delete m_vertexLightBuffer;
	delete m_colorBuffer;
	delete m_outputBuffer;
	delete m_depthBuffer;
	delete m_shaderClearGBuffer;
	delete m_shaderFillGBuffer;
	delete m_postprocessBuffer;
	delete m_shaderClearGBuffer;
	delete m_shaderFillGBuffer;
	delete m_shaderLight;
	delete m_shaderCombine;
	delete m_shaderBasic;
	delete m_shaderDepth;
	delete m_shaderReconstructZBuffer;
	delete m_shaderSprites;
	delete m_shaderRain;
	delete m_shaderTransparent;
	delete m_sphereMesh;
	delete m_shadowMap;
	delete m_renderTarget;
	delete m_quad;
	delete m_skyQuad;
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
	D3DXCreateTextureFromFileEx(m_device, g_Script->GetLevel(0)->Background.c_str(), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0,
								D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
								D3DCOLOR_XRGB(255, 255, 255), NULL, NULL, &m_titleScreen);

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
	m_dxSprite = NULL;
	res = D3DXCreateSprite(m_device, &m_dxSprite);
	if (res != S_OK)
		return false;

	// Initialise line objects
	m_lines2D = (RendererLine2D*)malloc(MAX_LINES_2D * sizeof(RendererLine2D));
	m_dxLine = NULL;
	res = D3DXCreateLine(m_device, &m_dxLine);
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

	m_shaderClearGBuffer = new Shader(m_device, (char*)"Shaders\\ClearGBuffer.fx");
	m_shaderClearGBuffer->Compile();
	if (m_shaderClearGBuffer->GetEffect() == NULL)
		return false;

	m_shaderFillGBuffer = new Shader(m_device, (char*)"Shaders\\FillGBuffer.fx");
	m_shaderFillGBuffer->Compile();
	if (m_shaderFillGBuffer->GetEffect() == NULL)
		return false;

	m_shaderLight = new Shader(m_device, (char*)"Shaders\\Light.fx");
	m_shaderLight->Compile();
	if (m_shaderLight->GetEffect() == NULL)
		return false;

	m_shaderCombine = new Shader(m_device, (char*)"Shaders\\CombineFinal.fx");
 	m_shaderCombine->Compile();
	if (m_shaderCombine->GetEffect() == NULL)
		return false;	

	m_shaderBasic = new Shader(m_device, (char*)"Shaders\\Basic.fx");
	m_shaderBasic->Compile();
	if (m_shaderBasic->GetEffect() == NULL)
		return false;

	m_shaderRain = new Shader(m_device, (char*)"Shaders\\Rain.fx");
	m_shaderRain->Compile();
	if (m_shaderRain->GetEffect() == NULL)
		return false;

	m_shaderReconstructZBuffer = new Shader(m_device, (char*)"Shaders\\ReconstructZBuffer.fx");
	m_shaderReconstructZBuffer->Compile();
	if (m_shaderReconstructZBuffer->GetEffect() == NULL)
		return false;

	m_shaderSprites = new Shader(m_device, (char*)"Shaders\\Sprites.fx");
	m_shaderSprites->Compile();
	if (m_shaderSprites->GetEffect() == NULL)
		return false;

	m_shaderRain = new Shader(m_device, (char*)"Shaders\\Rain.fx");
	m_shaderRain->Compile();
	if (m_shaderRain->GetEffect() == NULL)
		return false;

	m_shaderTransparent = new Shader(m_device, (char*)"Shaders\\Transparent.fx");
	m_shaderTransparent->Compile();
	if (m_shaderTransparent->GetEffect() == NULL)
		return false;

	m_shaderDepth = new Shader(m_device, (char*)"Shaders\\Depth.fx");
	m_shaderDepth->Compile();
	if (m_shaderDepth->GetEffect() == NULL)
		return false;
	   
	m_sphereMesh = new RendererSphere(m_device, 1280.0f, 6);
	m_quad = new RendererQuad(m_device, 1024.0f);
	m_skyQuad = new RendererQuad(m_device, 9728.0f);
	 
	m_halfPixelX = 0.5f / (float)ScreenWidth;
	m_halfPixelY = 0.5f / (float)ScreenHeight;

	// Load caustics
	char* causticsNames[NUM_CAUSTICS_TEXTURES] = { 
		"CausticsRender_001.bmp",
		"CausticsRender_002.bmp", 
		"CausticsRender_003.bmp",
		"CausticsRender_004.bmp", 
		"CausticsRender_005.bmp", 
		"CausticsRender_006.bmp",
		"CausticsRender_007.bmp", 
		"CausticsRender_008.bmp",
		"CausticsRender_009.bmp",
		"CausticsRender_010.bmp", 
		"CausticsRender_011.bmp",
		"CausticsRender_012.bmp", 
		"CausticsRender_013.bmp", 
		"CausticsRender_014.bmp",
		"CausticsRender_015.bmp", 
		"CausticsRender_016.bmp" 
	};
	  
	for (__int32 i = 0; i < NUM_CAUSTICS_TEXTURES; i++)
	{
		D3DXCreateTextureFromFileEx(m_device, causticsNames[i], D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, 0, 0,
			D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
			D3DCOLOR_XRGB(255, 255, 255), NULL, NULL, &m_caustics[i]);
	}

	// Initialise buffer for sprites and lines
	res = m_device->CreateVertexBuffer(NUM_SPRITES_PER_BUCKET * 4 * sizeof(RendererVertex), D3DUSAGE_WRITEONLY,
		0, D3DPOOL_MANAGED, &m_spritesVertexBuffer, NULL);
	if (res != S_OK)
		return false;
				      
	res = m_device->CreateIndexBuffer(NUM_SPRITES_PER_BUCKET * 6 * 4, D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_MANAGED,
		&m_spritesIndexBuffer, NULL);
	if (res != S_OK)
		return false;

	res = m_device->CreateVertexBuffer(NUM_LINES_PER_BUCKET * 2 * sizeof(RendererVertex), D3DUSAGE_WRITEONLY,
		0, D3DPOOL_MANAGED, &m_linesVertexBuffer, NULL);
	if (res != S_OK)
		return false;

	printf("DX initialised\n");

	// Initialise last non-DX stuff
	m_fadeTimer = 0;
	m_pickupRotation = 0;
	m_dynamicLights.reserve(1024);
	m_lights.reserve(1024);
	m_itemsToDraw.reserve(1024);
	m_spritesToDraw.reserve(NUM_SPRITES_PER_BUCKET * 4);
	m_lines3DToDraw.reserve(NUM_LINES_PER_BUCKET * 4);
	m_lines3DVertices.reserve(NUM_LINES_PER_BUCKET * 2);
	m_firstWeather = true;
	
	resetBlink();

	return true;
}

void Renderer::resetBlink()
{
	m_blinkColorDirection = -1;
	m_blinkColorValue = 255;
}
  
void Renderer::PrintString(__int32 x, __int32 y, char* string, D3DCOLOR color, __int32 flags)
{
	__int32 realX = x;
	__int32 realY = y;
	float factorX = ScreenWidth / 800.0f;
	float factorY = ScreenHeight / 600.0f;

	RECT rect = { 0, 0, 0, 0 };

	m_gameFont->DrawTextA(NULL, string, strlen(string), &rect, DT_CALCRECT, color);
	
	if (flags & PRINTSTRING_CENTER)
	{
		__int32 width = rect.right - rect.left;
		rect.left = x * factorX - width / 2;
		rect.right = x * factorX + width / 2;
		rect.top += y * factorY;
		rect.bottom += y * factorY;
	}
	else
	{
		rect.left = x * factorX;
		rect.right += x * factorX;
		rect.top = y * factorY;
		rect.bottom += y * factorY;
	}

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

bool Renderer::printDebugMessage(__int32 x, __int32 y, __int32 alpha, byte r, byte g, byte b, LPCSTR Message)
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
 
RendererMesh* Renderer::getRendererMeshFromTrMesh(RendererObject* obj, __int16* meshPtr, __int16* refMeshPtr, 
												  __int16 boneIndex, __int32 isJoints, __int32 isHairs)
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
		 
		// ColAddHorizon special handling
		if (obj->GetId() == ID_HORIZON && g_Script->GetLevel(CurrentLevel)->ColAddHorizon)
		{
			if (texture->attribute == 2 || (effects & 1))
				bucketIndex = RENDERER_BUCKET_TRANSPARENT;
			else
				bucketIndex = RENDERER_BUCKET_SOLID;
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
				vertex.nx = normals[indices[v]].vx / 16300.0f;
				vertex.ny = normals[indices[v]].vy / 16300.0f;
				vertex.nz = normals[indices[v]].vz / 16300.0f;
			}

			vertex.u = (texture->vertices[v].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
			vertex.v = (texture->vertices[v].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;

			vertex.boneAndFlags = boneIndex;
			if (isJoints && boneIndex != 0 && m_laraSkinJointRemap[boneIndex][indices[v]] != -1)
				vertex.boneAndFlags = m_laraSkinJointRemap[boneIndex][indices[v]];
			if (isHairs)
				vertex.boneAndFlags = indices[v];

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
				vertex.nx = normals[indices[v]].vx / 16300.0f;
				vertex.ny = normals[indices[v]].vy / 16300.0f;
				vertex.nz = normals[indices[v]].vz / 16300.0f;
			}

			D3DXMATRIX world;
			D3DXMatrixTranslation(&world, 80000, 0, 50000);
			D3DXVECTOR4 n;
			D3DXVec3Transform(&n, &D3DXVECTOR3(vertex.nx, vertex.ny, vertex.nz), &world);
			D3DXVECTOR3 n2 = D3DXVECTOR3(n.x, n.y, n.z);
			D3DXVec3TransformNormal(&n2, &D3DXVECTOR3(vertex.nx, vertex.ny, vertex.nz), &world);
			 
			D3DXVec4Normalize(&n, &n);
			D3DXVec3Normalize(&n2, &n2);

			n /= n.w;

			vertex.u = (texture->vertices[v].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
			vertex.v = (texture->vertices[v].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;
		
			vertex.boneAndFlags = boneIndex;
			if (isJoints && boneIndex != 0 && m_laraSkinJointRemap[boneIndex][indices[v]] != -1)
				vertex.boneAndFlags = m_laraSkinJointRemap[boneIndex][indices[v]];
			if (isHairs)
				vertex.boneAndFlags = indices[v];

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
	// Step -1 XD: prepare animated textures
	__int16 numSets = *AnimatedTextureRanges;
	__int16* animatedPtr = AnimatedTextureRanges;
	animatedPtr++;

	for (__int32 i = 0; i < numSets; i++)
	{
		RendererAnimatedTextureSet* set = new RendererAnimatedTextureSet();
		__int16 numTextures = *animatedPtr + 1;
		animatedPtr++;

		for (__int32 j = 0; j < numTextures; j++)
		{
			__int16 textureId = *animatedPtr;
			animatedPtr++;

			OBJECT_TEXTURE* texture = &ObjectTextures[textureId];
			__int32 tile = texture->tileAndFlag & 0x7FFF;

			RendererAnimatedTexture* newTexture = new RendererAnimatedTexture();
			newTexture->Id = textureId;

			for (__int32 k = 0; k < 4; k++)
			{
				float x = (texture->vertices[k].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
				float y = (texture->vertices[k].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;
				
				newTexture->UV[k] = D3DXVECTOR2(x, y);
			}

			set->Textures.push_back(newTexture);
		}

		m_animatedTextureSets.push_back(set);
	}

	// Step 1: create the texture atlas
	byte* buffer = (byte*)malloc(TEXTURE_ATLAS_SIZE * TEXTURE_ATLAS_SIZE * 4);
	__int32 blockX = 0;
	__int32 blockY = 0;

	ZeroMemory(buffer, TEXTURE_ATLAS_SIZE * TEXTURE_ATLAS_SIZE * 4);

	__int32 typ = LaraDrawType;
	if (gfLevelFlags & 1)
	{
		memcpy(m_laraSkinJointRemap, m_youngLaraSkinJointRemap, 15 * 32 * 2);
	}
	else
	{ 
		memcpy(m_laraSkinJointRemap, m_normalLaraSkinJointRemap, 15 * 32 * 2);
	}

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

	if (m_skyTexture != NULL)
		m_skyTexture->Release();

	// Create the new texture atlas
	D3DXCreateTexture(m_device, 256, 768, 0, D3DX_DEFAULT, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_fontAndMiscTexture);

	// Lock the texture pixels and copy them from the buffer
	m_fontAndMiscTexture->LockRect(0, &rect, NULL, NULL);
	dest = static_cast<unsigned char*>(rect.pBits);
	memcpy(dest, MiscTextures, 256 * 768 * 4);
	m_fontAndMiscTexture->UnlockRect(0);

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
					__int16 textureIndex = poly->Texture & 0x3FFF;
					bool doubleSided = (poly->Texture & 0x8000) >> 15;

					// Get the object texture
					OBJECT_TEXTURE* texture = &ObjectTextures[textureIndex];
					__int32 tile = texture->tileAndFlag & 0x7FFF;

					// Create vertices
					RendererBucket* bucket;

					__int32 animatedSetIndex = getAnimatedTextureInfo(textureIndex);
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

					if (animatedSetIndex == -1)
					{
						bucket = mesh->GetBucket(bucketIndex);
						roomObject->HasDataInBucket[bucketIndex] = true;
					}
					else
					{
						bucket = mesh->GetAnimatedBucket(bucketIndex);
						roomObject->HasDataInAnimatedBucket[bucketIndex] = true;
					}

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
					newPolygon.AnimatedSet = animatedSetIndex;
					newPolygon.TextureId = textureIndex;
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
					__int16 textureIndex = poly->Texture & 0x3FFF;
					bool doubleSided = (poly->Texture & 0x8000) >> 15;

					// Get the object texture
					OBJECT_TEXTURE* texture = &ObjectTextures[textureIndex];
					__int32 tile = texture->tileAndFlag & 0x7FFF;

					// Create vertices
					RendererBucket* bucket;

					__int32 animatedSetIndex = getAnimatedTextureInfo(textureIndex);
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

					if (animatedSetIndex == -1)
					{
						bucket = mesh->GetBucket(bucketIndex);
						roomObject->HasDataInBucket[bucketIndex] = true;
					}
					else
					{
						bucket = mesh->GetAnimatedBucket(bucketIndex);
						roomObject->HasDataInAnimatedBucket[bucketIndex] = true;
					}

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
						roomObject->NumVertices++;
					}

					bucket->Indices.push_back(baseVertices);
					bucket->Indices.push_back(baseVertices + 1);
					bucket->Indices.push_back(baseVertices + 2);
					bucket->NumIndices += 3;

					RendererPolygon newPolygon;
					newPolygon.Shape = SHAPE_TRIANGLE;
					newPolygon.AnimatedSet = animatedSetIndex;
					newPolygon.TextureId = textureIndex;
					newPolygon.Indices[0] = baseVertices;
					newPolygon.Indices[1] = baseVertices + 1;
					newPolygon.Indices[2] = baseVertices + 2;
					bucket->Polygons.push_back(newPolygon);

					polygons += sizeof(tr4_mesh_face3);
				}
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
				objNum == ID_ROPE || objNum == ID_KILL_ALL_TRIGGERS || objNum == ID_EARTHQUAKE || 
				objNum == ID_CAMERA_TARGET || objNum == ID_WATERFALLMIST)
			{
				moveable->DoNotDraw = true;
			}
			else
			{
				moveable->DoNotDraw = false;
			}

			for (__int32 j = 0; j < obj->nmeshes; j++)
			{
				// HACK: mesh pointer 0 is the placeholder for Lara's body parts and is right hand with pistols
				// We need to override the box index because the engine will take mesh 0 while drawing pistols anim,
				// and vertices have bone index 0 and not 10
				__int32 meshPtrIndex = RawMeshPointers[obj->meshIndex / 2 + j] / 2;
				__int32 boneIndex = (meshPtrIndex == 0 ? HAND_R : j);

				__int16* meshPtr = &RawMeshData[meshPtrIndex];
				RendererMesh* mesh = getRendererMeshFromTrMesh(moveable,
															   meshPtr, 
															   Meshes[obj->meshIndex + 2 * j],
															   boneIndex, MoveablesIds[i] == ID_LARA_SKIN_JOINTS,
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
			buildHierarchy(moveable);

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
							if (jointVertex->boneAndFlags != j)
							{
								RendererMesh* skinMesh = objSkin->ObjectMeshes[jointVertex->boneAndFlags];
								RendererBone* skinBone = objSkin->LinearizedBones[jointVertex->boneAndFlags];

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

				m_hairVertices = (RendererVertex*)malloc(m_numHairVertices * 2 * sizeof(RendererVertex));
				m_hairIndices = (__int32*)malloc(m_numHairIndices * 2 * 4);
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
		RendererMesh* mesh = getRendererMeshFromTrMesh(staticObject, meshPtr, Meshes[obj->meshNumber], 0, false, false);

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

		float left = (oldSprite->left * 256.0f + GET_ATLAS_PAGE_X(oldSprite->tile - 1));
		float top = (oldSprite->top * 256.0f + GET_ATLAS_PAGE_Y(oldSprite->tile - 1));
		float right = (oldSprite->right * 256.0f + GET_ATLAS_PAGE_X(oldSprite->tile - 1));
		float bottom = (oldSprite->bottom * 256.0f + GET_ATLAS_PAGE_Y(oldSprite->tile - 1));

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

void Renderer::buildHierarchyRecursive(RendererObject* obj, RendererBone* node, RendererBone* parentNode)
{
	D3DXMatrixMultiply(&node->GlobalTransform, &node->Transform, &parentNode->GlobalTransform);
	obj->BindPoseTransforms[node->Index] = node->GlobalTransform;
	obj->Skeleton->GlobalTranslation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	node->GlobalTranslation = node->Translation + parentNode->GlobalTranslation;

	for (int j = 0; j < node->Children.size(); j++)
	{
		buildHierarchyRecursive(obj, node->Children[j], node);
	}
}

void Renderer::buildHierarchy(RendererObject* obj)
{
	obj->Skeleton->GlobalTransform = obj->Skeleton->Transform;
	obj->BindPoseTransforms[obj->Skeleton->Index] = obj->Skeleton->GlobalTransform;
	obj->Skeleton->GlobalTranslation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	for (int j = 0; j < obj->Skeleton->Children.size(); j++)
	{
		buildHierarchyRecursive(obj, obj->Skeleton->Children[j], obj->Skeleton);
	}
}

void Renderer::fromTrAngle(D3DXMATRIX* matrix, __int16* frameptr, __int32 index)
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

void Renderer::getVisibleRooms(int from, int to, D3DXVECTOR4* viewPort, bool water, int count) 
{
	stack<RendererRoomNode*> stack;
	RendererRoomNode* node = new RendererRoomNode();
	node->To = to;
	node->From = -1;
	stack.push(node);
	
	while (!stack.empty())
	{
		node = stack.top();
		stack.pop();

		if (m_rooms[node->To]->Visited)
			continue;
		m_rooms[node->To]->Visited = true;
		m_roomsToDraw.push_back(node->To);

		ROOM_INFO* room = &Rooms[node->To];

		D3DXVECTOR4 clipPort;
		__int16 numDoors = *(room->door);
		if (numDoors)
		{
			__int16* door = room->door + 1;
			for (int i = 0; i < numDoors; i++) {
				__int16 adjoiningRoom = *(door);

				if (node->From != adjoiningRoom && checkPortal(node->To, door, viewPort, &node->ClipPort))
				{
					RendererRoomNode* childNode = new RendererRoomNode();
					childNode->From = node->To;
					childNode->To = adjoiningRoom;
					stack.push(childNode);
				}

				door += 16;
			}
		}

		delete node;
	}
}

bool Renderer::checkPortal(__int16 roomIndex, __int16* portal, D3DXVECTOR4* viewPort, D3DXVECTOR4* clipPort) 
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

void Renderer::collectRooms()
{
	__int16 baseRoomIndex = Camera.pos.roomNumber;

	for (__int32 i = 0; i < NumberRooms; i++)
		if (m_rooms[i] != NULL)
			m_rooms[i]->Visited = false;

	m_roomsToDraw.clear();
	getVisibleRooms(-1, baseRoomIndex, &D3DXVECTOR4(-1.0f, -1.0f, 1.0f, 1.0f), false, 0);
}

void Renderer::collectItems()
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

void Renderer::prepareShadowMap()
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

	LPD3DXEFFECT effect = m_shaderDepth->GetEffect();
	effect->Begin(&cPasses, 0);
	effect->SetTexture(effect->GetParameterByName(NULL, "TextureAtlas"), m_textureAtlas);

	D3DXMatrixPerspectiveFovRH(&m_lightProjection, 90.0f * RADIAN, 1.0f, 1.0f, m_shadowLight->Out * 1.5f);

	D3DXMatrixLookAtRH(&m_lightView,
		&lightPos,
		&D3DXVECTOR3(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos),
		&D3DXVECTOR3(0.0f, -1.0f, 0.0f));

	effect->SetMatrix(effect->GetParameterByName(NULL, "View"), &m_lightView);
	effect->SetMatrix(effect->GetParameterByName(NULL, "Projection"), &m_lightProjection);

	// Bind the shadow map
	m_shadowMap->Bind();

	// Begin the scene 
	m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFFFFFFFF, 1.0f, 0);
	m_device->BeginScene();

	//Draw Lara
	for (__int32 k = 0; k < 4; k++)
	{
		drawLara((RENDERER_BUCKETS)k, RENDERER_PASSES::RENDERER_PASS_SHADOW_MAP);
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
			drawItem(m_itemsToDraw[j], (RENDERER_BUCKETS)k, RENDERER_PASSES::RENDERER_PASS_SHADOW_MAP);
		}
	}

	m_device->EndScene();
	m_shadowMap->Unbind();

	effect->End();
}

bool Renderer::drawPrimitives(D3DPRIMITIVETYPE primitiveType, UINT baseVertexIndex, UINT minVertexIndex, UINT numVertices, UINT baseIndex, UINT primitiveCount)
{
	m_numVertices += numVertices;
	m_numTriangles += primitiveCount;
	m_numDrawCalls++;

	HRESULT res = m_device->DrawIndexedPrimitive(primitiveType, baseVertexIndex, minVertexIndex, numVertices, baseIndex, primitiveCount);
	return (res == S_OK);
}

void Renderer::updateLaraAnimations()
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
	updateAnimation(laraObj, framePtr, frac, rate, mask);

	// Then the arms, based on current weapon status
	if ((Lara.gunStatus == LG_NO_ARMS || Lara.gunStatus == LG_HANDS_BUSY) && Lara.gunType != WEAPON_FLARE)
	{
		// Both arms
		mask = (1 << UARM_L) | (1 << LARM_L) | (1 << HAND_L) | (1 << UARM_R) |
			   (1 << LARM_R) | (1 << HAND_R);
		frac = GetFrame_D2(LaraItem, framePtr, &rate);
		updateAnimation(laraObj, framePtr, frac, rate, mask);
	}
	else
	{
		// While handling weapon some extra rotation could be applied to arms
		laraObj->LinearizedBones[UARM_L]->ExtraRotation += D3DXVECTOR3(TR_ANGLE_TO_RAD(Lara.leftArm.xRot),
			TR_ANGLE_TO_RAD(Lara.leftArm.yRot), TR_ANGLE_TO_RAD(Lara.leftArm.zRot));
		laraObj->LinearizedBones[UARM_R]->ExtraRotation += D3DXVECTOR3(TR_ANGLE_TO_RAD(Lara.rightArm.xRot),
			TR_ANGLE_TO_RAD(Lara.rightArm.yRot), TR_ANGLE_TO_RAD(Lara.rightArm.zRot));

		if (Lara.gunType != WEAPON_FLARE)
		{
			// HACK: shotgun must be handled differently (and probably also crossbow)
			if (Lara.gunType == WEAPON_SHOTGUN)
			{
				// Left arm
				mask = (1 << UARM_L) | (1 << LARM_L) | (1 << HAND_L);
				__int16* shotgunFramePtr = Lara.leftArm.frameBase + (Lara.leftArm.frameNumber) * (Anims[Lara.leftArm.animNumber].interpolation >> 8);
				updateAnimation(laraObj, &shotgunFramePtr, 0, 1, mask);
				
				// Right arm
				mask = (1 << UARM_R) | (1 << LARM_R) | (1 << HAND_R);
				shotgunFramePtr = Lara.rightArm.frameBase + (Lara.rightArm.frameNumber) * (Anims[Lara.rightArm.animNumber].interpolation >> 8);
				updateAnimation(laraObj, &shotgunFramePtr, 0, 1, mask);
			}
			else
			{
				// Left arm
				mask = (1 << UARM_L) | (1 << LARM_L) | (1 << HAND_L);
				frac = getFrame(Lara.leftArm.animNumber, Lara.leftArm.frameNumber, framePtr, &rate);
				updateAnimation(laraObj, framePtr, frac, rate, mask);
				
				// Right arm
				mask = (1 << UARM_R) | (1 << LARM_R) | (1 << HAND_R);
				frac = getFrame(Lara.rightArm.animNumber, Lara.rightArm.frameNumber, framePtr, &rate);
				updateAnimation(laraObj, framePtr, frac, rate, mask);
			}			
		}
		else
		{
			// Left arm
			mask = (1 << UARM_L) | (1 << LARM_L) | (1 << HAND_L);
			frac = getFrame(Lara.leftArm.animNumber, Lara.leftArm.frameNumber, framePtr, &rate);
			updateAnimation(laraObj, framePtr, frac, rate, mask);

			// Right arm
			mask = (1 << UARM_R) | (1 << LARM_R) | (1 << HAND_R);
			frac = GetFrame_D2(LaraItem, framePtr, &rate);
			updateAnimation(laraObj, framePtr, frac, rate, mask);
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

		__int32 lastVertex = 0;
		__int32 lastIndex = 0;

		ZeroMemory(m_hairVertices, m_numHairVertices * 2 * sizeof(RendererObject));
		ZeroMemory(m_hairIndices, m_numHairIndices * 2 * 4);

		for (__int32 p = 0; p < ((gfLevelFlags & 1) ? 2 : 1); p++)
		{
			// We can't use hardware skinning here, however hairs have just a few vertices so 
			// it's not so bad doing skinning in software
			if (gfLevelFlags & 1)
			{
				if (p == 1)
				{
					D3DXVec3TransformCoord(&parentVertices[0][0], &parentMesh->Positions[68], &world);
					D3DXVec3TransformCoord(&parentVertices[0][1], &parentMesh->Positions[69], &world);
					D3DXVec3TransformCoord(&parentVertices[0][2], &parentMesh->Positions[70], &world);
					D3DXVec3TransformCoord(&parentVertices[0][3], &parentMesh->Positions[71], &world);
				}
				else
				{
					D3DXVec3TransformCoord(&parentVertices[0][0], &parentMesh->Positions[79], &world);
					D3DXVec3TransformCoord(&parentVertices[0][1], &parentMesh->Positions[78], &world);
					D3DXVec3TransformCoord(&parentVertices[0][2], &parentMesh->Positions[76], &world);
					D3DXVec3TransformCoord(&parentVertices[0][3], &parentMesh->Positions[77], &world);
				}
			}
			else
			{
				D3DXVec3TransformCoord(&parentVertices[0][0], &parentMesh->Positions[37], &world);
				D3DXVec3TransformCoord(&parentVertices[0][1], &parentMesh->Positions[39], &world);
				D3DXVec3TransformCoord(&parentVertices[0][2], &parentMesh->Positions[40], &world);
				D3DXVec3TransformCoord(&parentVertices[0][3], &parentMesh->Positions[38], &world);
			}

			for (__int32 i = 0; i < 6; i++)
			{
				RendererMesh* mesh = hairsObj->ObjectMeshes[i];
				RendererBucket* bucket = mesh->GetBucket(RENDERER_BUCKETS::RENDERER_BUCKET_SOLID);

				D3DXMatrixTranslation(&translation, Hairs[7 * p + i].pos.xPos, Hairs[7 * p + i].pos.yPos, Hairs[7 * p + i].pos.zPos);
				D3DXMatrixRotationYawPitchRoll(&rotation,
					TR_ANGLE_TO_RAD(Hairs[7 * p + i].pos.yRot),
					TR_ANGLE_TO_RAD(Hairs[7 * p + i].pos.xRot),
					TR_ANGLE_TO_RAD(Hairs[7 * p + i].pos.zRot));
				D3DXMatrixMultiply(&m_hairsMatrices[6 * p + i], &rotation, &translation);

				__int32 baseVertex = lastVertex;

				for (__int32 j = 0; j < bucket->Vertices.size(); j++)
				{
					__int32 oldVertexIndex = (__int32)bucket->Vertices[j].boneAndFlags;
					if (oldVertexIndex < 4)
					{
						m_hairVertices[lastVertex].x = parentVertices[i][oldVertexIndex].x;
						m_hairVertices[lastVertex].y = parentVertices[i][oldVertexIndex].y;
						m_hairVertices[lastVertex].z = parentVertices[i][oldVertexIndex].z;
						m_hairVertices[lastVertex].u = bucket->Vertices[j].u;
						m_hairVertices[lastVertex].v = bucket->Vertices[j].v;

						D3DXVECTOR3 n = D3DXVECTOR3(bucket->Vertices[j].nx, bucket->Vertices[j].ny, bucket->Vertices[j].nz);
						D3DXVec3Normalize(&n, &n);
						D3DXVec3TransformNormal(&n, &n, &m_hairsMatrices[6 * p + i]);
						D3DXVec3Normalize(&n, &n);

						m_hairVertices[lastVertex].nx = n.x;
						m_hairVertices[lastVertex].ny = n.y;
						m_hairVertices[lastVertex].nz = n.z;

						lastVertex++;
					}
					else
					{
						D3DXVECTOR3 in = D3DXVECTOR3(bucket->Vertices[j].x, bucket->Vertices[j].y, bucket->Vertices[j].z);
						D3DXVECTOR4 out;

						D3DXVec3Transform(&out, &in, &m_hairsMatrices[6 * p + i]);

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

						D3DXVECTOR3 n = D3DXVECTOR3(bucket->Vertices[j].nx, bucket->Vertices[j].ny, bucket->Vertices[j].nz);
						D3DXVec3Normalize(&n, &n);
						D3DXVec3TransformNormal(&n, &n, &m_hairsMatrices[6 * p + i]);
						D3DXVec3Normalize(&n, &n);

						m_hairVertices[lastVertex].nx = n.x;
						m_hairVertices[lastVertex].ny = n.y;
						m_hairVertices[lastVertex].nz = n.z;

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
}

void Renderer::updateItemsAnimations()
{
	D3DXMATRIX translation;
	D3DXMATRIX rotation;

	for (__int32 i = 0; i < m_itemsToDraw.size(); i++)
	{
		RendererItemToDraw* itemToDraw = m_itemsToDraw[i];
		ITEM_INFO* item = itemToDraw->Item;

		// Lara has her own routine
		if (item->objectNumber == ID_LARA)
			continue;

		OBJECT_INFO* obj = &Objects[item->objectNumber];
		RendererObject* moveableObj = m_moveableObjects[item->objectNumber];

		// Update animation matrices
		if (obj->animIndex != -1)
		{
			__int16	*framePtr[2];
			__int32 rate;
			__int32 frac = GetFrame_D2(item, framePtr, &rate);
			updateAnimation(moveableObj, framePtr, frac, rate, 0xFFFFFFFF);
		}

		// Update world matrix
		D3DXMatrixTranslation(&translation, item->pos.xPos, item->pos.yPos, item->pos.zPos);
		D3DXMatrixRotationYawPitchRoll(&rotation, TR_ANGLE_TO_RAD(item->pos.yRot),
			TR_ANGLE_TO_RAD(item->pos.xRot), TR_ANGLE_TO_RAD(item->pos.zRot));
		D3DXMatrixMultiply(&itemToDraw->World, &rotation, &translation);
	}
}

__int32 Renderer::DumpGameScene()
{
	return drawScene(true);
}

__int32 Renderer::Draw()
{
	return drawScene(false);
}

void Renderer::insertLine2D(__int32 x1, __int32 y1, __int32 x2, __int32 y2, byte r, byte g, byte b)
{
	m_lines2D[m_numLines2D].Vertices[0] = D3DXVECTOR2(x1, y1);
	m_lines2D[m_numLines2D].Vertices[1] = D3DXVECTOR2(x2, y2);
	m_lines2D[m_numLines2D].Color = D3DCOLOR_XRGB(r, g, b);
	m_numLines2D++;
}

void Renderer::drawAllLines2D()
{
	m_dxLine->SetWidth(1);
	m_dxLine->SetPattern(0xffffffff);
	m_dxLine->Begin();

	for (__int32 i = 0; i < m_numLines2D; i++)
		m_dxLine->Draw(m_lines2D[i].Vertices, 2, m_lines2D[i].Color);

	m_dxLine->End();
}

void Renderer::drawBar(__int32 x, __int32 y, __int32 w, __int32 h, __int32 percent, __int32 color1, __int32 color2)
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
		insertLine2D(realX, realY + i, realX + realW, realY + i, 0, 0, 0);

	for (__int32 i = 0; i < realH; i++)
		insertLine2D(realX, realY + i, realX + realPercent, realY + i, r1, g1, b1);

	insertLine2D(realX, realY, realX + realW, realY, 255, 255, 255);
	insertLine2D(realX, realY + realH, realX + realW, realY + realH, 255, 255, 255);
	insertLine2D(realX, realY, realX, realY + realH, 255, 255, 255);
	insertLine2D(realX + realW, realY, realX + realW, realY + realH + 1, 255, 255, 255);
}

void Renderer::drawGameInfo()
{
	__int32 flashState = FlashIt();

	UpdateHealtBar(flashState);
	UpdateAirBar(flashState);
	DrawDashBar();
}

void Renderer::DrawDashBar()
{
	if (DashTimer < 120)
		drawBar(460, 32, 150, 12, 100 * (unsigned __int16)DashTimer / 120, 0xA0A000, 0xA000);
}

void Renderer::DrawAirBar(__int32 percentual)
{
	drawBar(450, 10, 150, 12, percentual, 0x0000A0, 0x0050A0);
}

void Renderer::DrawHealthBar(__int32 percentual)
{
	__int32 color2 = 0xA00000;
	if (Lara.poisoned || Lara.gassed)
		color2 = 0xA0A000;
	drawBar(10, 10, 150, 12, percentual, 0xA00000, color2);
}

void Renderer::drawDebugInfo()
{
	setCullMode(RENDERER_CULLMODE::CULLMODE_CCW);
	setBlendState(RENDERER_BLENDSTATE::BLENDSTATE_OPAQUE);
	setDepthWrite(true);

	if (UseSpotCam)
	{
		sprintf_s(&m_message[0], 255, "Spotcam.camera = %d    Spotcam.flags = %d     Current: %d", SpotCam[CurrentSplineCamera].camera, SpotCam[CurrentSplineCamera].flags, CurrentSplineCamera);
		printDebugMessage(10, 70, 255, 255, 255, 255, m_message);
	}

	__int32 y = 100;

	sprintf_s(&m_message[0], 255, "TR5Main Alpha");
	printDebugMessage(10, 80+y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "LaraItem.currentAnimState = %d", LaraItem->currentAnimState);
	printDebugMessage(10, 90 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "LaraItem.animNumber = %d", LaraItem->animNumber);
	printDebugMessage(10, 100 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "LaraItem.frameNumber = %d", LaraItem->frameNumber);
	printDebugMessage(10, 110 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "LaraItem.roomNumber = %d", LaraItem->roomNumber);
	printDebugMessage(10, 120 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "LaraItem.pos = < %d %d %d >", LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
	printDebugMessage(10, 130 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "NumVertices = %d", m_numVertices);
	printDebugMessage(10, 140 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "NumTriangles = %d", m_numTriangles);
	printDebugMessage(10, 150 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Camera.pos = < %d %d %d >", Camera.pos.x, Camera.pos.y, Camera.pos.z);
	printDebugMessage(10, 160 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Camera.target = < %d %d %d >", Camera.target.x, Camera.target.y, Camera.target.z);
	printDebugMessage(10, 170 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "CurrentLevel: %d", CurrentLevel);
	printDebugMessage(10, 180 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.backGun: %d", Lara.backGun);
	printDebugMessage(10, 190 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.leftArm.animNumber: %d", Lara.leftArm.animNumber);
	printDebugMessage(10, 200 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.leftArm.frameNumber: %d", Lara.leftArm.frameNumber);
	printDebugMessage(10, 210 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.rightArm.animNumber: %d", Lara.rightArm.animNumber);
	printDebugMessage(10, 220 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.rightArm.frameNumber: %d", Lara.rightArm.frameNumber);
	printDebugMessage(10, 230 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.weaponItem: %d", Lara.weaponItem);
	printDebugMessage(10, 240 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.gunType: %d", Lara.gunType);
	printDebugMessage(10, 250 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.gunStatus: %d", Lara.gunStatus);
	printDebugMessage(10, 260 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.crowbar: %d", Lara.crowbar);
	printDebugMessage(10, 270 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.leftArm.rot: %d %d %d", Lara.leftArm.xRot, Lara.leftArm.yRot, Lara.leftArm.zRot);
	printDebugMessage(10, 280 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.rightArm.rot: %d %d %d", Lara.rightArm.xRot, Lara.rightArm.yRot, Lara.rightArm.zRot);
	printDebugMessage(10, 290 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.torsoRot: < %d %d %d >", Lara.torsoXrot, Lara.torsoYrot, Lara.torsoZrot);
	printDebugMessage(10, 300 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.headRot: < %d %d %d >", Lara.headXrot, Lara.headYrot, Lara.headZrot);
	printDebugMessage(10, 310 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.leftArm.lock: %d", Lara.leftArm.lock);
	printDebugMessage(10, 320 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Lara.rightArm.lock: %d", Lara.rightArm.lock);
	printDebugMessage(10, 330 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Autotarget: %d", OptionAutoTarget);
	printDebugMessage(10, 340 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "NumberDrawnRooms: %d", m_roomsToDraw.size());
	printDebugMessage(10, 350 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Camera.pos.roomNumber = %d", Camera.pos.roomNumber);
	printDebugMessage(10, 360 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "NumberDrawnItems = %d", m_itemsToDraw.size());
	printDebugMessage(10, 370 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "Update = %d, ShadowMap = %d, Clear = %d, Fill = %d, Light = %d, Final = %d, Z-Buffer = %d", 
			  m_timeUpdate, m_timePrepareShadowMap, m_timeClearGBuffer, m_timeFillGBuffer, m_timeLight, m_timeCombine, m_timeReconstructZBuffer);
	printDebugMessage(10, 380 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "DrawCalls = %d", m_numDrawCalls);
	printDebugMessage(10, 390 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "NumLights = %d", m_lights.size());
	printDebugMessage(10, 400 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "DrawSceneTime = %d", m_timeDrawScene);
	printDebugMessage(10, 410 + y, 255, 255, 255, 255, m_message);

	sprintf_s(&m_message[0], 255, "SkyPos = %d", SkyPos1);
	printDebugMessage(10, 420 + y, 255, 255, 255, 255, m_message);

	m_dxSprite->Begin(0);

	D3DXMATRIX scale;
	D3DXMatrixScaling(&scale, 150.0f / 800.0f, 150.0f / 800.0f, 150.0f / 800.0f);
	  
	m_dxSprite->SetTransform(&scale);
	m_dxSprite->Draw(m_colorBuffer->GetTexture(), NULL, &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(0, 0, 0), 0xFFFFFFFF);
	m_dxSprite->SetTransform(&scale);
	m_dxSprite->Draw(m_normalBuffer->GetTexture(), NULL, &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(1200, 0, 0), 0xFFFFFFFF);
	m_dxSprite->SetTransform(&scale);
	m_dxSprite->Draw(m_vertexLightBuffer->GetTexture(), NULL, &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(2400, 0, 0), 0xFFFFFFFF);
	m_dxSprite->SetTransform(&scale);
	m_dxSprite->Draw(m_lightBuffer->GetTexture(), NULL, &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(3600, 0, 0), 0xFFFFFFFF);

	m_dxSprite->End();

}

__int32	Renderer::DrawPauseMenu(__int32 selectedIndex, bool reset)
{
	if (reset)
		resetBlink();

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = ScreenWidth;
	rect.bottom = ScreenHeight;

	setBlendState(RENDERER_BLENDSTATE::BLENDSTATE_OPAQUE);
	setDepthWrite(true);
	setCullMode(RENDERER_CULLMODE::CULLMODE_CCW);

	m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	m_device->BeginScene();
	
	m_dxSprite->Begin(0);
	m_dxSprite->Draw(m_renderTarget->GetTexture(), &rect, &D3DXVECTOR3(0.0f, 0.0f, 0.0f), &D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DCOLOR_ARGB(255, 128, 128, 128));
	m_dxSprite->End();

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

	m_dxSprite->Begin(0);
	m_dxSprite->Draw(m_renderTarget->GetTexture(), &rect, &D3DXVECTOR3(0.0f, 0.0f, 0.0f), &D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DCOLOR_ARGB(255, 64, 64, 64));
	m_dxSprite->End();

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
	return drawInventoryScene();
}

__int32	Renderer::drawObjectOn2DPosition(__int16 x, __int16 y, __int16 objectNum, __int16 rotX, __int16 rotY, __int16 rotZ)
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
		updateAnimation(moveableObj, &Anims[obj->animIndex].framePtr, 0, 0, 0xFFFFFFFF);
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
	drawObjectOn2DPosition(600 + PickupX, 450, objectNum, 0, m_pickupRotation, 0);
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

	m_dxSprite->Begin(0);
	m_dxSprite->Draw(m_renderTarget->GetTexture(), &rect, &D3DXVECTOR3(0.0f, 0.0f, 0.0f), &D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DCOLOR_ARGB(255, 64, 64, 64));
	m_dxSprite->End();

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

__int32 Renderer::drawInventoryScene()
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
	setDepthWrite(true);
	setCullMode(RENDERER_CULLMODE::CULLMODE_CCW);
	setBlendState(RENDERER_BLENDSTATE::BLENDSTATE_OPAQUE);

	byte colorComponent = 255; // (m_fadeTimer < 15.0f ? 255.0f / (15.0f - min(m_fadeTimer, 15.0f)) : 255);

	// Draw the full screen background
	if (g_Inventory->GetType() == INV_TYPE_TITLE)
	{
		// Scale matrix for drawing full screen background
		D3DXMatrixScaling(&m_tempScale, ScreenWidth / 1024.0f, ScreenHeight / 768.0f, 0.0f);

		m_dxSprite->Begin(0);
		m_dxSprite->SetTransform(&m_tempScale);
		m_dxSprite->Draw(m_titleScreen, &rect, &D3DXVECTOR3(0.0f, 0.0f, 0.0f), &D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DCOLOR_ARGB(255, colorComponent, colorComponent, colorComponent));
		m_dxSprite->End();
	}
	else
	{
		D3DXMatrixScaling(&m_tempScale, 1.0f, 1.0f, 1.0f);

		m_dxSprite->Begin(0);
		m_dxSprite->SetTransform(&m_tempScale);
		m_dxSprite->Draw(m_renderTarget->GetTexture(), &rect, &D3DXVECTOR3(0.0f, 0.0f, 0.0f), &D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DCOLOR_ARGB(255, 64, 64, 64));
		m_dxSprite->End();
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
	effect->SetVector(effect->GetParameterByName(NULL, "LightColor"), &D3DXVECTOR4(1.0f, 1.0f, 0.5f, 1.0f));
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
				getFrame(obj->animIndex, ring->frameIndex, framePtr, &rate);
				updateAnimation(moveableObj, framePtr, 0, 1, 0xFFFFFFFF);
			}
			else
			{
				if (obj->animIndex != -1)
					updateAnimation(moveableObj, &Anims[obj->animIndex].framePtr, 0, 1, 0xFFFFFFFF);
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

					setGpuStateForBucket((RENDERER_BUCKETS)m);

					m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
					m_device->SetIndices(bucket->GetIndexBuffer());

					for (int iPass = 0; iPass < cPasses; iPass++)
					{
						effect->BeginPass(iPass);
						effect->CommitChanges();

						drawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

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
						else if (ring->passportAction == INV_WHAT_PASSPORT_SELECT_LEVEL)
						{
							__int16 lastY = 44;

							for (__int32 n = 1; n < g_Script->GetNumLevels(); n++)
							{
								GameScriptLevel* levelScript = g_Script->GetLevel(n);
								PrintString(400, lastY, g_Script->GetString(levelScript->Name), D3DCOLOR_ARGB(255, 255, 255, 255),
									PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == n - 1 ? PRINTSTRING_BLINK : 0));
								
								lastY += 24;
							}
						}
						char* string = (char*)"";
						switch (ring->passportAction)
						{
						case INV_WHAT_PASSPORT_NEW_GAME:
							string = g_Script->GetString(STRING_INV_NEW_GAME);
							break;
						case INV_WHAT_PASSPORT_SELECT_LEVEL:
							string = g_Script->GetString(STRING_INV_SELECT_LEVEL);
							break;
						case INV_WHAT_PASSPORT_LOAD_GAME:
							string = g_Script->GetString(STRING_INV_LOAD_GAME);
							break;
						case INV_WHAT_PASSPORT_SAVE_GAME:
							string = g_Script->GetString(STRING_INV_SAVE_GAME);
							break;
						case INV_WHAT_PASSPORT_EXIT_GAME:
							string = g_Script->GetString(STRING_INV_EXIT_GAME);
							break;
						case INV_WHAT_PASSPORT_EXIT_TO_TITLE:
							string = g_Script->GetString(STRING_INV_EXIT_TO_TITLE);
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

__int32 Renderer::getFrame(__int16 animation, __int16 frame, __int16** framePtr, __int32* rate)
{
	ITEM_INFO item;
	item.animNumber = animation;
	item.frameNumber = frame;

	return GetFrame_D2(&item, framePtr, rate);
}

void Renderer::collectSceneItems()
{
	collectRooms();
	collectItems();
	collectLights();
}

bool Renderer::drawScene(bool dump)
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

	GameScriptLevel* level = g_Script->GetLevel(CurrentLevel);

	// Collect scene data
	collectSceneItems();
	updateLaraAnimations();
	updateItemsAnimations();

	// Update animated textures every 2 frames
	if (GnFrameCounter % 2 == 0)
		updateAnimatedTextures();

	auto time2 = chrono::high_resolution_clock::now();
	m_timeUpdate = (chrono::duration_cast<ns>(time2 - time1)).count();
	time1 = time2;

	// Prepare the shadow map for the main light
	prepareShadowMap();

	time2 = chrono::high_resolution_clock::now();
	m_timePrepareShadowMap = (chrono::duration_cast<ns>(time2 - time1)).count();
	time1 = time2;

	// Set basic GPU state
	setCullMode(RENDERER_CULLMODE::CULLMODE_CCW);
	setBlendState(RENDERER_BLENDSTATE::BLENDSTATE_OPAQUE);
	setDepthWrite(true);
	m_device->SetVertexDeclaration(m_vertexDeclaration);

	// Clear the G-Buffer
	bindRenderTargets(m_colorBuffer, m_normalBuffer, m_depthBuffer, m_vertexLightBuffer);
	m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_COLORVALUE(0.0f, 0.0f, 0.0f, 1.0f), 1.0f, 0);

	effect = m_shaderClearGBuffer->GetEffect();
	m_device->BeginScene();
	effect->Begin(&cPasses, 0);
	effect->BeginPass(0);
	effect->CommitChanges();
	m_device->DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 6, 2, m_fullscreenQuadIndices, D3DFORMAT::D3DFMT_INDEX32,
		m_fullscreenQuadVertices, sizeof(RendererVertex));
	effect->EndPass();
	effect->End(); 
	m_device->EndScene(); 
 
	time2 = chrono::high_resolution_clock::now();
	m_timeClearGBuffer = (chrono::duration_cast<ns>(time2 - time1)).count();
	time1 = time2;

	// Fill the G-Buffer
	effect = m_shaderFillGBuffer->GetEffect();
	m_device->BeginScene();
	effect->Begin(&cPasses, 0);

	effect->SetMatrix(effect->GetParameterByName(NULL, "View"), &ViewMatrix);
	effect->SetMatrix(effect->GetParameterByName(NULL, "Projection"), &ProjectionMatrix);
	effect->SetTexture(effect->GetParameterByName(NULL, "TextureAtlas"), m_textureAtlas);
	effect->SetBool(effect->GetParameterByName(NULL, "UseSkinning"), false);
	effect->SetVector(effect->GetParameterByName(NULL, "Color"), &D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));
	effect->SetTexture(effect->GetParameterByName(NULL, "CausticsMap"), m_caustics[m_currentCausticsFrame / 2]);

	m_currentCausticsFrame++;
	m_currentCausticsFrame %= 32;

	D3DXMATRIX world;

	// Draw opaque geometry
	if (level->Horizon && m_rooms[Camera.pos.roomNumber]->Room->flags & 8)
		drawHorizonAndSky();

	for (__int32 i = 0; i < m_roomsToDraw.size(); i++)
	{
		RendererRoom* room = m_rooms[m_roomsToDraw[i]];
		if (room == NULL)
			continue;

		drawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_GBUFFER, false);
		drawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_GBUFFER, true);

		drawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER, false);
		drawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER, true);

		// Draw static objects
		ROOM_INFO* r = room->Room;
		if (r->numMeshes != 0)
		{
			for (__int32 j = 0; j < r->numMeshes; j++)
			{
				MESH_INFO* sobj = &r->mesh[j];
				RendererObject* staticObj = m_staticObjects[sobj->staticNumber];
				RendererMesh* staticMesh = staticObj->ObjectMeshes[0];

				drawStatic(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
				drawStatic(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
			}
		}
	}

	drawLara(RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
	drawLara(RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
	
	for (__int32 i = 0; i < m_itemsToDraw.size(); i++)
	{
		RendererObject* obj = m_moveableObjects[Items[m_itemsToDraw[i]->Id].objectNumber];
		drawItem(m_itemsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
		drawItem(m_itemsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
	}

	drawGunshells(RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
	drawGunshells(RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER);

	// Draw alpha tested geometry
	for (__int32 i = 0; i < m_roomsToDraw.size(); i++)
	{
		RendererRoom* room = m_rooms[m_roomsToDraw[i]];
		if (room == NULL)
			continue;

		drawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_GBUFFER, false);
		drawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_GBUFFER, true);
		
		drawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER, false);
		drawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER, true);

		// Draw static objects
		ROOM_INFO* r = room->Room;
		if (r->numMeshes != 0)
		{
			for (__int32 j = 0; j < r->numMeshes; j++)
			{
				MESH_INFO* sobj = &r->mesh[j];
				RendererObject* staticObj = m_staticObjects[sobj->staticNumber];
				RendererMesh* staticMesh = staticObj->ObjectMeshes[0];

				drawStatic(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
				drawStatic(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
			}
		}
	}

	drawLara(RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
	drawLara(RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
	 
	for (__int32 i = 0; i < m_itemsToDraw.size(); i++)
	{
		drawItem(m_itemsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
		drawItem(m_itemsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
	}

	drawGunshells(RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_GBUFFER);
	drawGunshells(RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_GBUFFER);

	effect->EndPass();
	effect->End();
	m_device->EndScene();

	time2 = chrono::high_resolution_clock::now();
	m_timeFillGBuffer = (chrono::duration_cast<ns>(time2 - time1)).count();
	time1 = time2;

	// Reset the back-buffer
	restoreBackBuffer();
	   
	// Bind the light target
	restoreBackBuffer();
	bindRenderTargets(m_shadowBuffer, NULL, NULL, NULL);
	m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_COLORVALUE(1.0f, 0.0f, 0.0f, 1.0f), 1.0f, 0);
	m_device->BeginScene();
	m_device->EndScene();

	restoreBackBuffer();
	bindRenderTargets(m_lightBuffer, NULL, NULL, NULL);
	
	effect = m_shaderLight->GetEffect();

	// Setup additive blending
	setCullMode(RENDERER_CULLMODE::CULLMODE_NONE);
	setBlendState(RENDERER_BLENDSTATE::BLENDSTATE_ALPHABLEND);
	 
	// Clear the screen and disable Z write
	m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	setDepthWrite(false);

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

			effect->SetBool(effect->GetParameterByName(NULL, "CastShadows"), false);
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

			drawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, m_sphereMesh->NumVertices, 0, m_sphereMesh->NumIndices / 3);

			effect->EndPass();
		}
	}

	effect->End();
	m_device->EndScene();

	time2 = chrono::high_resolution_clock::now();
	m_timeLight = (chrono::duration_cast<ns>(time2 - time1)).count();
	time1 = time2;

	restoreBackBuffer();
	         
	if (dump)
		bindRenderTargets(m_renderTarget, NULL, NULL, NULL);

	// Combine stage
	setCullMode(RENDERER_CULLMODE::CULLMODE_CCW);
	setBlendState(RENDERER_BLENDSTATE::BLENDSTATE_OPAQUE);
	setDepthWrite(true);

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
	m_device->DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 6, 2, m_fullscreenQuadIndices, D3DFORMAT::D3DFMT_INDEX32,
		m_fullscreenQuadVertices, sizeof(RendererVertex));
	effect->EndPass();
	effect->End();
	m_device->EndScene();
	 
	time2 = chrono::high_resolution_clock::now();
	m_timeCombine = (chrono::duration_cast<ns>(time2 - time1)).count();
	time1 = time2;

	// Clear depth and start reconstructing Z-Buffer
	m_device->Clear(0, NULL, D3DCLEAR_ZBUFFER, 0xFFFFFFFF, 1.0f, 0);

	// We need to use alpha blending because we need to write only to the Z-Buffer
	//setBlendState(RENDERER_BLENDSTATE::BLENDSTATE_SPECIAL_Z_BUFFER);
	// TODO: if I use setBlendSTate probably it's overridden later and I see a black screen
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);

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

		drawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH, false);
		drawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH, true);
		
		drawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH, false);
		drawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH, true);

		// Draw static objects
		ROOM_INFO* r = room->Room;
		if (r->numMeshes != 0)
		{
			for (__int32 j = 0; j < r->numMeshes; j++)
			{
				MESH_INFO* sobj = &r->mesh[j];
				RendererObject* staticObj = m_staticObjects[sobj->staticNumber];
				RendererMesh* staticMesh = staticObj->ObjectMeshes[0];

				drawStatic(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
				drawStatic(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
			}
		}
	}

	drawLara(RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
	drawLara(RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);

	for (__int32 i = 0; i < m_itemsToDraw.size(); i++)
	{
		drawItem(m_itemsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
		drawItem(m_itemsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
	}

	// Draw alpha tested geometry
	for (__int32 i = 0; i < m_roomsToDraw.size(); i++)
	{
		RendererRoom* room = m_rooms[m_roomsToDraw[i]];
		if (room == NULL)
			continue;

		drawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH, false);
		drawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH, false);

		// Draw static objects
		ROOM_INFO* r = room->Room;
		if (r->numMeshes != 0)
		{
			for (__int32 j = 0; j < r->numMeshes; j++)
			{
				MESH_INFO* sobj = &r->mesh[j];
				RendererObject* staticObj = m_staticObjects[sobj->staticNumber];
				RendererMesh* staticMesh = staticObj->ObjectMeshes[0];

				drawStatic(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
				drawStatic(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
			}
		}
	}

	drawLara(RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
	drawLara(RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);

	for (__int32 i = 0; i < m_itemsToDraw.size(); i++)
	{
		drawItem(m_itemsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
		drawItem(m_itemsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS, RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH);
	}

	m_device->EndScene();
	effect->End();

	// Transparent pass:
	effect = m_shaderTransparent->GetEffect();
	m_device->BeginScene();
	effect->Begin(&cPasses, 0);

	effect->SetMatrix(effect->GetParameterByName(NULL, "View"), &ViewMatrix);
	effect->SetMatrix(effect->GetParameterByName(NULL, "Projection"), &ProjectionMatrix);
	effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPES::MODEL_TYPE_ROOM);
	effect->SetTexture(effect->GetParameterByName(NULL, "TextureAtlas"), m_textureAtlas);
	effect->SetBool(effect->GetParameterByName(NULL, "UseSkinning"), false);
	effect->SetVector(effect->GetParameterByName(NULL, "Color"), &D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));

	for (__int32 i = 0; i < m_roomsToDraw.size(); i++)
	{
		RendererRoom* room = m_rooms[m_roomsToDraw[i]];
		if (room == NULL)
			continue;

		drawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT, RENDERER_PASSES::RENDERER_PASS_TRANSPARENT, false);
		drawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT, RENDERER_PASSES::RENDERER_PASS_TRANSPARENT, true);
		
		drawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT_DS, RENDERER_PASSES::RENDERER_PASS_TRANSPARENT, false);
		drawRoom(m_roomsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT_DS, RENDERER_PASSES::RENDERER_PASS_TRANSPARENT, true);

		// Draw static objects
		ROOM_INFO* r = room->Room;
		if (r->numMeshes != 0)
		{
			for (__int32 j = 0; j < r->numMeshes; j++)
			{
				MESH_INFO* sobj = &r->mesh[j];
				RendererObject* staticObj = m_staticObjects[sobj->staticNumber];
				RendererMesh* staticMesh = staticObj->ObjectMeshes[0];

				drawStatic(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT, RENDERER_PASSES::RENDERER_PASS_TRANSPARENT);
				drawStatic(m_roomsToDraw[i], j, RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT_DS, RENDERER_PASSES::RENDERER_PASS_TRANSPARENT);
			}
		}
	}

	drawLara(RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT, RENDERER_PASSES::RENDERER_PASS_TRANSPARENT);
	drawLara(RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT_DS, RENDERER_PASSES::RENDERER_PASS_TRANSPARENT);

	for (__int32 i = 0; i < m_itemsToDraw.size(); i++)
	{
		RendererObject* obj = m_moveableObjects[Items[m_itemsToDraw[i]->Id].objectNumber];
		drawItem(m_itemsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT, RENDERER_PASSES::RENDERER_PASS_TRANSPARENT);
		drawItem(m_itemsToDraw[i], RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT_DS, RENDERER_PASSES::RENDERER_PASS_TRANSPARENT);
	}

	drawGunshells(RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT, RENDERER_PASSES::RENDERER_PASS_TRANSPARENT);
	drawGunshells(RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT_DS, RENDERER_PASSES::RENDERER_PASS_TRANSPARENT);

	effect->End();
	m_device->EndScene();

	// Prepare sprites
	// TODO: preallocate big buffer and avoica memory allocations!
	for (vector<RendererSpriteToDraw*>::iterator it = m_spritesToDraw.begin(); it != m_spritesToDraw.end(); ++it)
		delete (*it);
	m_spritesToDraw.clear();
	for (vector<RendererLine3DToDraw*>::iterator it = m_lines3DToDraw.begin(); it != m_lines3DToDraw.end(); ++it)
		delete (*it);
	m_lines3DToDraw.clear();

	drawFires();
	drawSmokes();
	drawBlood();
	drawSparks();
	drawBubbles();
	drawDrips();
	drawRipples();
	drawUnderwaterDust();

	// Do weather
	if (level->Rain)
		doRain();
	else if (level->Snow)
		doSnow();

	// Draw sprites
	drawSprites();
	drawGunFlashes();
	drawLines3D();
	
	time2 = chrono::high_resolution_clock::now();
	m_timeReconstructZBuffer = (chrono::duration_cast<ns>(time2 - time1)).count();
	time1 = time2;

	auto timeScene2 = chrono::high_resolution_clock::now();
	m_timeDrawScene = (chrono::duration_cast<ns>(timeScene2 - timeScene1)).count() / 1000;

	if (!dump)
	{
		drawDebugInfo();
		m_device->Present(NULL, NULL, NULL, NULL);
	}
	else
		restoreBackBuffer();

	return true;
}

bool Renderer::bindRenderTargets(RenderTarget2D* rt1, RenderTarget2D* rt2, RenderTarget2D* rt3, RenderTarget2D* rt4)
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

bool Renderer::restoreBackBuffer()
{
	m_device->SetRenderTarget(0, m_backBufferTarget);
	m_device->SetRenderTarget(1, NULL);
	m_device->SetRenderTarget(2, NULL);
	m_device->SetRenderTarget(3, NULL);

	m_device->SetDepthStencilSurface(m_backBufferDepth);

	return true;
}

bool Renderer::drawLara(RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass)
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
		effect = m_shaderDepth->GetEffect();
	else if (pass == RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH)
		effect = m_shaderReconstructZBuffer->GetEffect();
	else if (pass == RENDERER_PASSES::RENDERER_PASS_GBUFFER)
		effect = m_shaderFillGBuffer->GetEffect();
	else
		effect = m_shaderTransparent->GetEffect();

	effect->SetBool(effect->GetParameterByName(NULL, "UseSkinning"), true);
	effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPES::MODEL_TYPE_LARA);
	effect->SetMatrixArray(effect->GetParameterByName(NULL, "Bones"), laraObj->AnimationTransforms, laraObj->ObjectMeshes.size());
	effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &m_LaraWorldMatrix);
	effect->SetVector(effect->GetParameterByName(NULL, "AmbientLight"), &m_rooms[LaraItem->roomNumber]->AmbientLight);

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
			setGpuStateForBucket(bucketIndex);

			// Bind buffers
			m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
			m_device->SetIndices(bucket->GetIndexBuffer());

			for (int iPass = 0; iPass < cPasses; iPass++)
			{
				effect->BeginPass(iPass);
				effect->CommitChanges();

				drawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

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
				setGpuStateForBucket(bucketIndex);

				// Bind buffers
				m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
				m_device->SetIndices(bucket->GetIndexBuffer());

				for (int iPass = 0; iPass < cPasses; iPass++)
				{
					effect->BeginPass(iPass);
					effect->CommitChanges();

					drawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

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
			setGpuStateForBucket(bucketIndex);

			m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
			m_device->SetIndices(bucket->GetIndexBuffer());

			D3DXMatrixMultiply(&world, &laraObj->AnimationTransforms[THIGH_L], &m_LaraWorldMatrix);

			for (int iPass = 0; iPass < cPasses; iPass++)
			{
				effect->BeginPass(iPass);
				effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &world);
				effect->CommitChanges();

				drawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

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

				drawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

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
				setGpuStateForBucket(bucketIndex);

				m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
				m_device->SetIndices(bucket->GetIndexBuffer());

				D3DXMatrixMultiply(&world, &laraObj->AnimationTransforms[HEAD], &m_LaraWorldMatrix);

				for (int iPass = 0; iPass < cPasses; iPass++)
				{
					effect->BeginPass(iPass);
					effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &world);
					effect->CommitChanges();

					drawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

					effect->EndPass();
				}
			}
		}
	}

	// Draw Lara's hairs
	if (bucketIndex == 0)
	{  
		setGpuStateForBucket(bucketIndex); 

		if (m_moveableObjects.find(ID_HAIR) != m_moveableObjects.end())
		{
			RendererObject* hairsObj = m_moveableObjects[ID_HAIR];
			D3DXMatrixIdentity(&world);

			for (int iPass = 0; iPass < cPasses; iPass++)
			{
				effect->BeginPass(iPass);
				effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &world);
				effect->CommitChanges();

				m_device->DrawIndexedPrimitiveUP(D3DPT_TRIANGLELIST, 0,
					m_numHairVertices*((gfLevelFlags & 1) ? 2 : 1),
					m_numHairIndices*((gfLevelFlags & 1) ? 2 : 1) / 3,
					m_hairIndices, D3DFMT_INDEX32, m_hairVertices, sizeof(RendererVertex));

				effect->EndPass();
			}
		}
	}

	return true;
}

bool Renderer::drawItem(RendererItemToDraw* itemToDraw, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass)
{ 
	D3DXMATRIX world;
	UINT cPasses = 1;

	ITEM_INFO* item = itemToDraw->Item;
	if (item->objectNumber == ID_LARA)
		return true;

	OBJECT_INFO* obj = &Objects[item->objectNumber];
	RendererObject* moveableObj = m_moveableObjects[item->objectNumber];
	if (moveableObj->DoNotDraw)
		return true;

	if (!moveableObj->HasDataInBucket[bucketIndex])
		return true;

	LPD3DXEFFECT effect;
	if (pass == RENDERER_PASSES::RENDERER_PASS_SHADOW_MAP)
		effect = m_shaderDepth->GetEffect();
	else if (pass == RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH)
		effect = m_shaderReconstructZBuffer->GetEffect();
	else if (pass == RENDERER_PASSES::RENDERER_PASS_GBUFFER)
		effect = m_shaderFillGBuffer->GetEffect();
	else
		effect = m_shaderTransparent->GetEffect();

	effect->SetBool(effect->GetParameterByName(NULL, "UseSkinning"), true);
	effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPES::MODEL_TYPE_MOVEABLE);
	effect->SetMatrixArray(effect->GetParameterByName(NULL, "Bones"), moveableObj->AnimationTransforms, moveableObj->ObjectMeshes.size());
	effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &itemToDraw->World);
	effect->SetVector(effect->GetParameterByName(NULL, "AmbientLight"), &m_rooms[item->roomNumber]->AmbientLight);
	
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

		setGpuStateForBucket(bucketIndex);

		m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
		m_device->SetIndices(bucket->GetIndexBuffer());

		for (int iPass = 0; iPass < cPasses; iPass++)
		{
			effect->BeginPass(iPass);
			effect->CommitChanges();

			drawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

			effect->EndPass();
		}
	}

	return true;
}
         
bool Renderer::drawRoom(__int32 roomIndex, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass, bool animated)
{
	D3DXMATRIX world;
	UINT cPasses = 1;

	RendererRoom* room = m_rooms[roomIndex];
	ROOM_INFO* r = room->Room;
	RendererObject* roomObj = room->RoomObject;
	
	if ((!animated && !roomObj->HasDataInBucket[bucketIndex]) || 
		(animated && !roomObj->HasDataInAnimatedBucket[bucketIndex]))
		return true;

	if (roomObj->ObjectMeshes.size() == 0)
		return true;

	RendererMesh* mesh = roomObj->ObjectMeshes[0];

	LPD3DXEFFECT effect;
	if (pass == RENDERER_PASSES::RENDERER_PASS_SHADOW_MAP)
		effect = m_shaderDepth->GetEffect();
	else if (pass == RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH)
		effect = m_shaderReconstructZBuffer->GetEffect();
	else if (pass == RENDERER_PASSES::RENDERER_PASS_GBUFFER)
		effect = m_shaderFillGBuffer->GetEffect();
	else
		effect = m_shaderTransparent->GetEffect();  
	       
	D3DXMatrixTranslation(&world, r->x, r->y, r->z);
	   
	effect->SetBool(effect->GetParameterByName(NULL, "UseSkinning"), false);
	effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &world);
	      
	if (bucketIndex == RENDERER_BUCKETS::RENDERER_BUCKET_SOLID || bucketIndex == RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS)
		effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLEND_MODES::BLENDMODE_OPAQUE);
	else
		effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLEND_MODES::BLENDMODE_ALPHATEST);

	if (pass == RENDERER_PASSES::RENDERER_PASS_GBUFFER)
	{
		if (isRoomUnderwater(roomIndex))
			effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPES::MODEL_TYPE_ROOM_UNDERWATER);
		else
			effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPES::MODEL_TYPE_ROOM);
	}

	if (!animated)
	{
		// Non animated buckets are rendered with vertex buffers
		RendererBucket* bucket = mesh->GetBucket(bucketIndex);
		setGpuStateForBucket(bucketIndex);

		m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
		m_device->SetIndices(bucket->GetIndexBuffer());

		for (int iPass = 0; iPass < cPasses; iPass++)
		{
			effect->BeginPass(iPass);
			effect->CommitChanges();

			drawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

			effect->EndPass();
		}
	}
	else
	{
		RendererBucket* bucket = mesh->GetAnimatedBucket(bucketIndex);
		setGpuStateForBucket(bucketIndex);

		for (int iPass = 0; iPass < cPasses; iPass++)
		{
			effect->BeginPass(iPass);
			effect->CommitChanges();

			m_device->DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, bucket->NumVertices, bucket->NumIndices / 3,
				bucket->Indices.data(), D3DFORMAT::D3DFMT_INDEX32, bucket->Vertices.data(), sizeof(RendererVertex));

			effect->EndPass();
		}
	}

	return true;
}

bool Renderer::drawStatic(__int32 roomIndex, __int32 staticIndex, RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass)
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

	setGpuStateForBucket(bucketIndex);

	LPD3DXEFFECT effect;
	if (pass == RENDERER_PASSES::RENDERER_PASS_SHADOW_MAP)
		effect = m_shaderDepth->GetEffect();
	else if (pass == RENDERER_PASSES::RENDERER_PASS_RECONSTRUCT_DEPTH)
		effect = m_shaderReconstructZBuffer->GetEffect();
	else if (pass == RENDERER_PASSES::RENDERER_PASS_GBUFFER)
		effect = m_shaderFillGBuffer->GetEffect();
	else
		effect = m_shaderTransparent->GetEffect();

	D3DXMatrixTranslation(&world, sobj->x, sobj->y, sobj->z);
	D3DXMatrixRotationY(&rotation, TR_ANGLE_TO_RAD(sobj->yRot));
	D3DXMatrixMultiply(&world, &rotation, &world);

	effect->SetBool(effect->GetParameterByName(NULL, "UseSkinning"), false);
	effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPES::MODEL_TYPE_STATIC);
	effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &world);
	effect->SetVector(effect->GetParameterByName(NULL, "AmbientLight"), &m_rooms[roomIndex]->AmbientLight);

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

		drawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

		effect->EndPass();
	}

	return true;
}

bool Renderer::drawGunFlashes()
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
	effect = m_shaderBasic->GetEffect();

	effect->SetBool(effect->GetParameterByName(NULL, "UseSkinning"), false);
	effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPES::MODEL_TYPE_MOVEABLE);
	effect->SetTexture(effect->GetParameterByName(NULL, "ModelTexture"), m_textureAtlas);
	effect->SetMatrix(effect->GetParameterByName(NULL, "View"), &ViewMatrix);
	effect->SetMatrix(effect->GetParameterByName(NULL, "Projection"), &ProjectionMatrix);

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

		for (__int32 b = 0; b < NUM_BUCKETS; b++)
		{
			RendererBucket* flashBucket = flashMesh->GetBucket(b);

			if (flashBucket->NumVertices != 0)
			{
				m_device->SetStreamSource(0, flashBucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
				m_device->SetIndices(flashBucket->GetIndexBuffer());

				D3DXMATRIX offset;
				D3DXMatrixTranslation(&offset, 0, length, zOffset);

				D3DXMATRIX rotation2;
				D3DXMatrixRotationX(&rotation2, TR_ANGLE_TO_RAD(rotationX));

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

						drawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, flashBucket->NumVertices, 0, flashBucket->NumIndices / 3);

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

						drawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, flashBucket->NumVertices, 0, flashBucket->NumIndices / 3);

						effect->EndPass();
					}
				}
			}
		}
	}

	return true;
}

void Renderer::collectLights()
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
			D3DXVECTOR3 lightVector = D3DXVECTOR3(room->Lights[j]->Position.x, room->Lights[j]->Position.y, room->Lights[j]->Position.z);

			if (D3DXVec3Length(&(laraPos-lightVector)) >= 1024.0f * 20.0f)
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

bool Renderer::drawHorizonAndSky()
{
	GameScriptLevel* level = g_Script->GetLevel(CurrentLevel);
	D3DXVECTOR4 color = D3DXVECTOR4(SkyColor1.r / 255.0f, SkyColor1.g / 255.0f, SkyColor1.b / 255.0f, 1.0f);

	// First update the sky in the case of storm
	if (level->Storm)
	{
		if (Unk_00E6D74C || Unk_00E6D73C)
		{
			UpdateStorm();
			if (StormTimer > -1)
				StormTimer--;
			if (!StormTimer)
				SoundEffect(SFX_THUNDER_RUMBLE, NULL, 0);
		}
		else if (!(rand() & 0x7F))
		{
			Unk_00E6D74C = (rand() & 0x1F) + 16;
			Unk_00E6E4DC = rand() + 256;
			StormTimer = (rand() & 3) + 12;
		}

		color = D3DXVECTOR4((SkyStormColor[0] + 44) / 255.0f, SkyStormColor[1] / 255.0f, SkyStormColor[2] / 255.0f, 1.0f);
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

			drawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, m_quad->NumVertices, 0, m_quad->NumIndices / 3);

			effect->EndPass();
		}
	}

	// Draw the horizon
	RendererObject* horizonObj = m_moveableObjects[ID_HORIZON];
	RendererMesh* mesh = horizonObj->ObjectMeshes[0];
	effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPES::MODEL_TYPE_HORIZON);

	effect->SetTexture(effect->GetParameterByName(NULL, "TextureAtlas"), m_textureAtlas);
	effect->SetVector(effect->GetParameterByName(NULL, "Color"), &D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));
	effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLEND_MODES::BLENDMODE_OPAQUE);

	D3DXMatrixTranslation(&world, Camera.pos.x, Camera.pos.y, Camera.pos.z);
	effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &world);
	
	for (__int32 i = 0; i < NUM_BUCKETS; i++)
	{
		RendererBucket* bucket = mesh->GetBucket(i);

		// Bind buffers
		m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
		m_device->SetIndices(bucket->GetIndexBuffer());

		setGpuStateForBucket((RENDERER_BUCKETS)i);

		for (int iPass = 0; iPass < cPasses; iPass++)
		{
			effect->BeginPass(iPass);
			effect->CommitChanges();

			drawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

			effect->EndPass();
		}
	}

	effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLEND_MODES::BLENDMODE_OPAQUE);

	m_device->Clear(0, NULL, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);

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

void Renderer::createBillboardMatrix(D3DXMATRIX* out, D3DXVECTOR3* particlePos, D3DXVECTOR3* cameraPos)
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

bool Renderer::drawSprites()
{
	setCullMode(RENDERER_CULLMODE::CULLMODE_NONE);
	setBlendState(RENDERER_BLENDSTATE::BLENDSTATE_ADDITIVE);
	setDepthWrite(false);

	UINT cPasses = 1;
	LPD3DXEFFECT effect = m_shaderSprites->GetEffect();
	m_device->BeginScene();
	effect->Begin(&cPasses, 0);

	effect->SetMatrix(effect->GetParameterByName(NULL, "View"), &ViewMatrix);
	effect->SetMatrix(effect->GetParameterByName(NULL, "Projection"), &ProjectionMatrix);
	effect->SetTexture(effect->GetParameterByName(NULL, "TextureAtlas"), m_textureAtlas);

	__int32 numSpritesToDraw = m_spritesToDraw.size();
	__int32 lastSprite = 0;
	while (numSpritesToDraw > 0)
	{
		m_spritesVertices.clear();
		m_spritesIndices.clear();

		// Fill the buffer for sprites
		__int32 maxSprite = min(m_spritesToDraw.size(), lastSprite + NUM_SPRITES_PER_BUCKET);
		for (__int32 i = lastSprite; i < maxSprite; i++)
		{
			RendererSpriteToDraw* spr = m_spritesToDraw[i];

			if (spr->Type == RENDERER_SPRITE_TYPE::SPRITE_TYPE_BILLBOARD)
			{
				float halfWidth = spr->Width / 2.0f;
				float halfHeight = spr->Height / 2.0f;

				D3DXMATRIX billboardMatrix;
				createBillboardMatrix(&billboardMatrix, &D3DXVECTOR3(spr->X, spr->Y, spr->Z),
					&D3DXVECTOR3(Camera.pos.x, Camera.pos.y, Camera.pos.z));

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
				v.u = spr->Sprite->UV[0].x;
				v.v = spr->Sprite->UV[0].y;
				v.r = spr->R / 255.0f;
				v.g = spr->G / 255.0f;
				v.b = spr->B / 255.0f;
				v.a = 1.0f;
				m_spritesVertices.push_back(v);

				v.x = p1t.x;
				v.y = p1t.y;
				v.z = p1t.z;
				v.u = spr->Sprite->UV[1].x;
				v.v = spr->Sprite->UV[1].y;
				v.r = spr->R / 255.0f;
				v.g = spr->G / 255.0f;
				v.b = spr->B / 255.0f;
				v.a = 1.0f;
				m_spritesVertices.push_back(v);

				v.x = p2t.x;
				v.y = p2t.y;
				v.z = p2t.z;
				v.u = spr->Sprite->UV[2].x;
				v.v = spr->Sprite->UV[2].y;
				v.r = spr->R / 255.0f;
				v.g = spr->G / 255.0f;
				v.b = spr->B / 255.0f;
				v.a = 1.0f;
				m_spritesVertices.push_back(v);

				v.x = p3t.x;
				v.y = p3t.y;
				v.z = p3t.z;
				v.u = spr->Sprite->UV[3].x;
				v.v = spr->Sprite->UV[3].y;
				v.r = spr->R / 255.0f;
				v.g = spr->G / 255.0f;
				v.b = spr->B / 255.0f;
				v.a = 1.0f;
				m_spritesVertices.push_back(v);

				m_spritesIndices.push_back(baseVertex + 0);
				m_spritesIndices.push_back(baseVertex + 1);
				m_spritesIndices.push_back(baseVertex + 2);
				m_spritesIndices.push_back(baseVertex + 0);
				m_spritesIndices.push_back(baseVertex + 2);
				m_spritesIndices.push_back(baseVertex + 3);
			}
			else
			{
				D3DXVECTOR3 p0t = D3DXVECTOR3(spr->X1, spr->Y1, spr->Z1);
				D3DXVECTOR3 p1t = D3DXVECTOR3(spr->X2, spr->Y2, spr->Z2);
				D3DXVECTOR3 p2t = D3DXVECTOR3(spr->X3, spr->Y3, spr->Z3);
				D3DXVECTOR3 p3t = D3DXVECTOR3(spr->X4, spr->Y4, spr->Z4);

				RendererVertex v;
				__int32 baseVertex = m_spritesVertices.size();

				v.x = p0t.x;
				v.y = p0t.y;
				v.z = p0t.z;
				v.u = spr->Sprite->UV[0].x;
				v.v = spr->Sprite->UV[0].y;
				v.r = spr->R / 255.0f;
				v.g = spr->G / 255.0f;
				v.b = spr->B / 255.0f;
				v.a = 1.0f;
				m_spritesVertices.push_back(v);

				v.x = p1t.x;
				v.y = p1t.y;
				v.z = p1t.z;
				v.u = spr->Sprite->UV[1].x;
				v.v = spr->Sprite->UV[1].y;
				v.r = spr->R / 255.0f;
				v.g = spr->G / 255.0f;
				v.b = spr->B / 255.0f;
				v.a = 1.0f;
				m_spritesVertices.push_back(v);

				v.x = p2t.x;
				v.y = p2t.y;
				v.z = p2t.z;
				v.u = spr->Sprite->UV[2].x;
				v.v = spr->Sprite->UV[2].y;
				v.r = spr->R / 255.0f;
				v.g = spr->G / 255.0f;
				v.b = spr->B / 255.0f;
				v.a = 1.0f;
				m_spritesVertices.push_back(v);

				v.x = p3t.x;
				v.y = p3t.y;
				v.z = p3t.z;
				v.u = spr->Sprite->UV[3].x;
				v.v = spr->Sprite->UV[3].y;
				v.r = spr->R / 255.0f;
				v.g = spr->G / 255.0f;
				v.b = spr->B / 255.0f;
				v.a = 1.0f;
				m_spritesVertices.push_back(v);

				m_spritesIndices.push_back(baseVertex + 0);
				m_spritesIndices.push_back(baseVertex + 1);
				m_spritesIndices.push_back(baseVertex + 2);
				m_spritesIndices.push_back(baseVertex + 0);
				m_spritesIndices.push_back(baseVertex + 2);
				m_spritesIndices.push_back(baseVertex + 3);
			}

			lastSprite++;
		}

		numSpritesToDraw -= NUM_SPRITES_PER_BUCKET;

		HRESULT res;

		void* vertices;

		res = m_spritesVertexBuffer->Lock(0, 0, &vertices, 0);
		if (res != S_OK)
			return false;

		memcpy(vertices, m_spritesVertices.data(), m_spritesVertices.size() * sizeof(RendererVertex));

		res = m_spritesVertexBuffer->Unlock();
		if (res != S_OK)
			return false;

		void* indices;

		res = m_spritesIndexBuffer->Lock(0, 0, &indices, 0);
		if (res != S_OK)
			return false;

		memcpy(indices, m_spritesIndices.data(), m_spritesIndices.size() * 4);

		m_spritesIndexBuffer->Unlock();
		if (res != S_OK)
			return false;

		m_device->SetStreamSource(0, m_spritesVertexBuffer, 0, sizeof(RendererVertex));
		m_device->SetIndices(m_spritesIndexBuffer);

		// Draw the sprites
		effect->BeginPass(0);
		effect->CommitChanges();

		drawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, m_spritesVertices.size(), 0, m_spritesIndices.size() / 3);

		effect->EndPass();
	}	

	effect->End();
	m_device->EndScene();

	return true;
}

void Renderer::addSpriteBillboard(RendererSprite* sprite, float x, float y, float z, byte r, byte g, byte b, float rotation, float scale, float width, float height)
{
	scale = 1.0f;

	width *= scale;
	height *= scale;

	RendererSpriteToDraw* spr = new RendererSpriteToDraw();
	spr->Type = RENDERER_SPRITE_TYPE::SPRITE_TYPE_BILLBOARD;
	spr->Sprite = sprite;
	spr->X = x;
	spr->Y = y;
	spr->Z = z;
	spr->R = r;
	spr->G = g;
	spr->B = b;
	spr->Rotation = rotation;
	spr->Scale = scale;
	spr->Width = width;
	spr->Height = height;

	m_spritesToDraw.push_back(spr);
}

void Renderer::drawFires()
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
					addSpriteBillboard(m_sprites[spark->def],
								fire->x + spark->x, fire->y + spark->y, fire->z + spark->z, 
								spark->r, spark->g, spark->b, 
								TR_ANGLE_TO_RAD(spark->rotAng), spark->scalar, spark->size * 4.0f, spark->size * 4.0f);
				}
			}
		}
	}
}

void Renderer::drawSmokes()
{
	for (__int32 i = 0; i < 32; i++)
	{
		SMOKE_SPARKS* spark = &SmokeSparks[i];
		if (spark->On)
		{
			addSpriteBillboard(m_sprites[spark->Def],
				spark->x, spark->y, spark->z,
				spark->Shade, spark->Shade, spark->Shade,
				TR_ANGLE_TO_RAD(spark->RotAng), spark->Scalar, spark->Size * 4.0f, spark->Size * 4.0f);
		}
	}
}

void Renderer::drawSparks()
{
	for (__int32 i = 0; i < 1024; i++)
	{
		SPARKS* spark = &Sparks[i];
		if (spark->on)
		{
			if (spark->flags & SP_DEF)
			{
				addSpriteBillboard(m_sprites[spark->def],
					spark->x, spark->y, spark->z,
					spark->r, spark->g, spark->b,
					TR_ANGLE_TO_RAD(spark->rotAng), spark->scalar, spark->size * 12.0f, spark->size * 12.0f);
			}
			else
			{
				D3DXVECTOR3 v = D3DXVECTOR3(spark->xVel, spark->yVel, spark->zVel);
				D3DXVec3Normalize(&v, &v);
				addLine3D(spark->x, spark->y, spark->z, spark->x + v.x * 24.0f, spark->y + v.y * 24.0f, spark->z + v.z * 24.0f, spark->r, spark->g, spark->b);
			}
		}
	}
}

void Renderer::drawBlood()
{
	for (__int32 i = 0; i < 32; i++)
	{
		BLOOD_STRUCT* blood = &Blood[i];
		if (blood->On)
		{
			addSpriteBillboard(m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + 15],
				blood->x, blood->y, blood->z,
				blood->Shade * 244, blood->Shade * 0, blood->Shade * 0,
				TR_ANGLE_TO_RAD(blood->RotAng), 1.0f, blood->Size * 8.0f, blood->Size * 8.0f);
		}
	}
}

bool Renderer::drawGunshells(RENDERER_BUCKETS bucketIndex, RENDERER_PASSES pass)
{
	D3DXMATRIX world;
	UINT cPasses = 1;

	LPD3DXEFFECT effect;
	if (pass == RENDERER_PASSES::RENDERER_PASS_SHADOW_MAP)
		effect = m_shaderDepth->GetEffect();
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

				setGpuStateForBucket(bucketIndex);

				m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
				m_device->SetIndices(bucket->GetIndexBuffer());

				for (int iPass = 0; iPass < cPasses; iPass++)
				{
					effect->BeginPass(iPass);
					effect->CommitChanges();

					drawPrimitives(D3DPRIMITIVETYPE::D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

					effect->EndPass();
				}
			}
		}

	}

	return true;
}

bool Renderer::doRain()
{
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
			drop->Y = LaraItem->pos.yPos - (m_firstWeather ? rand() % WEATHER_HEIGHT : WEATHER_HEIGHT);
			drop->Z = LaraItem->pos.zPos + rand() % WEATHER_RADIUS - WEATHER_RADIUS / 2.0f;

			// Check if in inside room
			__int16 roomNumber = Camera.pos.roomNumber;
			FLOOR_INFO* floor = GetFloor(drop->X, drop->Y, drop->Z, &roomNumber);
			ROOM_INFO* room = &Rooms[roomNumber];
			if (!(room->flags & ENV_FLAG_OUTSIDE))
			{
				drop->Reset = true;
				continue;
			}

			drop->Size = RAIN_SIZE + (rand() % 64);
			drop->AngleH = (rand() % RAIN_MAX_ANGLE_H) * RADIAN;
			drop->AngleV = (rand() % RAIN_MAX_ANGLE_V) * RADIAN;
			drop->Reset = false;
		}

		float x1 = drop->X;
		float y1 = drop->Y;
		float z1 = drop->Z;

		float radius = drop->Size * sin(drop->AngleV);

		float dx = sin(drop->AngleH) * radius;
		float dy = drop->Size * cos(drop->AngleV);
		float dz = cos(drop->AngleH) * radius;

		drop->X += dx;
		drop->Y += RAIN_DELTA_Y;
		drop->Z += dz;

		addLine3D(x1, y1, z1, drop->X, drop->Y, drop->Z, (byte)(RAIN_COLOR * 255.0f), (byte)(RAIN_COLOR * 255.0f), (byte)(RAIN_COLOR * 255.0f));

		// If rain drop has hit the ground, then reset it and add a little drip
		__int16 roomNumber = Camera.pos.roomNumber;
		FLOOR_INFO* floor = GetFloor(drop->X, drop->Y, drop->Z, &roomNumber);
		ROOM_INFO* room = &Rooms[roomNumber];
		if (drop->Y >= room->y + room->minfloor)
		{
			drop->Reset = true;
			AddWaterSparks(drop->X, room->y + room->minfloor, drop->Z, 1);
		}
	}

	m_firstWeather = false;

	return true;
}

bool Renderer::doSnow()
{
	if (m_firstWeather)
	{
		for (__int32 i = 0; i < NUM_SNOW_PARTICLES; i++)
			m_snow[i].Reset = true;
	}

	for (__int32 i = 0; i < NUM_SNOW_PARTICLES; i++)
	{
		RendererWeatherParticle* snow = &m_snow[i];

		if (snow->Reset)
		{
			snow->X = LaraItem->pos.xPos + rand() % WEATHER_RADIUS - WEATHER_RADIUS / 2.0f;
			snow->Y = LaraItem->pos.yPos - (m_firstWeather ? rand() % WEATHER_HEIGHT : WEATHER_HEIGHT) + (rand() % 512);
			snow->Z = LaraItem->pos.zPos + rand() % WEATHER_RADIUS - WEATHER_RADIUS / 2.0f;

			// Check if in inside room
			__int16 roomNumber = Camera.pos.roomNumber;
			FLOOR_INFO* floor = GetFloor(snow->X, snow->Y, snow->Z, &roomNumber);
			ROOM_INFO* room = &Rooms[roomNumber];
			if (!(room->flags & ENV_FLAG_OUTSIDE))
				continue;

			snow->Size = SNOW_DELTA_Y + (rand() % 64);
			snow->AngleH = (rand() % SNOW_MAX_ANGLE_H) * RADIAN;
			snow->AngleV = (rand() % SNOW_MAX_ANGLE_V) * RADIAN;
			snow->Reset = false;
		}

		float radius = snow->Size * sin(snow->AngleV);

		float dx = sin(snow->AngleH) * radius;
		float dz = cos(snow->AngleH) * radius;

		snow->X += dx;
		snow->Y += SNOW_DELTA_Y;
		snow->Z += dz;

		if (snow->X <= 0 || snow->Z <= 0 || snow->X >= 100 * 1024.0f || snow->Z >= 100 * 1024.0f)
		{
			snow->Reset = true;
			continue;
		}

		addSpriteBillboard(m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + 14], snow->X, snow->Y, snow->Z, 255, 255, 255,
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

void Renderer::addLine3D(__int32 x1, __int32 y1, __int32 z1, __int32 x2, __int32 y2, __int32 z2, byte r, byte g, byte b)
{
	RendererLine3DToDraw* line = new RendererLine3DToDraw();

	line->X1 = x1;
	line->Y1 = y1;
	line->Z1 = z1;
	line->X2 = x2;
	line->Y2 = y2;
	line->Z2 = z2;
	line->R = r;
	line->G = g;
	line->B = b;

	m_lines3DToDraw.push_back(line);
}

bool Renderer::drawLines3D()
{
	setCullMode(RENDERER_CULLMODE::CULLMODE_NONE);
	setBlendState(RENDERER_BLENDSTATE::BLENDSTATE_ADDITIVE);
	setDepthWrite(false);

	UINT cPasses = 1;
	LPD3DXEFFECT effect = m_shaderRain->GetEffect();
	m_device->BeginScene();
	effect->Begin(&cPasses, 0);

	effect->SetMatrix(effect->GetParameterByName(NULL, "View"), &ViewMatrix);
	effect->SetMatrix(effect->GetParameterByName(NULL, "Projection"), &ProjectionMatrix);

	__int32 numLinesToDraw = m_lines3DToDraw.size();
	__int32 lastLine = 0;
	while (numLinesToDraw > 0)
	{
		m_lines3DVertices.clear();

		// Fill the buffer for lines
		__int32 maxLine = min(m_lines3DToDraw.size(), lastLine + NUM_LINES_PER_BUCKET);
		for (__int32 i = lastLine; i < maxLine; i++)
		{
			RendererLine3DToDraw* line = m_lines3DToDraw[i];

			RendererVertex v;

			v.x = line->X1;
			v.y = line->Y1;
			v.z = line->Z1;
			v.r = line->R / 255.0f;
			v.g = line->G / 255.0f;
			v.b = line->B / 255.0f;
			v.a = 1.0f;
			m_lines3DVertices.push_back(v);

			v.x = line->X2;
			v.y = line->Y2;
			v.z = line->Z2;
			v.r = line->R / 255.0f;
			v.g = line->G / 255.0f;
			v.b = line->B / 255.0f;
			v.a = 1.0f;
			m_lines3DVertices.push_back(v);

			lastLine++;
		}

		numLinesToDraw -= NUM_LINES_PER_BUCKET;

		HRESULT res;

		void* vertices;

		res = m_linesVertexBuffer->Lock(0, 0, &vertices, 0);
		if (res != S_OK)
			return false;

		memcpy(vertices, m_lines3DVertices.data(), m_lines3DVertices.size() * sizeof(RendererVertex));

		res = m_linesVertexBuffer->Unlock();
		if (res != S_OK)
			return false;
		
		m_device->SetStreamSource(0, m_linesVertexBuffer, 0, sizeof(RendererVertex));
		m_device->SetIndices(NULL);

		// Draw the sprites
		effect->BeginPass(0);
		effect->CommitChanges();

		m_device->DrawPrimitive(D3DPRIMITIVETYPE::D3DPT_LINELIST, 0, m_lines3DVertices.size() / 2);

		effect->EndPass();
	}

	effect->End();
	m_device->EndScene();

	return true;
}

void Renderer::drawDrips()
{
	for (__int32 i = 0; i < 32; i++)
	{
		DRIP_STRUCT* drip = &Drips[i];
		
		if (drip->On)
		{
			addLine3D(drip->x, drip->y, drip->z, drip->x, drip->y + 24.0f, drip->z, drip->R, drip->G, drip->B);
		}
	}
}

void Renderer::setCullMode(RENDERER_CULLMODE mode)
{
	if (m_cullMode == mode)
		return;

	m_cullMode = mode;

	if (m_cullMode == RENDERER_CULLMODE::CULLMODE_NONE)
		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	else if (m_cullMode == RENDERER_CULLMODE::CULLMODE_CW)
		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	else
		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}

void Renderer::setBlendState(RENDERER_BLENDSTATE state)
{
	if (m_blendState == state)
		return;

	m_blendState = state;

	if (m_blendState == RENDERER_BLENDSTATE::BLENDSTATE_OPAQUE)
	{
		m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
	}
	else if (m_blendState == RENDERER_BLENDSTATE::BLENDSTATE_ADDITIVE)
	{
		m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		m_device->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ZERO);
		m_device->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ZERO);
		m_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		m_device->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
	}
	else if (m_blendState == RENDERER_BLENDSTATE::BLENDSTATE_ALPHABLEND)
	{
		m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
		m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
		m_device->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
		m_device->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
		m_device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		m_device->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
	}
	else if (m_blendState == RENDERER_BLENDSTATE::BLENDSTATE_SPECIAL_Z_BUFFER)
	{
		m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	}
}

void Renderer::setDepthWrite(bool value)
{
	if (m_enableZwrite == value)
		return;

	m_enableZwrite = value;

	m_device->SetRenderState(D3DRS_ZWRITEENABLE, value);
}

void Renderer::setGpuStateForBucket(RENDERER_BUCKETS bucket)
{
	switch (bucket)
	{
	case RENDERER_BUCKETS::RENDERER_BUCKET_SOLID:
		setCullMode(RENDERER_CULLMODE::CULLMODE_CCW);
		setBlendState(RENDERER_BLENDSTATE::BLENDSTATE_OPAQUE);
		setDepthWrite(true);
		break;

	case RENDERER_BUCKETS::RENDERER_BUCKET_SOLID_DS:
		setCullMode(RENDERER_CULLMODE::CULLMODE_NONE);
		setBlendState(RENDERER_BLENDSTATE::BLENDSTATE_OPAQUE);
		setDepthWrite(true);
		break;

	case RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST:
		setCullMode(RENDERER_CULLMODE::CULLMODE_CCW);
		setBlendState(RENDERER_BLENDSTATE::BLENDSTATE_OPAQUE);
		setDepthWrite(true);
		break;

	case RENDERER_BUCKETS::RENDERER_BUCKET_ALPHA_TEST_DS:
		setCullMode(RENDERER_CULLMODE::CULLMODE_NONE);
		setBlendState(RENDERER_BLENDSTATE::BLENDSTATE_OPAQUE);
		setDepthWrite(true);
		break;

	case RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT:
		setCullMode(RENDERER_CULLMODE::CULLMODE_CCW);
		setBlendState(RENDERER_BLENDSTATE::BLENDSTATE_ADDITIVE);
		setDepthWrite(false);
		break;

	case RENDERER_BUCKETS::RENDERER_BUCKET_TRANSPARENT_DS:
		setCullMode(RENDERER_CULLMODE::CULLMODE_NONE);
		setBlendState(RENDERER_BLENDSTATE::BLENDSTATE_ADDITIVE);
		setDepthWrite(false);
		break;
	}
}

void Renderer::drawBubbles()
{
	for (__int32 i = 0; i < 40; i++)
	{
		BUBBLE_STRUCT* bubble = &Bubbles[i];

		if (bubble->size)
		{
			addSpriteBillboard(m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + 13],
				bubble->pos.x, bubble->pos.y, bubble->pos.z,
				bubble->shade * 255, bubble->shade * 255, bubble->shade * 255,
				0.0f, 1.0f, bubble->size * 0.5f, bubble->size * 0.5f);
		}
	}
}

bool Renderer::isRoomUnderwater(__int16 roomNumber)
{ 
	return (m_rooms[roomNumber]->Room->flags & 1);
}
 
bool Renderer::isInRoom(__int32 x, __int32 y, __int32 z, __int16 roomNumber)
{
	RendererRoom* room = m_rooms[roomNumber];
	ROOM_INFO* r = room->Room;

	return (x >= r->x && x <= r->x + r->xSize * 1024.0f &&
			y >= r->maxceiling && y <= r->minfloor &&
			z >= r->z && z <= r->z + r->ySize * 1024.0f);
}

void Renderer::drawSplahes()
{
	for (__int32 i = 0; i < 4; i++)
	{
		SPLASH_STRUCT* splash = &Splashes[i];

		int x = 0;
	}
}

void Renderer::addSprite3D(RendererSprite* sprite, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4, byte r, byte g, byte b, float rotation, float scale, float width, float height)
{
	scale = 1.0f;

	width *= scale;
	height *= scale;

	RendererSpriteToDraw* spr = new RendererSpriteToDraw();
	spr->Type = RENDERER_SPRITE_TYPE::SPRITE_TYPE_3D;
	spr->Sprite = sprite;
	spr->X1 = x1;
	spr->Y1 = y1;
	spr->Z1 = z1;
	spr->X2 = x2;
	spr->Y2 = y2;
	spr->Z2 = z2;
	spr->X3 = x3;
	spr->Y3 = y3;
	spr->Z3 = z3;
	spr->X4 = x4;
	spr->Y4 = y4;
	spr->Z4 = z4;
	spr->R = r;
	spr->G = g;
	spr->B = b;
	spr->Rotation = rotation;
	spr->Scale = scale;
	spr->Width = width;
	spr->Height = height;

	m_spritesToDraw.push_back(spr);
}

void Renderer::drawRipples()
{
	for (__int32 i = 0; i < 32; i++)
	{
		RIPPLE_STRUCT* ripple = &Ripples[i];

		if (ripple->flags & 1)
		{
			float x1 = ripple->x - ripple->size;
			float z1 = ripple->z - ripple->size;
			float x2 = ripple->x + ripple->size;
			float z2 = ripple->z + ripple->size;
			float y = ripple->y;

			byte color = (ripple->init ? ripple->init << 1 : ripple->life << 1);

			addSprite3D(m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + 9],
				x1, y, z2, x2, y, z2, x2, y, z1, x1, y, z1, color, color, color, 0.0f, 1.0f, ripple->size, ripple->size);
		}
	}
}

void Renderer::drawUnderwaterDust()
{
	if (m_firstUnderwaterDustParticles)
	{
		for (__int32 i = 0; i < NUM_UNDERWATER_DUST_PARTICLES; i++)
			m_underwaterDustParticles[i].Reset = true;
	}
	 
	for (__int32 i = 0; i < NUM_UNDERWATER_DUST_PARTICLES; i++)
	{
		RendererUnderwaterDustParticle* dust = &m_underwaterDustParticles[i];
		 
		if (dust->Reset)
		{
			dust->X = LaraItem->pos.xPos + rand() % UNDERWATER_DUST_PARTICLES_RADIUS - UNDERWATER_DUST_PARTICLES_RADIUS / 2.0f;
			dust->Y = LaraItem->pos.yPos + rand() % UNDERWATER_DUST_PARTICLES_RADIUS - UNDERWATER_DUST_PARTICLES_RADIUS / 2.0f;
			dust->Z = LaraItem->pos.zPos + rand() % UNDERWATER_DUST_PARTICLES_RADIUS - UNDERWATER_DUST_PARTICLES_RADIUS / 2.0f;

			// Check if water room
			__int16 roomNumber = Camera.pos.roomNumber;
			FLOOR_INFO* floor = GetFloor(dust->X, dust->Y, dust->Z, &roomNumber);
			if (!isRoomUnderwater(roomNumber))
				continue;

			if (!isInRoom(dust->X, dust->Y, dust->Z, roomNumber))
			{
				dust->Reset = true;
				continue;
			}

			dust->Life = 0;
			dust->Reset = false;
		}

		dust->Life++;
		byte color = (dust->Life > 16 ? 32 - dust->Life : dust->Life) * 4;
		
		addSpriteBillboard(m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + 14], dust->X, dust->Y, dust->Z, color, color, color,
			0.0f, 1.0f, UNDERWATER_DUST_PARTICLES_SIZE, UNDERWATER_DUST_PARTICLES_SIZE);

		if (dust->Life >= 32)
			dust->Reset = true;
	}

	m_firstUnderwaterDustParticles = false;

	return;
}

__int32 Renderer::getAnimatedTextureInfo(__int16 textureId)
{
	for (__int32 i = 0; i < m_animatedTextureSets.size(); i++)
	{
		RendererAnimatedTextureSet* set = m_animatedTextureSets[i];
		for (__int32 j = 0; j < set->Textures.size(); j++)
		{
			if (set->Textures[j]->Id == textureId)
				return i;
		}
	}

	return -1;
}

void Renderer::updateAnimatedTextures()
{
	for (__int32 i = 0; i < m_rooms.size(); i++)
	{
		RendererRoom* room = m_rooms[i];
		RendererObject* roomObj = room->RoomObject;
		if (roomObj->ObjectMeshes.size() == 0)
			continue;

		RendererMesh* mesh = roomObj->ObjectMeshes[0];

		for (__int32 bucketIndex = 0; bucketIndex < NUM_BUCKETS; bucketIndex++)
		{
			RendererBucket* bucket = mesh->GetAnimatedBucket(bucketIndex);
			if (bucket->Vertices.size() == 0)
				continue;

			for (__int32 p = 0; p < bucket->Polygons.size(); p++)
			{
				RendererPolygon* polygon = &bucket->Polygons[p];
				RendererAnimatedTextureSet* set = m_animatedTextureSets[polygon->AnimatedSet];
				__int32 textureIndex = -1;
				for (__int32 j = 0; j < set->Textures.size(); j++)
				{
					if (set->Textures[j]->Id == polygon->TextureId)
					{
						textureIndex = j;
						break;
					}
				}
				if (textureIndex == -1)
					continue;

				if (textureIndex == set->Textures.size() - 1)
					textureIndex = 0;
				else
					textureIndex++;

				polygon->TextureId = set->Textures[textureIndex]->Id;

				for (__int32 v = 0; v < (polygon->Shape == SHAPE_RECTANGLE ? 4 : 3); v++)
				{
					bucket->Vertices[polygon->Indices[v]].u = set->Textures[textureIndex]->UV[v].x;
					bucket->Vertices[polygon->Indices[v]].v = set->Textures[textureIndex]->UV[v].y;
				}
			}
		}
	}
}

void Renderer::updateAnimation(RendererObject* obj, __int16** frmptr, __int16 frac, __int16 rate, __int32 mask)
{
	stack<RendererBone*> bones;
	D3DXMATRIX rotation;

	bones.push(obj->Skeleton);
	while (!bones.empty())
	{
		// Pop the last bone in the stack
		RendererBone* bone = bones.top();
		bones.pop();

		bool calculateMatrix = (mask >> bone->Index) & 1;
		if (calculateMatrix)
		{
			D3DXVECTOR3 p = D3DXVECTOR3((int)*(frmptr[0] + 6), (int)*(frmptr[0] + 7), (int)*(frmptr[0] + 8));

			fromTrAngle(&rotation, frmptr[0], bone->Index);

			if (frac)
			{
				D3DXVECTOR3 p2 = D3DXVECTOR3((int)*(frmptr[1] + 6), (int)*(frmptr[1] + 7), (int)*(frmptr[1] + 8));
				D3DXVec3Lerp(&p, &p, &p2, frac / ((float)rate));

				D3DXMATRIX rotation2;
				fromTrAngle(&rotation2, frmptr[1], bone->Index);

				D3DXQUATERNION q1, q2, q3;

				D3DXQuaternionRotationMatrix(&q1, &rotation);
				D3DXQuaternionRotationMatrix(&q2, &rotation2);

				D3DXQuaternionSlerp(&q3, &q1, &q2, frac / ((float)rate));

				D3DXMatrixRotationQuaternion(&rotation, &q3);
			}

			D3DXMATRIX translation;
			if (bone == obj->Skeleton)
				D3DXMatrixTranslation(&translation, p.x, p.y, p.z);

			D3DXMATRIX extraRotation;
			D3DXMatrixRotationYawPitchRoll(&extraRotation, bone->ExtraRotation.y, bone->ExtraRotation.x, bone->ExtraRotation.z);

			if (bone != obj->Skeleton)
				D3DXMatrixMultiply(&obj->AnimationTransforms[bone->Index], &extraRotation, &bone->Transform);
			else
				D3DXMatrixMultiply(&obj->AnimationTransforms[bone->Index], &extraRotation, &translation);

			D3DXMatrixMultiply(&obj->AnimationTransforms[bone->Index], &rotation, &obj->AnimationTransforms[bone->Index]);
			
			if (bone != obj->Skeleton)
				D3DXMatrixMultiply(&obj->AnimationTransforms[bone->Index], &obj->AnimationTransforms[bone->Index], &obj->AnimationTransforms[bone->Parent->Index]);
		}

		for (__int32 i = 0; i < bone->Children.size(); i++)
		{
			bones.push(bone->Children[i]);
		}
	}
}

void Renderer::FreeRendererData()
{
	DX_RELEASE(m_textureAtlas);
	m_textureAtlas = NULL;

	DX_RELEASE(m_fontAndMiscTexture);
	m_fontAndMiscTexture = NULL;

	DX_RELEASE(m_skyTexture);
	m_skyTexture = NULL;

	for (map<__int32, RendererRoom*>::iterator it = m_rooms.begin(); it != m_rooms.end(); ++it)
		delete (it->second);
	m_rooms.clear();

	for (map<__int32, RendererObject*>::iterator it = m_moveableObjects.begin(); it != m_moveableObjects.end(); ++it)
		delete (it->second);
	m_moveableObjects.clear();

	for (map<__int32, RendererObject*>::iterator it = m_staticObjects.begin(); it != m_staticObjects.end(); ++it)
		delete (it->second);
	m_staticObjects.clear();

	for (vector<RendererAnimatedTextureSet*>::iterator it = m_animatedTextureSets.begin(); it != m_animatedTextureSets.end(); ++it)
		delete (*it);
	m_animatedTextureSets.clear();

	for (map<__int32, RendererSprite*>::iterator it = m_sprites.begin(); it != m_sprites.end(); ++it)
		delete (it->second);
	m_sprites.clear();

	for (map<__int32, RendererSpriteSequence*>::iterator it = m_spriteSequences.begin(); it != m_spriteSequences.end(); ++it)
		delete (it->second);
	m_spriteSequences.clear();
}