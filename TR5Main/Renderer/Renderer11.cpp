#include "Renderer11.h"

#include "..\Specific\input.h"
#include "..\Specific\winmain.h"
#include "..\Specific\roomload.h"
#include "..\Specific\game.h"
#include "..\Specific\configuration.h"

#include "..\Game\draw.h"
#include "..\Game\healt.h"
#include "..\Game\pickup.h"
#include "..\Game\inventory.h"
#include "..\Game\gameflow.h"
#include "..\Game\lara.h"
#include "..\Game\effect2.h"
#include "..\Game\rope.h"
#include "..\Game\items.h"
#include "..\Game\camera.h"

#include <D3Dcompiler.h>
#include <chrono> 
#include <stack>

using ns = chrono::nanoseconds;
using get_time = chrono::steady_clock;

extern GameConfiguration g_Configuration;
extern GameFlow* g_GameFlow;
extern __int32 NumTextureTiles;
extern Inventory* g_Inventory;

__int32 SortLightsFunction(RendererLight* a, RendererLight* b)
{
	if (a->Dynamic > b->Dynamic)
		return -1;
	return 0;
}

bool SortRoomsFunction(RendererRoom* a, RendererRoom* b)
{
	return (a->Distance < b->Distance);
}

__int32 SortRoomsFunctionNonStd(RendererRoom* a, RendererRoom* b)
{
	return (a->Distance - b->Distance);
}

Renderer11::Renderer11()
{
	initialiseHairRemaps();

	m_blinkColorDirection = 1;
}

Renderer11::~Renderer11()
{
	DX11_RELEASE(m_device);
	DX11_RELEASE(m_context);
	DX11_RELEASE(m_swapChain);
	DX11_RELEASE(m_backBufferRTV);
	DX11_RELEASE(m_backBufferTexture);
	DX11_RELEASE(m_depthStencilState);
	DX11_RELEASE(m_depthStencilTexture);
	DX11_RELEASE(m_depthStencilView);

	DX11_DELETE(m_primitiveBatch);
	DX11_DELETE(m_spriteBatch);
	DX11_DELETE(m_gameFont);
	DX11_DELETE(m_states);

	for (__int32 i = 0; i < NUM_CAUSTICS_TEXTURES; i++)
		DX11_DELETE(m_caustics[i]);

	DX11_DELETE(m_titleScreen);
	DX11_DELETE(m_binocularsTexture);
	DX11_DELETE(m_whiteTexture);

	DX11_RELEASE(m_vsRooms);
	DX11_RELEASE(m_psRooms);
	DX11_RELEASE(m_vsItems);
	DX11_RELEASE(m_psItems);
	DX11_RELEASE(m_vsStatics);
	DX11_RELEASE(m_psStatics);
	DX11_RELEASE(m_vsHairs);
	DX11_RELEASE(m_psHairs);
	DX11_RELEASE(m_vsSky);
	DX11_RELEASE(m_psSky);
	DX11_RELEASE(m_vsSprites);
	DX11_RELEASE(m_psSprites);
	DX11_RELEASE(m_vsSolid);
	DX11_RELEASE(m_psSolid);
	DX11_RELEASE(m_vsInventory);
	DX11_RELEASE(m_psInventory);
	DX11_RELEASE(m_vsFullScreenQuad);
	DX11_RELEASE(m_psFullScreenQuad);
	DX11_RELEASE(m_cbCameraMatrices);
	DX11_RELEASE(m_cbItem);
	DX11_RELEASE(m_cbStatic);
	DX11_RELEASE(m_cbLights);
	DX11_RELEASE(m_cbMisc);

	DX11_DELETE(m_renderTarget);
	DX11_DELETE(m_dumpScreenRenderTarget);
	DX11_DELETE(m_shadowMap);

	FreeRendererData();
}

void Renderer11::FreeRendererData()
{
	m_meshPointersToMesh.clear();

	for (__int32 i = 0; i < NUM_OBJECTS; i++)
		DX11_DELETE(m_moveableObjects[i]);
	free(m_moveableObjects);

	for (__int32 i = 0; i < g_NumSprites; i++)
		DX11_DELETE(m_sprites[i]);
	free(m_sprites);

	for (__int32 i = 0; i < NUM_OBJECTS; i++)
		DX11_DELETE(m_spriteSequences[i]);
	free(m_spriteSequences);

	for (__int32 i = 0; i < NUM_STATICS; i++)
		DX11_DELETE(m_staticObjects[i]);
	free(m_staticObjects);

	for (__int32 i = 0; i < NUM_ROOMS; i++)
		DX11_DELETE(m_rooms[i]);
	free(m_rooms);

	for (__int32 i = 0; i < m_numAnimatedTextureSets; i++)
		DX11_DELETE(m_animatedTextureSets[i]);
	free(m_animatedTextureSets);

	DX11_DELETE(m_textureAtlas);
	DX11_DELETE(m_textureAtlas);
	DX11_DELETE(m_skyTexture);

	DX11_DELETE(m_roomsVertexBuffer);
	DX11_DELETE(m_roomsIndexBuffer);

	DX11_DELETE(m_moveablesVertexBuffer);
	DX11_DELETE(m_moveablesIndexBuffer);

	DX11_DELETE(m_staticsVertexBuffer);
	DX11_DELETE(m_staticsIndexBuffer);
}

bool Renderer11::Create()
{
	D3D_FEATURE_LEVEL levels[1] = { D3D_FEATURE_LEVEL_10_1 };
	D3D_FEATURE_LEVEL featureLevel;

	HRESULT res = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, levels, 1, D3D11_SDK_VERSION,
		&m_device, &featureLevel, &m_context);
	if (FAILED(res))
		return false;

	return true;
}

bool Renderer11::EnumerateVideoModes()
{
	HRESULT res;

	IDXGIFactory* dxgiFactory = NULL;
	res = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
	if (FAILED(res))
		return false;

	IDXGIAdapter* dxgiAdapter = NULL;

	for (int i = 0; dxgiFactory->EnumAdapters(i, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND; i++)
	{
		DXGI_ADAPTER_DESC adapterDesc;
		UINT stringLength;
		char videoCardDescription[128];

		dxgiAdapter->GetDesc(&adapterDesc);
		__int32 error = wcstombs_s(&stringLength, videoCardDescription, 128, adapterDesc.Description, 128);

		RendererVideoAdapter adapter;

		adapter.Index = i;
		adapter.Name = videoCardDescription;

		printf("Adapter %d\n", i);
		printf("\t Device Name: %s\n", videoCardDescription);

		IDXGIOutput* output = NULL;
		res = dxgiAdapter->EnumOutputs(0, &output);
		if (FAILED(res))
			return false;

		UINT numModes = 0;
		DXGI_MODE_DESC* displayModes = NULL;
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;

		// Get the number of elements
		res = output->GetDisplayModeList(format, 0, &numModes, NULL);
		if (FAILED(res))
			return false;

		// Get the list
		displayModes = new DXGI_MODE_DESC[numModes];
		res = output->GetDisplayModeList(format, 0, &numModes, displayModes);
		if (FAILED(res))
		{
			delete displayModes;
			return false;
		}

		for (__int32 j = 0; j < numModes; j++)
		{
			DXGI_MODE_DESC* mode = &displayModes[j];

			RendererDisplayMode newMode;

			// discard lower resolutions
			if (mode->Width < 1024 || mode->Height < 768)
				continue;

			newMode.Width = mode->Width;
			newMode.Height = mode->Height;
			newMode.RefreshRate = mode->RefreshRate.Numerator / mode->RefreshRate.Denominator;

			bool found = false;
			for (__int32 k = 0; k < adapter.DisplayModes.size(); k++)
			{
				RendererDisplayMode* currentMode = &adapter.DisplayModes[k];
				if (currentMode->Width == newMode.Width && currentMode->Height == newMode.Height &&
					currentMode->RefreshRate == newMode.RefreshRate)
				{
					found = true;
					break;
				}
			}
			if (found)
				continue;

			adapter.DisplayModes.push_back(newMode);

			printf("\t\t %d x %d %d Hz\n", newMode.Width, newMode.Height, newMode.RefreshRate);
		}

		m_adapters.push_back(adapter);	

		delete displayModes;
	}

	dxgiFactory->Release();

	return true;
}

bool Renderer11::initialiseScreen(__int32 w, __int32 h, __int32 refreshRate, bool windowed, HWND handle, bool reset)
{
	HRESULT res;

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = w;
	sd.BufferDesc.Height = h;
	sd.BufferDesc.RefreshRate.Numerator = refreshRate;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	sd.Windowed = windowed;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.OutputWindow = handle;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferCount = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	
	IDXGIDevice* dxgiDevice = NULL;
	res = m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)& dxgiDevice);
	if (FAILED(res))
		return false;

	IDXGIAdapter* dxgiAdapter = NULL;
	res = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)& dxgiAdapter);
	if (FAILED(res))
		return false;

	IDXGIFactory* dxgiFactory = NULL;
	res = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)& dxgiFactory);
	if (FAILED(res))
		return false;

	if (reset)
	{
		// Always return to windowed mode otherwise crash will happen
		m_swapChain->SetFullscreenState(false, NULL);
		m_swapChain->Release();
	}

	m_swapChain = NULL;
	res = dxgiFactory->CreateSwapChain(m_device, &sd, &m_swapChain);
	if (FAILED(res))
		return false;

	dxgiFactory->MakeWindowAssociation(handle, 0);
	res = m_swapChain->SetFullscreenState(!windowed, NULL);

	dxgiDevice->Release();
	dxgiAdapter->Release();
	dxgiFactory->Release();

	// Initialise the back buffer
	m_backBufferTexture = NULL;
	res = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast <void**>(&m_backBufferTexture));
	if (FAILED(res))
		return false;

	m_backBufferRTV = NULL;
	res = m_device->CreateRenderTargetView(m_backBufferTexture, NULL, &m_backBufferRTV);
	if (FAILED(res))
		return false;

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = w;
	depthStencilDesc.Height = h;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	m_depthStencilTexture = NULL;
	res = m_device->CreateTexture2D(&depthStencilDesc, NULL, &m_depthStencilTexture);
	if (FAILED(res))
		return false;

	m_depthStencilView = NULL;
	res = m_device->CreateDepthStencilView(m_depthStencilTexture, NULL, &m_depthStencilView);
	if (FAILED(res))
		return false;

	// Bind the back buffer and the depth stencil
	m_context->OMSetRenderTargets(1, &m_backBufferRTV, m_depthStencilView);

	// Initialise sprites and font
	m_spriteBatch = new SpriteBatch(m_context);
	m_gameFont = new SpriteFont(m_device, L"Font.spritefont");
	m_primitiveBatch = new PrimitiveBatch<RendererVertex>(m_context);

	// Initialise buffers
	m_renderTarget = RenderTarget2D::Create(m_device, w, h, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_dumpScreenRenderTarget = RenderTarget2D::Create(m_device, w, h, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_shadowMap = RenderTarget2D::Create(m_device, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, DXGI_FORMAT_R32_FLOAT);
	
	// Initialise viewport
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = w;
	m_viewport.Height = h;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

	m_shadowMapViewport.TopLeftX = 0;
	m_shadowMapViewport.TopLeftY = 0;
	m_shadowMapViewport.Width = SHADOW_MAP_SIZE;
	m_shadowMapViewport.Height = SHADOW_MAP_SIZE;
	m_shadowMapViewport.MinDepth = 0.0f;
	m_shadowMapViewport.MaxDepth = 1.0f;

	m_viewportToolkit = new Viewport(m_viewport.TopLeftX, m_viewport.TopLeftY, m_viewport.Width, m_viewport.Height,
									 m_viewport.MinDepth, m_viewport.MaxDepth);

	if (windowed)
	{
		SetWindowLong(WindowsHandle, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		SetWindowPos(WindowsHandle, HWND_TOP, 0, 0, g_Configuration.Width, g_Configuration.Height,
			SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	}
	else
	{
		SetWindowLong(WindowsHandle, GWL_STYLE, WS_POPUPWINDOW);
		SetWindowPos(WindowsHandle, HWND_TOP, 0, 0, 0, 0,
			SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	}

	UpdateWindow(handle);

	return true;
}

bool Renderer11::Initialise(__int32 w, __int32 h, __int32 refreshRate, bool windowed, HWND handle)
{
	HRESULT res;

	DB_Log(2, "Renderer::Initialise - DLL");
	printf("Initialising DX11\n");

	CoInitialize(NULL);
	 
	ScreenWidth = w;
	ScreenHeight = h;
	Windowed = windowed;

	if (!initialiseScreen(w, h, refreshRate, windowed, handle, false))
		return false;

	// Initialise render states
	m_states = new CommonStates(m_device);

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
		m_caustics[i] = Texture2D::LoadFromFile(m_device, causticsNames[i]);
		if (m_caustics[i] == NULL)
			return false;
	}

	m_titleScreen = Texture2D::LoadFromFile(m_device, "Screens\\Title.jpg");
	if (m_titleScreen == NULL)
		return false;

	m_binocularsTexture = Texture2D::LoadFromFile(m_device, "Binoculars.png");
	if (m_binocularsTexture == NULL)
		return false;

	m_whiteTexture = Texture2D::LoadFromFile(m_device, "WhiteSprite.png");
	if (m_whiteTexture == NULL)
		return false;

	// Load shaders
	ID3D10Blob* blob;

	m_vsRooms = compileVertexShader("Shaders\\DX11_Rooms.fx", "VS", "vs_4_0", &blob);
	if (m_vsRooms == NULL)
		return false;

	// Initialise input layout using the first vertex shader
	D3D11_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BLENDINDICES", 0, DXGI_FORMAT_R32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	m_inputLayout = NULL;
	res = m_device->CreateInputLayout(inputLayout, 5, blob->GetBufferPointer(), blob->GetBufferSize(), &m_inputLayout);
	if (FAILED(res))
		return false;

	m_psRooms = compilePixelShader("Shaders\\DX11_Rooms.fx", "PS", "ps_4_0", &blob);
	if (m_psRooms == NULL)
		return false;

	m_vsItems = compileVertexShader("Shaders\\DX11_Items.fx", "VS", "vs_4_0", &blob);
	if (m_vsItems == NULL)
		return false;

	m_psItems = compilePixelShader("Shaders\\DX11_Items.fx", "PS", "ps_4_0", &blob);
	if (m_psItems == NULL)
		return false;

	m_vsStatics = compileVertexShader("Shaders\\DX11_Statics.fx", "VS", "vs_4_0", &blob);
	if (m_vsStatics == NULL)
		return false;

	m_psStatics = compilePixelShader("Shaders\\DX11_Statics.fx", "PS", "ps_4_0", &blob);
	if (m_psStatics == NULL)
		return false;

	m_vsHairs = compileVertexShader("Shaders\\DX11_Hairs.fx", "VS", "vs_4_0", &blob);
	if (m_vsHairs == NULL)
		return false;

	m_psHairs = compilePixelShader("Shaders\\DX11_Hairs.fx", "PS", "ps_4_0", &blob);
	if (m_psHairs == NULL)
		return false;

	m_vsSky = compileVertexShader("Shaders\\DX11_Sky.fx", "VS", "vs_4_0", &blob);
	if (m_vsSky == NULL)
		return false;

	m_psSky = compilePixelShader("Shaders\\DX11_Sky.fx", "PS", "ps_4_0", &blob);
	if (m_psSky == NULL)
		return false;

	m_vsSprites = compileVertexShader("Shaders\\DX11_Sprites.fx", "VS", "vs_4_0", &blob);
	if (m_vsSprites == NULL)
		return false;

	m_psSprites = compilePixelShader("Shaders\\DX11_Sprites.fx", "PS", "ps_4_0", &blob);
	if (m_psSprites == NULL)
		return false;

	m_vsSolid = compileVertexShader("Shaders\\DX11_Solid.fx", "VS", "vs_4_0", &blob);
	if (m_vsSolid == NULL)
		return false;

	m_psSolid = compilePixelShader("Shaders\\DX11_Solid.fx", "PS", "ps_4_0", &blob);
	if (m_psSolid == NULL)
		return false;

	m_vsInventory = compileVertexShader("Shaders\\DX11_Inventory.fx", "VS", "vs_4_0", &blob);
	if (m_vsInventory == NULL)
		return false;

	m_psInventory = compilePixelShader("Shaders\\DX11_Inventory.fx", "PS", "ps_4_0", &blob);
	if (m_psInventory == NULL)
		return false;

	m_vsFullScreenQuad = compileVertexShader("Shaders\\DX11_FullScreenQuad.fx", "VS", "vs_4_0", &blob);
	if (m_vsFullScreenQuad == NULL)
		return false;

	m_psFullScreenQuad = compilePixelShader("Shaders\\DX11_FullScreenQuad.fx", "PS", "ps_4_0", &blob);
	if (m_psFullScreenQuad == NULL)
		return false;

	m_vsShadowMap = compileVertexShader("Shaders\\DX11_ShadowMap.fx", "VS", "vs_4_0", &blob);
	if (m_vsShadowMap == NULL)
		return false;

	m_psShadowMap = compilePixelShader("Shaders\\DX11_ShadowMap.fx", "PS", "ps_4_0", &blob);
	if (m_psShadowMap == NULL)
		return false;

	// Initialise constant buffers
	m_cbCameraMatrices = createConstantBuffer(sizeof(CCameraMatrixBuffer));
	m_cbItem = createConstantBuffer(sizeof(CItemBuffer));
	m_cbStatic = createConstantBuffer(sizeof(CStaticBuffer));
	m_cbLights = createConstantBuffer(sizeof(CLightBuffer));
	m_cbMisc = createConstantBuffer(sizeof(CMiscBuffer));
	m_cbShadowMap = createConstantBuffer(sizeof(CShadowLightBuffer));

	m_currentCausticsFrame = 0;
	m_firstWeather = true;

	// Preallocate lists
	m_roomsToDraw.Reserve(NumberRooms);
	m_itemsToDraw.Reserve(NUM_ITEMS);
	m_effectsToDraw.Reserve(NUM_ITEMS);
	m_lightsToDraw.Reserve(16384);
	m_dynamicLights.Reserve(16384);
	m_staticsToDraw.Reserve(16384);
	m_spritesToDraw.Reserve(MAX_SPRITES);
	m_lines3DToDraw.Reserve(MAX_LINES_3D);
	m_lines2DToDraw.Reserve(MAX_LINES_2D);
	m_tempItemLights.Reserve(MAX_LIGHTS);
	m_spritesBuffer = (RendererSpriteToDraw*)malloc(sizeof(RendererSpriteToDraw) * MAX_SPRITES);
	m_lines3DBuffer = (RendererLine3D*)malloc(sizeof(RendererLine3D) * MAX_LINES_3D);
	m_lines2DBuffer = (RendererLine2D*)malloc(sizeof(RendererLine2D) * MAX_LINES_2D);
	m_pickupRotation = 0;

	for (__int32 i = 0; i < NUM_ITEMS; i++)
	{
		m_items[i].Lights.Reserve(MAX_LIGHTS_PER_ITEM);
	}

	m_textureAtlas = NULL;
	m_skyTexture = NULL;

	return true;
}

__int32	Renderer11::Draw()
{
	drawScene(false);
	drawFinalPass();

	return 0;
}

void Renderer11::UpdateCameraMatrices(float posX, float posY, float posZ, float targetX, float targetY, float targetZ, float roll, float fov)
{
	g_Configuration.MaxDrawDistance = 200;

	FieldOfView = fov;
	View = Matrix::CreateLookAt(Vector3(posX, posY, posZ), Vector3(targetX, targetY, targetZ), -Vector3::UnitY);
	Projection = Matrix::CreatePerspectiveFieldOfView(fov, ScreenWidth / (float)ScreenHeight, 20.0f, g_Configuration.MaxDrawDistance * 1024.0f);

	m_stCameraMatrices.View = View; 
	m_stCameraMatrices.Projection = Projection;
}

bool Renderer11::drawAmbientCubeMap(__int16 roomNumber)
{
	return true;
}

void Renderer11::clearSceneItems()
{
	m_roomsToDraw.Clear();
	m_itemsToDraw.Clear();
	m_effectsToDraw.Clear();
	m_lightsToDraw.Clear();
	m_staticsToDraw.Clear();
	m_spritesToDraw.Clear();
	m_lines3DToDraw.Clear();
	m_lines2DToDraw.Clear();
}

bool Renderer11::drawHorizonAndSky()
{
	// Update the sky
	GameScriptLevel* level = g_GameFlow->GetLevel(CurrentLevel);
	Vector4 color = Vector4(SkyColor1.r / 255.0f, SkyColor1.g / 255.0f, SkyColor1.b / 255.0f, 1.0f);

	if (!level->Horizon)
		return true;

	if (BinocularRange)
		phd_AlterFOV(14560 - BinocularRange);

	// Storm
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

		color = Vector4((SkyStormColor[0]) / 255.0f, SkyStormColor[1] / 255.0f, SkyStormColor[2] / 255.0f, 1.0f);
	}

	ID3D11SamplerState* sampler;
	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	// Draw the sky
	Matrix rotation = Matrix::CreateRotationX(PI);

	RendererVertex vertices[4];
	float size = 9728.0f;

	vertices[0].Position.x = -size / 2.0f;
	vertices[0].Position.y = 0.0f;
	vertices[0].Position.z = size / 2.0f;
	vertices[0].UV.x = 0.0f;
	vertices[0].UV.y = 0.0f;
	vertices[0].Color.x = 1.0f;
	vertices[0].Color.y = 1.0f;
	vertices[0].Color.z = 1.0f;
	vertices[0].Color.w = 1.0f;

	vertices[1].Position.x = size / 2.0f;
	vertices[1].Position.y = 0.0f;
	vertices[1].Position.z = size / 2.0f;
	vertices[1].UV.x = 1.0f;
	vertices[1].UV.y = 0.0f;
	vertices[1].Color.x = 1.0f;
	vertices[1].Color.y = 1.0f;
	vertices[1].Color.z = 1.0f;
	vertices[1].Color.w = 1.0f;

	vertices[2].Position.x = size / 2.0f;
	vertices[2].Position.y = 0.0f;
	vertices[2].Position.z = -size / 2.0f;
	vertices[2].UV.x = 1.0f;
	vertices[2].UV.y = 1.0f;
	vertices[2].Color.x = 1.0f;
	vertices[2].Color.y = 1.0f;
	vertices[2].Color.z = 1.0f;
	vertices[2].Color.w = 1.0f;

	vertices[3].Position.x = -size / 2.0f;
	vertices[3].Position.y = 0.0f;
	vertices[3].Position.z = -size / 2.0f;
	vertices[3].UV.x = 0.0f;
	vertices[3].UV.y = 1.0f;
	vertices[3].Color.x = 1.0f;
	vertices[3].Color.y = 1.0f;
	vertices[3].Color.z = 1.0f;
	vertices[3].Color.w = 1.0f;

	m_context->VSSetShader(m_vsSky, NULL, 0);
	m_context->PSSetShader(m_psSky, NULL, 0);

	m_stCameraMatrices.View = View.Transpose();
	m_stCameraMatrices.Projection = Projection.Transpose();
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	m_stMisc.AlphaTest = true;
	updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
	m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

	m_context->PSSetShaderResources(0, 1, &m_skyTexture->ShaderResourceView);
	sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);
	
	for (__int32 i = 0; i < 2; i++)
	{
		Matrix translation = Matrix::CreateTranslation(Camera.pos.x + SkyPos1 - i * 9728.0f, Camera.pos.y - 1536.0f, Camera.pos.z);
		Matrix world = rotation * translation;

		m_stStatic.World = (rotation * translation).Transpose();
		m_stStatic.Color = color;
		updateConstantBuffer(m_cbStatic, &m_stStatic, sizeof(CStaticBuffer));
		m_context->VSSetConstantBuffers(1, 1, &m_cbStatic);
		m_context->PSSetConstantBuffers(1, 1, &m_cbStatic);

		m_primitiveBatch->Begin();
		m_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
		m_primitiveBatch->End();
	}

	// Draw horizon
	if (m_moveableObjects[ID_HORIZON] != NULL)
	{
		m_context->IASetVertexBuffers(0, 1, &m_moveablesVertexBuffer->Buffer, &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout);
		m_context->IASetIndexBuffer(m_moveablesIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

		m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
		sampler = m_states->AnisotropicClamp();
		m_context->PSSetSamplers(0, 1, &sampler);

		RendererObject* moveableObj = m_moveableObjects[ID_HORIZON];

		m_stStatic.World = Matrix::CreateTranslation(Camera.pos.x, Camera.pos.y, Camera.pos.z).Transpose();
		m_stStatic.Position = Vector4::Zero;
		m_stStatic.Color = Vector4::One;
		updateConstantBuffer(m_cbStatic, &m_stStatic, sizeof(CStaticBuffer));
		m_context->VSSetConstantBuffers(1, 1, &m_cbStatic);
		m_context->PSSetConstantBuffers(1, 1, &m_cbStatic);

		m_stMisc.AlphaTest = true;
		updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
		m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

		for (__int32 k = 0; k < moveableObj->ObjectMeshes.size(); k++)
		{
			RendererMesh* mesh = moveableObj->ObjectMeshes[k];

			for (__int32 j = 0; j < NUM_BUCKETS; j++)
			{
				RendererBucket* bucket = &mesh->Buckets[j];

				if (bucket->Vertices.size() == 0)
					continue;

				if (j == RENDERER_BUCKET_SOLID_DS || j == RENDERER_BUCKET_TRANSPARENT_DS)
					m_context->RSSetState(m_states->CullNone());
				else
					m_context->RSSetState(m_states->CullCounterClockwise());

				if (j == RENDERER_BUCKET_TRANSPARENT || j == RENDERER_BUCKET_TRANSPARENT_DS)
					m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);
				else
					m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);

				// Draw vertices
				m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
				m_numDrawCalls++;
			}
		}
	}

	// Clear just the Z-buffer so we can start drawing on top of the horizon
	m_context->ClearDepthStencilView(m_currentRenderTarget->DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	return true;
}

bool Renderer11::drawRooms(bool transparent, bool animated)
{
	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	__int32 firstBucket = (transparent ? 2 : 0);
	__int32 lastBucket = (transparent ? 4 : 2);

	if (!animated)
	{
		// Set vertex buffer
		m_context->IASetVertexBuffers(0, 1, &m_roomsVertexBuffer->Buffer, &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout);
		m_context->IASetIndexBuffer(m_roomsIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);
	}

	// Set shaders
	m_context->VSSetShader(m_vsRooms, NULL, 0);
	m_context->PSSetShader(m_psRooms, NULL, 0);

	// Set texture
	m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
	ID3D11SamplerState* sampler = m_states->AnisotropicWrap();
	ID3D11SamplerState* shadowSampler = m_states->PointClamp();
	m_context->PSSetSamplers(0, 1, &sampler);
	m_context->PSSetShaderResources(1, 1, &m_caustics[m_currentCausticsFrame / 2]->ShaderResourceView);
	m_context->PSSetSamplers(1, 1, &shadowSampler);
	m_context->PSSetShaderResources(2, 1, &m_shadowMap->ShaderResourceView);
	 
	// Set camera matrices
	m_stCameraMatrices.View = View.Transpose();
	m_stCameraMatrices.Projection = Projection.Transpose();
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);
	 
	// Set shadow map data
	if (m_shadowLight != NULL)
	{    
		memcpy(&m_stShadowMap.Light, m_shadowLight, sizeof(ShaderLight));
		m_stShadowMap.CastShadows = true;
		//m_stShadowMap.ViewProjectionInverse = ViewProjection.Invert().Transpose();
	}
	else
	{
		m_stShadowMap.CastShadows = false;
	}

	updateConstantBuffer(m_cbShadowMap, &m_stShadowMap, sizeof(CShadowLightBuffer));
	m_context->VSSetConstantBuffers(4, 1, &m_cbShadowMap);
	m_context->PSSetConstantBuffers(4, 1, &m_cbShadowMap);

	if (animated)
		m_primitiveBatch->Begin();

	for (__int32 i = 0; i < m_roomsToDraw.Size(); i++)
	{ 
		RendererRoom* room = m_roomsToDraw[i];
		
		m_stLights.NumLights = room->LightsToDraw.Size();
		for (__int32 j = 0; j < room->LightsToDraw.Size(); j++)
			memcpy(&m_stLights.Lights[j], room->LightsToDraw[j], sizeof(ShaderLight));
		updateConstantBuffer(m_cbLights, &m_stLights, sizeof(CLightBuffer));
		m_context->PSSetConstantBuffers(1, 1, &m_cbLights);

		m_stMisc.Caustics = (room->Room->flags & ENV_FLAG_WATER);
		m_stMisc.AlphaTest = !transparent;
		updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
		m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

		for (__int32 j = firstBucket; j < lastBucket; j++)
		{
			RendererBucket* bucket;
			if (!animated)
				bucket = &room->Buckets[j];
			else
				bucket = &room->AnimatedBuckets[j];

			if (bucket->Vertices.size() == 0)
				continue;

			if (!animated)
				
			if (j == RENDERER_BUCKET_SOLID_DS || j == RENDERER_BUCKET_TRANSPARENT_DS)
				m_context->RSSetState(m_states->CullNone());
			else
				m_context->RSSetState(m_states->CullCounterClockwise());

			if (!animated)
			{
				m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
				m_numDrawCalls++;
			}
			else
			{
				for (__int32 k = 0; k < bucket->Polygons.size(); k++)
				{
					RendererPolygon* poly = &bucket->Polygons[k];

					if (poly->Shape == SHAPE_RECTANGLE)
					{
						m_primitiveBatch->DrawQuad(bucket->Vertices[poly->Indices[0]], bucket->Vertices[poly->Indices[1]],
							bucket->Vertices[poly->Indices[2]], bucket->Vertices[poly->Indices[3]]);
					}
					else
					{
						m_primitiveBatch->DrawTriangle(bucket->Vertices[poly->Indices[0]], bucket->Vertices[poly->Indices[1]],
							bucket->Vertices[poly->Indices[2]]);
					}
				}
			}
		}
	}

	if (animated)
		m_primitiveBatch->End();

	return true;
}

bool Renderer11::drawStatics(bool transparent)
{  
	//return true;
	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;
	   
	__int32 firstBucket = (transparent ? 2 : 0);
	__int32 lastBucket = (transparent ? 4 : 2);

	m_context->IASetVertexBuffers(0, 1, &m_staticsVertexBuffer->Buffer, &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetIndexBuffer(m_staticsIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

	// Set shaders
	m_context->VSSetShader(m_vsStatics, NULL, 0);
	m_context->PSSetShader(m_psStatics, NULL, 0);
	 
	// Set texture
	m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
	ID3D11SamplerState* sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	// Set camera matrices
	m_stCameraMatrices.View = View.Transpose();
	m_stCameraMatrices.Projection = Projection.Transpose();
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	m_stMisc.AlphaTest = !transparent;
	updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
	m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

	for (__int32 i = 0; i < m_staticsToDraw.Size(); i++)
	{
		MESH_INFO* msh = m_staticsToDraw[i]->Mesh;

		if (!(msh->Flags & 1))
			continue;

		RendererRoom* room = m_rooms[m_staticsToDraw[i]->RoomIndex];

		RendererObject* staticObj = m_staticObjects[msh->staticNumber];
		RendererMesh* mesh = staticObj->ObjectMeshes[0];

		m_stStatic.World = (Matrix::CreateRotationY(TR_ANGLE_TO_RAD(msh->yRot)) * Matrix::CreateTranslation(msh->x, msh->y, msh->z)).Transpose();
		m_stStatic.Color = Vector4(((msh->shade >> 10) & 0xFF) / 255.0f, ((msh->shade >> 5) & 0xFF) / 255.0f, ((msh->shade >> 0) & 0xFF) / 255.0f, 1.0f);
		updateConstantBuffer(m_cbStatic, &m_stStatic, sizeof(CStaticBuffer));
		m_context->VSSetConstantBuffers(1, 1, &m_cbStatic);

		for (__int32 j = firstBucket; j < lastBucket; j++)
		{
			if (j == RENDERER_BUCKET_SOLID_DS || j == RENDERER_BUCKET_TRANSPARENT_DS)
				m_context->RSSetState(m_states->CullNone());
			else
				m_context->RSSetState(m_states->CullCounterClockwise());

			RendererBucket* bucket = &mesh->Buckets[j];

			if (bucket->Vertices.size() == 0)
				continue;

			// Draw vertices
			m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
			m_numDrawCalls++;
		}
	}

	return true;
}

bool Renderer11::drawAnimatingItem(RendererItem* item, bool transparent, bool animated)
{
	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	__int32 firstBucket = (transparent ? 2 : 0);
	__int32 lastBucket = (transparent ? 4 : 2);

	RendererRoom* room = m_rooms[item->Item->roomNumber];
	RendererObject* moveableObj = m_moveableObjects[item->Item->objectNumber];

	m_stItem.World = item->World.Transpose();
	m_stItem.Position = Vector4(item->Item->pos.xPos, item->Item->pos.yPos, item->Item->pos.zPos, 1.0f);
	m_stItem.AmbientLight = room->AmbientLight;
	memcpy(m_stItem.BonesMatrices, item->AnimationTransforms, sizeof(Matrix) * 32);
	updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));
	m_context->VSSetConstantBuffers(1, 1, &m_cbItem);

	m_stLights.NumLights = item->Lights.Size();
	for (__int32 j = 0; j < item->Lights.Size(); j++)
		memcpy(&m_stLights.Lights[j], item->Lights[j], sizeof(ShaderLight));
	updateConstantBuffer(m_cbLights, &m_stLights, sizeof(CLightBuffer));
	m_context->PSSetConstantBuffers(2, 1, &m_cbLights);

	m_stMisc.AlphaTest = !transparent;
	updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
	m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

	for (__int32 k = 0; k < moveableObj->ObjectMeshes.size(); k++)
	{
		RendererMesh* mesh = moveableObj->ObjectMeshes[k];

		for (__int32 j = firstBucket; j < lastBucket; j++)
		{
			RendererBucket* bucket = &mesh->Buckets[j];

			if (bucket->Vertices.size() == 0)
				continue;

			if (j == RENDERER_BUCKET_SOLID_DS || j == RENDERER_BUCKET_TRANSPARENT_DS)
				m_context->RSSetState(m_states->CullNone());
			else
				m_context->RSSetState(m_states->CullCounterClockwise());

			// Draw vertices
			m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
			m_numDrawCalls++;
		}
	}

	return true;
}

bool Renderer11::drawWaterfalls()
{
	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	// Draw waterfalls
	m_context->RSSetState(m_states->CullCounterClockwise());
	
	for (__int32 i = 0; i < m_itemsToDraw.Size(); i++)
	{
		RendererItem* item = m_itemsToDraw[i];
		RendererRoom* room = m_rooms[item->Item->roomNumber];
		RendererObject* moveableObj = m_moveableObjects[item->Item->objectNumber];

		__int16 objectNumber = item->Item->objectNumber;
		if (objectNumber >= ID_WATERFALL1 && objectNumber <= ID_WATERFALLSS2)
		{
			RendererRoom* room = m_rooms[item->Item->roomNumber];
			RendererObject* moveableObj = m_moveableObjects[item->Item->objectNumber];

			m_stItem.World = item->World.Transpose();
			m_stItem.Position = Vector4(item->Item->pos.xPos, item->Item->pos.yPos, item->Item->pos.zPos, 1.0f);
			m_stItem.AmbientLight = room->AmbientLight; //Vector4::One * 0.1f; // room->AmbientLight;
			memcpy(m_stItem.BonesMatrices, item->AnimationTransforms, sizeof(Matrix) * 32);
			updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));
			m_context->VSSetConstantBuffers(1, 1, &m_cbItem);

			m_stLights.NumLights = item->Lights.Size();
			for (__int32 j = 0; j < item->Lights.Size(); j++)
				memcpy(&m_stLights.Lights[j], item->Lights[j], sizeof(ShaderLight));
			updateConstantBuffer(m_cbLights, &m_stLights, sizeof(CLightBuffer));
			m_context->PSSetConstantBuffers(2, 1, &m_cbLights);

			m_stMisc.AlphaTest = false;
			updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
			m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

			m_primitiveBatch->Begin();

			for (__int32 k = 0; k < moveableObj->ObjectMeshes.size(); k++)
			{
				RendererMesh* mesh = moveableObj->ObjectMeshes[k];

				for (__int32 b = 0; b < NUM_BUCKETS; b++)
				{
					RendererBucket* bucket = &mesh->Buckets[b];

					if (bucket->Vertices.size() == 0)
						continue;

					for (__int32 p = 0; p < bucket->Polygons.size(); p++)
					{
						RendererPolygon* poly = &bucket->Polygons[p];

						OBJECT_TEXTURE* texture = &ObjectTextures[poly->TextureId];
						__int32 tile = texture->tileAndFlag & 0x7FFF;

						if (poly->Shape == SHAPE_RECTANGLE)
						{
							bucket->Vertices[poly->Indices[0]].UV.y = (texture->vertices[0].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;
							bucket->Vertices[poly->Indices[1]].UV.y = (texture->vertices[1].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;
							bucket->Vertices[poly->Indices[2]].UV.y = (texture->vertices[2].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;
							bucket->Vertices[poly->Indices[3]].UV.y = (texture->vertices[3].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;

							m_primitiveBatch->DrawQuad(bucket->Vertices[poly->Indices[0]],
								bucket->Vertices[poly->Indices[1]],
								bucket->Vertices[poly->Indices[2]],
								bucket->Vertices[poly->Indices[3]]);
						}
						else
						{
							bucket->Vertices[poly->Indices[0]].UV.y = (texture->vertices[0].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;
							bucket->Vertices[poly->Indices[1]].UV.y = (texture->vertices[1].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;
							bucket->Vertices[poly->Indices[2]].UV.y = (texture->vertices[2].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;

							m_primitiveBatch->DrawTriangle(bucket->Vertices[poly->Indices[0]],
								bucket->Vertices[poly->Indices[1]],
								bucket->Vertices[poly->Indices[2]]);
						}
					}
				}
			}

			m_primitiveBatch->End();
		}
		else
		{
			continue;
		}
	}

	return true;
}

bool Renderer11::drawItems(bool transparent, bool animated)
{
	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	__int32 firstBucket = (transparent ? 2 : 0);
	__int32 lastBucket = (transparent ? 4 : 2);

	m_context->IASetVertexBuffers(0, 1, &m_moveablesVertexBuffer->Buffer, &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetIndexBuffer(m_moveablesIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

	for (__int32 i = 0; i < m_itemsToDraw.Size(); i++)
	{
		RendererItem* item = m_itemsToDraw[i];
		RendererRoom* room = m_rooms[item->Item->roomNumber];
		RendererObject* moveableObj = m_moveableObjects[item->Item->objectNumber];

		__int16 objectNumber = item->Item->objectNumber;
		if (moveableObj->DoNotDraw)
		{
			continue;
		}
		else if (objectNumber >= ID_WATERFALL1 && objectNumber <= ID_WATERFALLSS2)
		{
			// We'll draw waterfalls later
			continue;
		}
		else
		{
			drawAnimatingItem(item, transparent, animated);
		}
	}

	return true;
}

bool Renderer11::drawLara(bool transparent, bool shadowMap)
{
	// Don't draw Lara if binoculars or sniper
	if (BinocularRange || SpotcamOverlay || SpotcamDontDrawLara)
		return true;

	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	__int32 firstBucket = (transparent ? 2 : 0);
	__int32 lastBucket = (transparent ? 4 : 2);

	m_context->IASetVertexBuffers(0, 1, &m_moveablesVertexBuffer->Buffer, &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetIndexBuffer(m_moveablesIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

	RendererItem* item = &m_items[Lara.itemNumber];

	// Set shaders
	if (shadowMap)
	{
		m_context->VSSetShader(m_vsShadowMap, NULL, 0);
		m_context->PSSetShader(m_psShadowMap, NULL, 0);
	}
	else
	{
		m_context->VSSetShader(m_vsItems, NULL, 0);
		m_context->PSSetShader(m_psItems, NULL, 0);
	}

	// Set texture
	m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
	ID3D11SamplerState* sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	// Set camera matrices
	m_stCameraMatrices.View = View.Transpose();
	m_stCameraMatrices.Projection = Projection.Transpose();
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	m_stMisc.AlphaTest = !transparent;
	updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
	m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

	RendererObject* laraObj = m_moveableObjects[ID_LARA];
	RendererObject* laraSkin = m_moveableObjects[ID_LARA_SKIN];
	RendererRoom* room = m_rooms[LaraItem->roomNumber];

	m_stItem.World = m_LaraWorldMatrix.Transpose();
	m_stItem.Position = Vector4(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, 1.0f);
	m_stItem.AmbientLight = room->AmbientLight;
	memcpy(m_stItem.BonesMatrices, laraObj->AnimationTransforms.data(), sizeof(Matrix) * 32);
	updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));
	m_context->VSSetConstantBuffers(1, 1, &m_cbItem);
	m_context->PSSetConstantBuffers(1, 1, &m_cbItem);

	if (!shadowMap)
	{
		m_stLights.NumLights = item->Lights.Size();
		for (__int32 j = 0; j < item->Lights.Size(); j++)
			memcpy(&m_stLights.Lights[j], item->Lights[j], sizeof(ShaderLight));
		updateConstantBuffer(m_cbLights, &m_stLights, sizeof(CLightBuffer));
		m_context->PSSetConstantBuffers(2, 1, &m_cbLights);
	}

	for (__int32 k = 0; k < laraSkin->ObjectMeshes.size(); k++)
	{
		RendererMesh* mesh = m_meshPointersToMesh[reinterpret_cast<unsigned int>(Lara.meshPtrs[k])];
		
		for (__int32 j = firstBucket; j < lastBucket; j++)
		{
			RendererBucket* bucket = &mesh->Buckets[j];

			if (bucket->Vertices.size() == 0)
				continue;

			if (j == RENDERER_BUCKET_SOLID_DS || j == RENDERER_BUCKET_TRANSPARENT_DS)
				m_context->RSSetState(m_states->CullNone());
			else
				m_context->RSSetState(m_states->CullCounterClockwise());

			// Draw vertices
			m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
			m_numDrawCalls++;
		}
	}

	if (m_moveableObjects[ID_LARA_SKIN_JOINTS] != NULL)
	{
		RendererObject* laraSkinJoints = m_moveableObjects[ID_LARA_SKIN_JOINTS];

		for (__int32 k = 0; k < laraSkinJoints->ObjectMeshes.size(); k++)
		{
			RendererMesh* mesh = laraSkinJoints->ObjectMeshes[k];

			for (__int32 j = firstBucket; j < lastBucket; j++)
			{
				RendererBucket* bucket = &mesh->Buckets[j];

				if (bucket->Vertices.size() == 0)
					continue;

				// Draw vertices
				m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
				m_numDrawCalls++;
			}
		}
	}

	if (!transparent)
	{
		for (__int32 k = 0; k < laraSkin->ObjectMeshes.size(); k++)
		{
			RendererMesh* mesh = laraSkin->ObjectMeshes[k];

			for (__int32 j = 0; j < NUM_BUCKETS; j++)
			{
				RendererBucket* bucket = &mesh->Buckets[j];

				if (bucket->Vertices.size() == 0)
					continue;

				// Draw vertices
				m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
				m_numDrawCalls++;
			}
		}

		// Hairs are pre-transformed
		Matrix matrices[8] = { Matrix::Identity, Matrix::Identity, Matrix::Identity, Matrix::Identity, 
							   Matrix::Identity, Matrix::Identity, Matrix::Identity, Matrix::Identity };
		memcpy(m_stItem.BonesMatrices, matrices, sizeof(Matrix) * 8);
		m_stItem.World = Matrix::Identity;
		updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));

		if (m_moveableObjects[ID_HAIR] != NULL)
		{
			m_primitiveBatch->Begin();
			m_primitiveBatch->DrawIndexed(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
				(const unsigned __int16*)m_hairIndices.data(), m_numHairIndices,
				m_hairVertices.data(), m_numHairVertices);
			m_primitiveBatch->End();
		}
	}

	return true;
}

bool Renderer11::drawScene(bool dump)
{
	m_timeUpdate = 0;
	m_timeDraw = 0;
	m_timeFrame = 0;
	m_numDrawCalls = 0;
	m_nextLight = 0;
	m_nextSprite = 0;
	m_nextLine3D = 0;
	m_nextLine2D = 0;

	m_currentCausticsFrame++;
	m_currentCausticsFrame %= 32;

	m_strings.clear();

	GameScriptLevel* level = g_GameFlow->GetLevel(CurrentLevel);

	ViewProjection = View * Projection;
	m_stLights.CameraPosition = Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z);

	// Prepare the scene to draw
	auto time1 = chrono::high_resolution_clock::now();

	clearSceneItems();
	collectRooms();
	updateLaraAnimations();
	updateItemsAnimations();
	updateEffects();
	  
	m_items[Lara.itemNumber].Item = LaraItem;
 	collectLightsForItem(LaraItem->roomNumber, &m_items[Lara.itemNumber]);
	     
	// Update animated textures every 2 frames  
	if (GnFrameCounter % 2 == 0)  
		updateAnimatedTextures();
	 
	auto time2 = chrono::high_resolution_clock::now();
	m_timeUpdate = (chrono::duration_cast<ns>(time2 - time1)).count() / 1000000;
	time1 = time2;  

	// Draw shadow map
	if (g_Configuration.EnableShadows)
		drawShadowMap();
	
	// Reset GPU state
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->RSSetState(m_states->CullCounterClockwise());
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	// Bind and clear render target
	m_currentRenderTarget = (dump ? m_dumpScreenRenderTarget : m_renderTarget);

	m_context->ClearRenderTargetView(m_currentRenderTarget->RenderTargetView, Colors::Black);
	m_context->ClearDepthStencilView(m_currentRenderTarget->DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &m_currentRenderTarget->RenderTargetView, m_currentRenderTarget->DepthStencilView);

	m_context->RSSetViewports(1, &m_viewport);

	drawHorizonAndSky();

	// Opaque geometry
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	
	drawRooms(false, false);
	drawRooms(false, true);
	drawStatics(false);
	drawLara(false, false);
	drawItems(false, false);
	drawItems(false, true);
	drawGunFlashes();
	drawGunShells();
	drawDebris(false);
	drawBats();
	drawRats();
	drawSpiders();

	// Transparent geometry
	m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthRead(), 0);

	drawRooms(true, false);
	drawRooms(true, true);
	drawStatics(true);
	drawLara(true, false);
	drawItems(true, false);
	drawItems(true, true);
	drawWaterfalls();
	drawDebris(true);

	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	// Do special effects and weather
	drawFires();
	drawSmokes();
	drawBlood();
	drawSparks();
	drawBubbles();
	drawDrips();
	drawRipples();
	drawUnderwaterDust();
	drawSplahes();
	drawShockwaves();

	if (level->Weather == WEATHER_RAIN)
		doRain();
	else if (level->Weather == WEATHER_SNOW)
		doSnow();

	drawRopes();

	drawSprites();
	drawLines3D();

	time2 = chrono::high_resolution_clock::now();
	m_timeFrame = (chrono::duration_cast<ns>(time2 - time1)).count() / 1000000;
	time1 = time2;

	// Bars
	DrawDashBar();
	UpdateHealtBar(0);
	UpdateAirBar(0);
	DrawAllPickups();

	drawLines2D();

	// Draw binoculars or lasersight
	drawOverlays();

	ROOM_INFO* r = &Rooms[LaraItem->roomNumber];
	m_currentY = 60;

	printDebugMessage("Update time: %d", m_timeUpdate);
	printDebugMessage("Frame time: %d", m_timeFrame);
	printDebugMessage("Draw calls: %d", m_numDrawCalls);
	printDebugMessage("Rooms: %d", m_roomsToDraw.Size());
	printDebugMessage("Items: %d", m_itemsToDraw.Size());
	printDebugMessage("Statics: %d", m_staticsToDraw.Size());
	printDebugMessage("Lights: %d", m_lightsToDraw.Size());
	printDebugMessage("Lara.roomNumber: %d", LaraItem->roomNumber);
	printDebugMessage("Lara.pos: %d %d %d", LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
	printDebugMessage("Room: %d %d %d %d", r->x, r->z, r->x + r->xSize * WALL_SIZE, r->z + r->ySize * WALL_SIZE);

	drawAllStrings();

	m_spriteBatch->Begin();
	RECT rect; rect.top = rect.left = 0; rect.right = rect.bottom = 300;
	m_spriteBatch->Draw(m_shadowMap->ShaderResourceView, rect, Colors::White);
	m_spriteBatch->End();

	if (!dump)
		m_swapChain->Present(0, 0);

	return true;
}

__int32 Renderer11::DumpGameScene()
{
	drawScene(true);
	return 0; 
}

__int32 Renderer11::DrawInventory()
{
	drawInventoryScene();
	drawFinalPass();

	return 0;
}

__int32 Renderer11::SyncRenderer()
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

bool Renderer11::PrintString(__int32 x, __int32 y, char* string, D3DCOLOR color, __int32 flags)
{
	__int32 realX = x;
	__int32 realY = y;
	float factorX = ScreenWidth / 800.0f;
	float factorY = ScreenHeight / 600.0f;

	RECT rect = { 0, 0, 0, 0 };

	// Convert the string to wstring
	__int32 sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, string, strlen(string), NULL, 0);
	std::wstring wstr(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, string, strlen(string), &wstr[0], sizeNeeded);

	// Prepare the structure for the renderer
	RendererStringToDraw str;
	str.String = wstr;
	str.Flags = flags;
	str.X = 0;
	str.Y = 0;
	str.Color = Vector3((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);

	// Measure the string
	Vector2 size = m_gameFont->MeasureString(wstr.c_str());

	if (flags & PRINTSTRING_CENTER)
	{
		__int32 width = size.x;
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

	str.X = rect.left;
	str.Y = rect.top;

	if (flags & PRINTSTRING_BLINK)
	{
		str.Color = Vector3(m_blinkColorValue, m_blinkColorValue, m_blinkColorValue);

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

	m_strings.push_back(str);

	return true;
}

__int32 Renderer11::drawFinalPass()
{
	// Update fade status
	if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_IN && m_fadeFactor > 0.99f)
		m_fadeStatus = RENDERER_FADE_STATUS::NO_FADE;

	if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_OUT && m_fadeFactor <= 0.01f)
		m_fadeStatus = RENDERER_FADE_STATUS::NO_FADE;

	// Reset GPU state
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->RSSetState(m_states->CullCounterClockwise());
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	m_context->ClearRenderTargetView(m_backBufferRTV, Colors::Black);
	m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &m_backBufferRTV, m_depthStencilView);

	drawFullScreenQuad(m_renderTarget->ShaderResourceView, Vector3(m_fadeFactor, m_fadeFactor, m_fadeFactor), m_enableCinematicBars);

	m_swapChain->Present(0, 0);

	// Update fade status
	if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_IN)
		m_fadeFactor += FADE_FACTOR;

	if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_OUT)
		m_fadeFactor -= FADE_FACTOR;

	return 0;
}

bool Renderer11::drawFullScreenImage(ID3D11ShaderResourceView* texture, float fade)
{
	// Reset GPU state
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->RSSetState(m_states->CullCounterClockwise());
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	m_context->ClearRenderTargetView(m_backBufferRTV, Colors::White);
	m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &m_backBufferRTV, m_depthStencilView);
	m_context->RSSetViewports(1, &m_viewport);

	drawFullScreenQuad(texture, Vector3(fade, fade, fade), false);

	m_swapChain->Present(0, 0);

	return true;
}

bool Renderer11::drawAllStrings()
{
	m_spriteBatch->Begin();

	for (__int32 i = 0; i < m_strings.size(); i++)
	{
		RendererStringToDraw* str = &m_strings[i];

		// Draw shadow if needed
		if (str->Flags & PRINTSTRING_OUTLINE)
			m_gameFont->DrawString(m_spriteBatch, str->String.c_str(), Vector2(str->X + 1, str->Y + 1),
				Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	
		// Draw string
		m_gameFont->DrawString(m_spriteBatch, str->String.c_str(), Vector2(str->X, str->Y), 
			Vector4(str->Color.x / 255.0f, str->Color.y / 255.0f, str->Color.z / 255.0f, 1.0f));
	}

	m_spriteBatch->End();

	return true;
}

bool Renderer11::PrepareDataForTheRenderer()
{
	m_moveableObjects = (RendererObject**)malloc(sizeof(RendererObject*) * NUM_OBJECTS);
	ZeroMemory(m_moveableObjects, sizeof(RendererObject*) * NUM_OBJECTS);

	m_spriteSequences = (RendererSpriteSequence**)malloc(sizeof(RendererSpriteSequence*) * NUM_OBJECTS);
	ZeroMemory(m_spriteSequences, sizeof(RendererSpriteSequence*) * NUM_OBJECTS);

	m_staticObjects = (RendererObject**)malloc(sizeof(RendererObject*) * NUM_STATICS);
	ZeroMemory(m_staticObjects, sizeof(RendererObject*) * NUM_STATICS);

	m_rooms = (RendererRoom**)malloc(sizeof(RendererRoom*) * NUM_ROOMS);
	ZeroMemory(m_rooms, sizeof(RendererRoom*) * NUM_ROOMS);

	m_meshes.clear();

	// Step 0: prepare animated textures
	__int16 numSets = *AnimatedTextureRanges;
	__int16* animatedPtr = AnimatedTextureRanges;
	animatedPtr++;
	
	m_animatedTextureSets = (RendererAnimatedTextureSet**)malloc(sizeof(RendererAnimatedTextureSet*) * NUM_ANIMATED_SETS);
	m_numAnimatedTextureSets = numSets;

	for (__int32 i = 0; i < numSets; i++)
	{
		RendererAnimatedTextureSet* set = new RendererAnimatedTextureSet();
		__int16 numTextures = *animatedPtr + 1;
		animatedPtr++;

		set->Textures = (RendererAnimatedTexture**)malloc(sizeof(RendererAnimatedTexture) * numTextures);
		set->NumTextures = numTextures;

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

				newTexture->UV[k] = Vector2(x, y);
			}

			set->Textures[j] = newTexture;
		}

		m_animatedTextureSets[i] = set;
	}

	// Step 1: create the texture atlas
	byte* buffer = (byte*)malloc(TEXTURE_ATLAS_SIZE * TEXTURE_ATLAS_SIZE * 4);
	ZeroMemory(buffer, TEXTURE_ATLAS_SIZE * TEXTURE_ATLAS_SIZE * 4);

	__int32 blockX = 0;
	__int32 blockY = 0;

	if (g_GameFlow->GetLevel(CurrentLevel)->LaraType == LARA_YOUNG)
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

				buffer[pixelIndex + 2] = r;
				buffer[pixelIndex + 1] = g;
				buffer[pixelIndex + 0] = b;
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

	if (m_textureAtlas != NULL)
		delete m_textureAtlas;

	m_textureAtlas = Texture2D::LoadFromByteArray(m_device, TEXTURE_ATLAS_SIZE, TEXTURE_ATLAS_SIZE, &buffer[0]);
	if (m_textureAtlas == NULL)
		return false;
	 
	free(buffer);

	buffer = (byte*)malloc(256 * 256 * 4);
	memcpy(buffer, MiscTextures + 256 * 512 * 4, 256 * 256 * 4);
	
	m_skyTexture = Texture2D::LoadFromByteArray(m_device, 256, 256, &buffer[0]);
	if (m_skyTexture == NULL)
		return false;

	//D3DX11SaveTextureToFileA(m_context, m_skyTexture->Texture, D3DX11_IFF_PNG, "H:\\sky.png");

	free(buffer);

	// Step 2: prepare rooms
	vector<RendererVertex> roomVertices;
	vector<__int32> roomIndices;

	__int32 baseRoomVertex = 0;
	__int32 baseRoomIndex = 0;

	for (__int32 i = 0; i < NumberRooms; i++)
	{
		ROOM_INFO* room = &Rooms[i];

		RendererRoom* r = new RendererRoom();
		
		r->RoomNumber = i;
		r->Room = room;
		r->AmbientLight = Vector4(room->ambient.b / 255.0f, room->ambient.g / 255.0f, room->ambient.r / 255.0f, 1.0f);
		r->LightsToDraw.Reserve(32);

		m_rooms[i] = r;

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
				for (int n = 0; n < layer->NumLayerRectangles; n++)
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
						else
							bucketIndex = RENDERER_BUCKET_SOLID;
					}
					else
					{
						if (texture->attribute == 2)
							bucketIndex = RENDERER_BUCKET_TRANSPARENT_DS;
						else 
							bucketIndex = RENDERER_BUCKET_SOLID_DS;
					}

					if (animatedSetIndex == -1)
					{
						bucket = &r->Buckets[bucketIndex];
					}
					else
					{
						bucket = &r->AnimatedBuckets[bucketIndex];
					}

					// Calculate face normal
					Vector3 p0 = Vector3(vertices[poly->Vertices[0]].Vertex.x,
											vertices[poly->Vertices[0]].Vertex.y,
											vertices[poly->Vertices[0]].Vertex.z);
					Vector3 p1 = Vector3(vertices[poly->Vertices[1]].Vertex.x,
											vertices[poly->Vertices[1]].Vertex.y,
											vertices[poly->Vertices[1]].Vertex.z);
					Vector3 p2 = Vector3(vertices[poly->Vertices[2]].Vertex.x,
											vertices[poly->Vertices[2]].Vertex.y,
											vertices[poly->Vertices[2]].Vertex.z);
					Vector3 e1 = p1 - p0;
					Vector3 e2 = p1 - p2;
					Vector3 normal = e1.Cross(e2);
					normal.Normalize();

					__int32 baseVertices = bucket->NumVertices;
					for (__int32 v = 0; v < 4; v++)
					{
						RendererVertex vertex; 

						vertex.Position.x = room->x + vertices[poly->Vertices[v]].Vertex.x;
						vertex.Position.y = room->y + vertices[poly->Vertices[v]].Vertex.y;
						vertex.Position.z = room->z + vertices[poly->Vertices[v]].Vertex.z;

						vertex.Normal.x = vertices[poly->Vertices[v]].Normal.x;
						vertex.Normal.y = vertices[poly->Vertices[v]].Normal.y;
						vertex.Normal.z = vertices[poly->Vertices[v]].Normal.z;

						vertex.UV.x = (texture->vertices[v].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
						vertex.UV.y = (texture->vertices[v].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;

						vertex.Color.x = ((vertices[poly->Vertices[v]].Colour >> 16) & 0xFF) / 255.0f;
						vertex.Color.y = ((vertices[poly->Vertices[v]].Colour >> 8) & 0xFF) / 255.0f;
						vertex.Color.z = ((vertices[poly->Vertices[v]].Colour >> 0) & 0xFF) / 255.0f;
						vertex.Color.w = 1.0f;

						vertex.Bone = 0;

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
				for (int n = 0; n < layer->NumLayerTriangles; n++)
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
						else
							bucketIndex = RENDERER_BUCKET_SOLID;
					}
					else
					{
						if (texture->attribute == 2)
							bucketIndex = RENDERER_BUCKET_TRANSPARENT_DS;
						else
							bucketIndex = RENDERER_BUCKET_SOLID_DS;
					}

					if (animatedSetIndex == -1)
					{
						bucket = &r->Buckets[bucketIndex];
					}
					else
					{
						bucket = &r->AnimatedBuckets[bucketIndex];
					}

					// Calculate face normal
					Vector3 p0 = Vector3(vertices[poly->Vertices[0]].Vertex.x,
						vertices[poly->Vertices[0]].Vertex.y,
						vertices[poly->Vertices[0]].Vertex.z);
					Vector3 p1 = Vector3(vertices[poly->Vertices[1]].Vertex.x,
						vertices[poly->Vertices[1]].Vertex.y,
						vertices[poly->Vertices[1]].Vertex.z);
					Vector3 p2 = Vector3(vertices[poly->Vertices[2]].Vertex.x,
						vertices[poly->Vertices[2]].Vertex.y,
						vertices[poly->Vertices[2]].Vertex.z);
					Vector3 e1 = p1 - p0;
					Vector3 e2 = p1 - p2;
					Vector3 normal = e1.Cross(e2);
					normal.Normalize();

					__int32 baseVertices = bucket->NumVertices;
					for (__int32 v = 0; v < 3; v++)
					{
						RendererVertex vertex;

						vertex.Position.x = room->x + vertices[poly->Vertices[v]].Vertex.x;
						vertex.Position.y = room->y + vertices[poly->Vertices[v]].Vertex.y;
						vertex.Position.z = room->z + vertices[poly->Vertices[v]].Vertex.z;

						vertex.Normal.x = vertices[poly->Vertices[v]].Normal.x;
						vertex.Normal.y = vertices[poly->Vertices[v]].Normal.y;
						vertex.Normal.z = vertices[poly->Vertices[v]].Normal.z;

						vertex.UV.x = (texture->vertices[v].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
						vertex.UV.y = (texture->vertices[v].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;

						vertex.Color.x = ((vertices[poly->Vertices[v]].Colour >> 16) & 0xFF) / 255.0f;
						vertex.Color.y = ((vertices[poly->Vertices[v]].Colour >> 8) & 0xFF) / 255.0f;
						vertex.Color.z = ((vertices[poly->Vertices[v]].Colour >> 0) & 0xFF) / 255.0f;
						vertex.Color.w = 1.0f;

						vertex.Bone = 0;

						bucket->NumVertices++;
						bucket->Vertices.push_back(vertex);
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
				RendererLight light;

				if (oldLight->LightType == LIGHT_TYPES::LIGHT_TYPE_SUN)
				{
					light.Color = Vector3(oldLight->r, oldLight->g, oldLight->b);
					light.Direction = Vector4(oldLight->dx, oldLight->dy, oldLight->dz, 1.0f);
					light.Type = LIGHT_TYPES::LIGHT_TYPE_SUN;
					light.Intensity = 1.0f;

					r->Lights.push_back(light);
				}
				else if (oldLight->LightType == LIGHT_TYPE_POINT)
				{
					light.Position = Vector3(oldLight->x, oldLight->y, oldLight->z);
					light.Color = Vector3(oldLight->r, oldLight->g, oldLight->b);
					light.Direction = Vector4(oldLight->dx, oldLight->dy, oldLight->dz, 1.0f);
					light.Intensity = 1.0f;
					light.In = oldLight->In;
					light.Out = oldLight->Out;
					light.Type = LIGHT_TYPE_POINT;

					r->Lights.push_back(light);
				} 
				else if (oldLight->LightType == LIGHT_TYPE_SHADOW)
				{
					light.Position = Vector3(oldLight->x, oldLight->y, oldLight->z);
					light.Color = Vector3(oldLight->r, oldLight->g, oldLight->b);
					light.In = oldLight->In;
					light.Out = oldLight->Out;
					light.Type = LIGHT_TYPE_SHADOW;
					light.Intensity = 1.0f;

					r->Lights.push_back(light);
				}
				else if (oldLight->LightType == LIGHT_TYPE_SPOT)
				{
					light.Position = Vector3(oldLight->x, oldLight->y, oldLight->z);
					light.Color = Vector3(oldLight->r, oldLight->g, oldLight->b);
					light.Direction = Vector4(oldLight->dx, oldLight->dy, oldLight->dz, 1.0f);
					light.Intensity = 1.0f;
					light.In = oldLight->In;
					light.Out = oldLight->Out;    
					light.Range = oldLight->Range;
					light.Type = LIGHT_TYPE_SPOT;

					r->Lights.push_back(light);
				} 

				oldLight++;
			}
		}

		MESH_INFO* mesh = room->mesh;
		for (__int32 j = 0; j < room->numMeshes; j++)
		{
			RendererStatic obj;
			obj.Mesh = mesh;
			obj.RoomIndex = i;
			r->Statics.push_back(obj);
		}

		// Merge vertices and indices in a single list
		for (__int32 j = 0; j < NUM_BUCKETS; j++)
		{
			RendererBucket* bucket = &r->Buckets[j];
			
			bucket->StartVertex = baseRoomVertex;
			bucket->StartIndex = baseRoomIndex;

			for (__int32 k = 0; k < bucket->Vertices.size(); k++)
				roomVertices.push_back(bucket->Vertices[k]);

			for (__int32 k = 0; k < bucket->Indices.size(); k++)
				roomIndices.push_back(baseRoomVertex + bucket->Indices[k]);
			 
			baseRoomVertex += bucket->Vertices.size();
			baseRoomIndex += bucket->Indices.size();
		} 
	}

	// Create a single vertex buffer and a single index buffer for all rooms
	// NOTICE: in theory, a 1,000,000 vertices scene should have a VB of 52 MB and an IB of 4 MB
	m_roomsVertexBuffer = VertexBuffer::Create(m_device, roomVertices.size(), roomVertices.data());
	m_roomsIndexBuffer = IndexBuffer::Create(m_device, roomIndices.size(), roomIndices.data());

	m_numHairVertices = 0;
	m_numHairIndices = 0;

	vector<RendererVertex> moveablesVertices;
	vector<__int32> moveablesIndices;
	__int32 baseMoveablesVertex = 0;
	__int32 baseMoveablesIndex = 0;

	// Step 3: prepare moveables
	for (__int32 i = 0; i < MoveablesIds.size(); i++)
	{
		__int32 objNum = MoveablesIds[i];
		OBJECT_INFO* obj = &Objects[objNum];

		if (obj->nmeshes > 0)
		{
			RendererObject* moveable = new RendererObject();
			moveable->Id = MoveablesIds[i];

			// Assign the draw routine
			if (objNum == ID_FLAME || objNum == ID_FLAME_EMITTER || objNum == ID_FLAME_EMITTER2 || objNum == ID_FLAME_EMITTER3 ||
				objNum == ID_TRIGGER_TRIGGERER || objNum == ID_TIGHT_ROPE || objNum == ID_AI_AMBUSH ||
				objNum == ID_AI_FOLLOW || objNum == ID_AI_GUARD || objNum == ID_AI_MODIFY ||
				objNum == ID_AI_PATROL1 || objNum == ID_AI_PATROL2 || objNum == ID_AI_X1 ||
				objNum == ID_AI_X2 || objNum == ID_DART_EMITTER || objNum == ID_HOMING_DART_EMITTER ||
				objNum == ID_ROPE || objNum == ID_KILL_ALL_TRIGGERS || objNum == ID_EARTHQUAKE ||
				objNum == ID_CAMERA_TARGET || objNum == ID_WATERFALLMIST || objNum == ID_SMOKE_EMITTER_BLACK ||
				objNum == ID_SMOKE_EMITTER_WHITE)
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
				// We need to override the bone index because the engine will take mesh 0 while drawing pistols anim,
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

			__int32* bone = &Bones[obj->boneIndex];

			stack<RendererBone*> stack;

			for (int j = 0; j < obj->nmeshes; j++)
			{
				moveable->LinearizedBones.push_back(new RendererBone(j));
				moveable->AnimationTransforms.push_back(Matrix::Identity);
				moveable->BindPoseTransforms.push_back(Matrix::Identity);
			}

			RendererBone* currentBone = moveable->LinearizedBones[0];
			RendererBone* stackBone = moveable->LinearizedBones[0];

			for (int mi = 0; mi < obj->nmeshes - 1; mi++)
			{
				int j = mi + 1;

				__int32 opcode = *(bone++);
				int linkX = *(bone++);
				int linkY = *(bone++);
				int linkZ = *(bone++);

				byte flags = opcode & 0x1C;

				moveable->LinearizedBones[j]->ExtraRotationFlags = flags;

				switch (opcode & 0x03)
				{
				case 0:
					moveable->LinearizedBones[j]->Parent = currentBone;
					moveable->LinearizedBones[j]->Translation = Vector3(linkX, linkY, linkZ);
					currentBone->Children.push_back(moveable->LinearizedBones[j]);
					currentBone = moveable->LinearizedBones[j];

					break;
				case 1:
					if (stack.empty())
						continue;
					currentBone = stack.top();
					stack.pop();

					moveable->LinearizedBones[j]->Parent = currentBone;
					moveable->LinearizedBones[j]->Translation = Vector3(linkX, linkY, linkZ);
					currentBone->Children.push_back(moveable->LinearizedBones[j]);
					currentBone = moveable->LinearizedBones[j];

					break;
				case 2:
					stack.push(currentBone);

					moveable->LinearizedBones[j]->Translation = Vector3(linkX, linkY, linkZ);
					moveable->LinearizedBones[j]->Parent = currentBone;
					currentBone->Children.push_back(moveable->LinearizedBones[j]);
					currentBone = moveable->LinearizedBones[j];

					break;
				case 3:
					if (stack.empty())
						continue;
					RendererBone* theBone = stack.top();
					stack.pop();

					moveable->LinearizedBones[j]->Translation = Vector3(linkX, linkY, linkZ);
					moveable->LinearizedBones[j]->Parent = theBone;
					theBone->Children.push_back(moveable->LinearizedBones[j]);
					currentBone = moveable->LinearizedBones[j];
					stack.push(theBone);

					break;
				}
			}

			for (int n = 0; n < obj->nmeshes; n++)
				moveable->LinearizedBones[n]->Transform = Matrix::CreateTranslation(
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
						RendererBucket* jointBucket = &jointMesh->Buckets[b1];
						for (__int32 v1 = 0; v1 < jointBucket->Vertices.size(); v1++)
						{
							RendererVertex* jointVertex = &jointBucket->Vertices[v1];
							if (jointVertex->Bone != j)
							{
								RendererMesh* skinMesh = objSkin->ObjectMeshes[jointVertex->Bone];
								RendererBone* skinBone = objSkin->LinearizedBones[jointVertex->Bone];

								for (__int32 b2 = 0; b2 < NUM_BUCKETS; b2++)
								{
									RendererBucket* skinBucket = &skinMesh->Buckets[b2];
									for (__int32 v2 = 0; v2 < skinBucket->Vertices.size(); v2++)
									{
										RendererVertex* skinVertex = &skinBucket->Vertices[v2];

										__int32 x1 = jointBucket->Vertices[v1].Position.x + jointBone->GlobalTranslation.x;
										__int32 y1 = jointBucket->Vertices[v1].Position.y + jointBone->GlobalTranslation.y;
										__int32 z1 = jointBucket->Vertices[v1].Position.z + jointBone->GlobalTranslation.z;

										__int32 x2 = skinBucket->Vertices[v2].Position.x + skinBone->GlobalTranslation.x;
										__int32 y2 = skinBucket->Vertices[v2].Position.y + skinBone->GlobalTranslation.y;
										__int32 z2 = skinBucket->Vertices[v2].Position.z + skinBone->GlobalTranslation.z;

										if (abs(x1 - x2) < 2 && abs(y1 - y2) < 2 && abs(z1 - z2) < 2)
										{
											jointVertex->Position.x = skinVertex->Position.x;
											jointVertex->Position.y = skinVertex->Position.y;
											jointVertex->Position.z = skinVertex->Position.z;
										}
									}
								}
							}
						}
					}
				}
			}

			if (MoveablesIds[i] == ID_HAIR)
			{
				for (__int32 j = 0; j < moveable->ObjectMeshes.size(); j++)
				{
					RendererMesh* mesh = moveable->ObjectMeshes[j];
					for (__int32 n = 0; n < NUM_BUCKETS; n++)
					{
						m_numHairVertices += mesh->Buckets[n].NumVertices;
						m_numHairIndices += mesh->Buckets[n].NumIndices;
					}
				}

				m_hairVertices.clear();
				m_hairIndices.clear();

				RendererVertex vertex;
				for (__int32 m = 0; m < m_numHairVertices * 2; m++)
					m_hairVertices.push_back(vertex);
				for (__int32 m = 0; m < m_numHairIndices * 2; m++)
					m_hairIndices.push_back(0);
			}

			m_moveableObjects[MoveablesIds[i]] = moveable;

			// Merge vertices and indices in a single list
			for (__int32 m = 0; m < moveable->ObjectMeshes.size(); m++)
			{
				RendererMesh* msh = moveable->ObjectMeshes[m];

				for (__int32 j = 0; j < NUM_BUCKETS; j++)
				{
					RendererBucket* bucket = &msh->Buckets[j];

					bucket->StartVertex = baseMoveablesVertex;
					bucket->StartIndex = baseMoveablesIndex;

					for (__int32 k = 0; k < bucket->Vertices.size(); k++)
						moveablesVertices.push_back(bucket->Vertices[k]);

					for (__int32 k = 0; k < bucket->Indices.size(); k++)
						moveablesIndices.push_back(baseMoveablesVertex + bucket->Indices[k]);

					baseMoveablesVertex += bucket->Vertices.size();
					baseMoveablesIndex += bucket->Indices.size();
				}
			}
		}
	}

	// Create a single vertex buffer and a single index buffer for all moveables
	m_moveablesVertexBuffer = VertexBuffer::Create(m_device, moveablesVertices.size(), moveablesVertices.data());
	m_moveablesIndexBuffer = IndexBuffer::Create(m_device, moveablesIndices.size(), moveablesIndices.data());

	// Step 4: prepare static meshes
	vector<RendererVertex> staticsVertices;
	vector<__int32> staticsIndices;
	__int32 baseStaticsVertex = 0;
	__int32 baseStaticsIndex = 0;

	for (__int32 i = 0; i < StaticObjectsIds.size(); i++)
	{
		STATIC_INFO* obj = &StaticObjects[StaticObjectsIds[i]];
		RendererObject* staticObject = new RendererObject();
		staticObject->Id = StaticObjectsIds[i];

		__int16* meshPtr = &RawMeshData[RawMeshPointers[obj->meshNumber / 2] / 2];
		RendererMesh* mesh = getRendererMeshFromTrMesh(staticObject, meshPtr, Meshes[obj->meshNumber], 0, false, false);

		staticObject->ObjectMeshes.push_back(mesh);

		m_staticObjects[StaticObjectsIds[i]] = staticObject;

		// Merge vertices and indices in a single list
		RendererMesh* msh = staticObject->ObjectMeshes[0];

		for (__int32 j = 0; j < NUM_BUCKETS; j++)
		{
			RendererBucket* bucket = &msh->Buckets[j];

			bucket->StartVertex = baseStaticsVertex;
			bucket->StartIndex = baseStaticsIndex;

			for (__int32 k = 0; k < bucket->Vertices.size(); k++)
				staticsVertices.push_back(bucket->Vertices[k]);

			for (__int32 k = 0; k < bucket->Indices.size(); k++)
				staticsIndices.push_back(baseStaticsVertex + bucket->Indices[k]);

			baseStaticsVertex += bucket->Vertices.size();
			baseStaticsIndex += bucket->Indices.size();
		}
	}

	// Create a single vertex buffer and a single index buffer for all statics
	m_staticsVertexBuffer = VertexBuffer::Create(m_device, staticsVertices.size(), staticsVertices.data());
	m_staticsIndexBuffer = IndexBuffer::Create(m_device, staticsIndices.size(), staticsIndices.data());

	// Step 5: prepare sprites
	m_sprites = (RendererSprite**)malloc(sizeof(RendererSprite*) * g_NumSprites);
	ZeroMemory(m_sprites, sizeof(RendererSprite*) * g_NumSprites);

	for (__int32 i = 0; i < g_NumSprites; i++)
	{
		SPRITE* oldSprite = &Sprites[i];

		RendererSprite* sprite = new RendererSprite();

		sprite->Width = (oldSprite->right - oldSprite->left)*256.0f;
		sprite->Height = (oldSprite->bottom - oldSprite->top)*256.0f;

		float left = (oldSprite->left * 256.0f + GET_ATLAS_PAGE_X(oldSprite->tile - 1));
		float top = (oldSprite->top * 256.0f + GET_ATLAS_PAGE_Y(oldSprite->tile - 1));
		float right = (oldSprite->right * 256.0f + GET_ATLAS_PAGE_X(oldSprite->tile - 1));
		float bottom = (oldSprite->bottom * 256.0f + GET_ATLAS_PAGE_Y(oldSprite->tile - 1));

		sprite->UV[0] = Vector2(left / (float)TEXTURE_ATLAS_SIZE, top / (float)TEXTURE_ATLAS_SIZE);
		sprite->UV[1] = Vector2(right / (float)TEXTURE_ATLAS_SIZE, top / (float)TEXTURE_ATLAS_SIZE);
		sprite->UV[2] = Vector2(right / (float)TEXTURE_ATLAS_SIZE, bottom / (float)TEXTURE_ATLAS_SIZE);
		sprite->UV[3] = Vector2(left / (float)TEXTURE_ATLAS_SIZE, bottom / (float)TEXTURE_ATLAS_SIZE);

		m_sprites[i] = sprite;
	}

	for (__int32 i = 0; i < MoveablesIds.size(); i++)
	{
		OBJECT_INFO* obj = &Objects[MoveablesIds[i]];

		if (obj->nmeshes < 0)
		{
			__int16 numSprites = abs(obj->nmeshes);
			__int16 baseSprite = obj->meshIndex;

			RendererSpriteSequence* sequence = new RendererSpriteSequence(MoveablesIds[i], numSprites);

			for (__int32 j = baseSprite; j < baseSprite + numSprites; j++)
			{
				sequence->SpritesList[j - baseSprite] = m_sprites[j];
			}

			m_spriteSequences[MoveablesIds[i]] = sequence;
		}
	}

	for (__int32 i = 0; i < 6; i++)
	{ 
		if (Objects[ID_WATERFALL1 + i].loaded)
		{
			// Get the first textured bucket
			RendererBucket* bucket = NULL;
			for (__int32 j = 0; j < NUM_BUCKETS; j++)
				if (m_moveableObjects[ID_WATERFALL1 + i]->ObjectMeshes[0]->Buckets[j].Polygons.size() > 0)
					bucket = &m_moveableObjects[ID_WATERFALL1 + i]->ObjectMeshes[0]->Buckets[j];
			 
			if (bucket == NULL)
				continue;

			OBJECT_TEXTURE* texture = &ObjectTextures[bucket->Polygons[0].TextureId];
			WaterfallTextures[i] = texture;
			WaterfallY[i] = texture->vertices[0].y;
		}
	}

	return true;
}

ID3D11VertexShader* Renderer11::compileVertexShader(char* fileName, char* function, char* model, ID3D10Blob** bytecode)
{
	HRESULT res;

	*bytecode = NULL;
	ID3DBlob* errors = NULL;

	printf("Compiling vertex shader: %s\n", fileName);

	res = D3DX11CompileFromFileA(fileName, NULL, NULL, function, model, D3D10_SHADER_OPTIMIZATION_LEVEL3, 0, NULL, bytecode, &errors, NULL);
	if (FAILED(res))
	{
		printf("Compilation failed: %s\n", errors->GetBufferPointer());
		return NULL;
	}

	ID3D11VertexShader* shader = NULL;
	res = m_device->CreateVertexShader((*bytecode)->GetBufferPointer(), (*bytecode)->GetBufferSize(), NULL, &shader);
	if (FAILED(res))
		return NULL;

	return shader;
}

ID3D11PixelShader* Renderer11::compilePixelShader(char* fileName, char* function, char* model, ID3D10Blob** bytecode)
{
	HRESULT res;

	*bytecode = NULL;
	ID3DBlob* errors = NULL;

	printf("Compiling pixel shader: %s\n", fileName);

	res = D3DX11CompileFromFileA(fileName, NULL, NULL, function, model, D3D10_SHADER_OPTIMIZATION_LEVEL3, 0, NULL, bytecode, &errors, NULL);
	if (FAILED(res))
	{
		printf("Compilation failed: %s\n", errors->GetBufferPointer());
		return NULL;
	}

	ID3D11PixelShader* shader = NULL;
	res = m_device->CreatePixelShader((*bytecode)->GetBufferPointer(), (*bytecode)->GetBufferSize(), NULL, &shader);
	if (FAILED(res))
		return NULL;

	return shader;
}

ID3D11GeometryShader* Renderer11::compileGeometryShader(char* fileName)
{
	HRESULT res;

	ID3DBlob* bytecode = NULL;
	ID3DBlob* errors = NULL;

	res = D3DX11CompileFromFileA(fileName, NULL, NULL, NULL, "gs_4_0", D3D10_SHADER_OPTIMIZATION_LEVEL3, 0, NULL, &bytecode, &errors, NULL);
	if (FAILED(res))
		return NULL;

	ID3D11GeometryShader* shader = NULL;
	res = m_device->CreateGeometryShader(bytecode->GetBufferPointer(), bytecode->GetBufferSize(), NULL, &shader);
	if (FAILED(res))
		return NULL;

	return shader;
}

ID3D11ComputeShader* Renderer11::compileComputeShader(char* fileName)
{
	HRESULT res;

	ID3DBlob* bytecode = NULL;
	ID3DBlob* errors = NULL;

	res = D3DX11CompileFromFileA(fileName, NULL, NULL, NULL, "gs_4_0", D3D10_SHADER_OPTIMIZATION_LEVEL3, 0, NULL, &bytecode, &errors, NULL);
	if (FAILED(res))
		return NULL;

	ID3D11ComputeShader* shader = NULL;
	res = m_device->CreateComputeShader(bytecode->GetBufferPointer(), bytecode->GetBufferSize(), NULL, &shader);
	if (FAILED(res))
		return NULL;

	return shader;
}

void Renderer11::DrawDashBar()
{
	if (DashTimer < 120)
		drawBar(630, 32, 150, 12, 100 * (unsigned __int16)DashTimer / 120, 0xA0A000, 0xA000);
}

void Renderer11::DrawHealthBar(__int32 percentual)
{
	__int32 color2 = 0xA00000;
	if (Lara.poisoned || Lara.gassed)
		color2 = 0xA0A000;
	drawBar(20, 32, 150, 12, percentual, 0xA00000, color2);
}

void Renderer11::DrawAirBar(__int32 percentual)
{
	drawBar(20, 10, 150, 12, percentual, 0x0000A0, 0x0050A0);
}

void Renderer11::ClearDynamicLights()
{
	m_dynamicLights.Clear();
}

void Renderer11::AddDynamicLight(__int32 x, __int32 y, __int32 z, __int16 falloff, byte r, byte g, byte b)
{
	if (m_nextLight >= MAX_LIGHTS)
		return;

	RendererLight* dynamicLight = &m_lights[m_nextLight++];

	dynamicLight->Position = Vector3(x, y, z);
	dynamicLight->Color = Vector3(r / 255.0f, g / 255.0f, b / 255.0f);
	dynamicLight->Out = falloff * 256.0f;
	dynamicLight->Type = LIGHT_TYPES::LIGHT_TYPE_POINT;
	dynamicLight->Dynamic = 1;
	dynamicLight->Intensity = 2.0f;

	m_dynamicLights.Add(dynamicLight);
	NumDynamics++;
}

void Renderer11::EnableCinematicBars(bool value)
{
	m_enableCinematicBars = value;
}

void Renderer11::FadeIn()
{
	m_fadeStatus = RENDERER_FADE_STATUS::FADE_IN;
	m_fadeFactor = 0.0f;
}

void Renderer11::FadeOut()
{
	m_fadeStatus = RENDERER_FADE_STATUS::FADE_OUT;
	m_fadeFactor = 1.0f;
}

void Renderer11::DrawLoadingScreen(char* fileName)
{
	return; 

	Texture2D* texture = Texture2D::LoadFromFile(m_device, fileName);
	if (texture == NULL)
		return;

	m_fadeStatus = RENDERER_FADE_STATUS::FADE_IN;
	m_fadeFactor = 0.0f;

	while (true)
	{
		if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_IN && m_fadeFactor < 1.0f)
			m_fadeFactor += FADE_FACTOR;

		if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_OUT && m_fadeFactor > 0.0f)
			m_fadeFactor -= FADE_FACTOR;

		// Set basic render states
		m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
		m_context->RSSetState(m_states->CullCounterClockwise());
		m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);

		// Clear screen
		m_context->ClearRenderTargetView(m_backBufferRTV, Colors::Black);
		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// Bind the back buffer
		m_context->OMSetRenderTargets(1, &m_backBufferRTV, m_depthStencilView);
		m_context->RSSetViewports(1, &m_viewport);

		// Draw the full screen background
		drawFullScreenQuad(texture->ShaderResourceView, Vector3(m_fadeFactor, m_fadeFactor, m_fadeFactor), false);
		m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		m_swapChain->Present(0, 0);

		if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_IN && m_fadeFactor >= 1.0f)
		{
			m_fadeStatus = RENDERER_FADE_STATUS::NO_FADE;
			m_fadeFactor = 1.0f;
		}

		if (m_fadeStatus == RENDERER_FADE_STATUS::NO_FADE && m_progress == 100)
		{
			m_fadeStatus = RENDERER_FADE_STATUS::FADE_OUT;
			m_fadeFactor = 1.0f;
		}

		if (m_fadeStatus == RENDERER_FADE_STATUS::FADE_OUT && m_fadeFactor <= 0.0f)
		{
			break;
		}
	}

	delete texture;
}

void Renderer11::UpdateProgress(float value)
{
	m_progress = value;
}

bool Renderer11::IsFading()
{
	return (m_fadeStatus != FADEMODE_NONE);
}

void Renderer11::GetLaraBonePosition(Vector3* pos, __int32 bone)
{

}

bool Renderer11::ToggleFullScreen()
{
	return true;
}

bool Renderer11::IsFullsScreen()
{
	return (!Windowed);
}

bool Renderer11::ChangeScreenResolution(__int32 width, __int32 height, __int32 frequency, bool windowed)
{
	HRESULT res;

	/*if (windowed && !Windowed)
	{
		res = m_swapChain->SetFullscreenState(false, NULL);
		if (FAILED(res))
			return false;
	}
	else if (!windowed && Windowed)
	{
		res = m_swapChain->SetFullscreenState(true, NULL);
		if(FAILED(res))
			return false;
	}

	IDXGIOutput* output;
	res = m_swapChain->GetContainingOutput(&output);
	if(FAILED(res))
		return false;

	DXGI_SWAP_CHAIN_DESC scd;
	res = m_swapChain->GetDesc(&scd);
	if (FAILED(res))
		return false;

	UINT numModes = 1024;
	DXGI_MODE_DESC modes[1024];
	res = output->GetDisplayModeList(scd.BufferDesc.Format, 0, &numModes, modes);
	if (FAILED(res))
		return false;

	DXGI_MODE_DESC* mode = &modes[0];
	for (__int32 i = 0; i < numModes; i++)
	{
		mode = &modes[i];
		if (mode->Width == width && mode->Height == height)
			break;
	}


	ID3D11RenderTargetView* nullViews[] = { nullptr };
	m_context->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
	m_backBufferRTV->Release(); // Microsoft::WRL::ComPtr here does a Release();
	m_depthStencilView->Release();
	m_context->Flush();

	res = m_swapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);




	res = m_swapChain->ResizeTarget(mode);
	if (FAILED(res))
		return false;

	RECT rect;
	GetClientRect(WindowsHandle, &rect);
	UINT w = static_cast<UINT>(rect.right);
	UINT h = static_cast<UINT>(rect.bottom);

	m_context->ClearState();
	m_backBufferRTV->Release();
	m_depthStencilView->Release();
	m_depthStencilTexture->Release();

	res = m_swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	if (FAILED(res))
		return false;

	// Recreate render target
	res = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&m_backBufferTexture);
	if (FAILED(res))
		return false;

	m_device->CreateRenderTargetView(m_backBufferTexture, NULL, &m_backBufferRTV);

	D3D11_TEXTURE2D_DESC backBufferDesc;
	m_backBufferTexture->GetDesc(&backBufferDesc);
	m_backBufferTexture->Release();

	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	m_depthStencilTexture = NULL;
	res = m_device->CreateTexture2D(&depthStencilDesc, NULL, &m_depthStencilTexture);
	if (FAILED(res))
		return false;

	m_depthStencilView = NULL;
	res = m_device->CreateDepthStencilView(m_depthStencilTexture, NULL, &m_depthStencilView);
	if (FAILED(res))
		return false;

	DX11_DELETE(m_renderTarget);
	DX11_DELETE(m_dumpScreenRenderTarget);
	DX11_DELETE(m_shadowMap);

	m_renderTarget = RenderTarget2D::Create(m_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_dumpScreenRenderTarget = RenderTarget2D::Create(m_device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_shadowMap = RenderTarget2D::Create(m_device, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, DXGI_FORMAT_R32_FLOAT);

	DX11_DELETE(m_gameFont);
	DX11_DELETE(m_spriteBatch);
	DX11_DELETE(m_primitiveBatch);

	m_spriteBatch = new SpriteBatch(m_context);
	m_gameFont = new SpriteFont(m_device, L"Font.spritefont");
	m_primitiveBatch = new PrimitiveBatch<RendererVertex>(m_context);

	ScreenWidth = width;
	ScreenHeight = height;
	Windowed = windowed;*/

	ID3D11RenderTargetView* nullViews[] = { nullptr };
	m_context->OMSetRenderTargets(0, nullViews, NULL);

	DX11_DELETE(m_renderTarget);
	DX11_DELETE(m_dumpScreenRenderTarget);
	DX11_DELETE(m_shadowMap);
	DX11_DELETE(m_gameFont);
	DX11_DELETE(m_spriteBatch);
	DX11_DELETE(m_primitiveBatch);

	m_backBufferTexture->Release();
	m_backBufferRTV->Release();
	m_depthStencilView->Release();
	m_depthStencilTexture->Release();
	m_context->Flush();
	m_context->ClearState();

	//res = m_swapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	/*res = m_swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	if (FAILED(res))
		return false;*/

	IDXGIOutput* output;
	res = m_swapChain->GetContainingOutput(&output);
	if (FAILED(res))
		return false;

	DXGI_SWAP_CHAIN_DESC scd;
	res = m_swapChain->GetDesc(&scd);
	if (FAILED(res))
		return false;

	UINT numModes = 1024;
	DXGI_MODE_DESC modes[1024];
	res = output->GetDisplayModeList(scd.BufferDesc.Format, 0, &numModes, modes);
	if (FAILED(res))
		return false;

	DXGI_MODE_DESC* mode = &modes[0];
	for (__int32 i = 0; i < numModes; i++)
	{
		mode = &modes[i];
		if (mode->Width == width && mode->Height == height)
			break;
	}

	res = m_swapChain->ResizeTarget(mode);
	if (FAILED(res))
		return false;

	if (!initialiseScreen(width, height, frequency, windowed, WindowsHandle, true))
		return false;

	ScreenWidth = width;
	ScreenHeight = height;
	Windowed = windowed;

	return true;
}

ID3D11Buffer* Renderer11::createConstantBuffer(__int32 size)
{
	ID3D11Buffer* buffer;

	D3D11_BUFFER_DESC desc;	
	ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));
		
	desc.ByteWidth = ceil(size / 16) * 16; // Constant buffer must have a size multiple of 16 bytes
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT res = m_device->CreateBuffer(&desc, NULL, &buffer);
	if (FAILED(res))
		return NULL;

	return buffer;
}

__int32 Renderer11::getAnimatedTextureInfo(__int16 textureId)
{
	for (__int32 i = 0; i < m_numAnimatedTextureSets; i++)
	{
		RendererAnimatedTextureSet* set = m_animatedTextureSets[i];
		for (__int32 j = 0; j < set->NumTextures; j++)
		{
			if (set->Textures[j]->Id == textureId)
				return i;
		}
	}

	return -1;
}

void Renderer11::initialiseHairRemaps()
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
}

void Renderer11::getVisibleRooms(int from, int to, Vector4* viewPort, bool water, int count)
{
	// Avoid allocations, 1024 should be fine
	RendererRoomNode nodes[1024];
	__int32 nextNode = 0;
	
	// Avoid reallocations, 1024 should be fine
	RendererRoomNode* stack[1024];
	__int32 stackDepth = 0;

	RendererRoomNode* node = &nodes[nextNode++];
	node->To = to;
	node->From = -1;

	// Push
	stack[stackDepth++] = node;

	while (stackDepth > 0)
	{
		// Pop
		node = stack[--stackDepth]; 
	
		if (m_rooms[node->To]->Visited)
			continue;

		ROOM_INFO* room = &Rooms[node->To];

		Vector3 roomCentre = Vector3(room->x + room->xSize * WALL_SIZE / 2.0f, 
									 (room->RoomYTop + room->RoomYBottom) / 2.0f,
									 room->z + room->ySize * WALL_SIZE / 2.0f);
		Vector3 laraPosition = Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z);

		m_rooms[node->To]->Distance = (roomCentre - laraPosition).Length();
		m_rooms[node->To]->Visited = true;
		m_roomsToDraw.Add(m_rooms[node->To]);

		collectLightsForRoom(node->To);
		collectItems(node->To);
		collectStatics(node->To);
		collectEffects(node->To);
				
		Vector4 clipPort;
		__int16 numDoors = *(room->door);
		if (numDoors)
		{
			__int16* door = room->door + 1;
			for (int i = 0; i < numDoors; i++) {
				__int16 adjoiningRoom = *(door);

				if (node->From != adjoiningRoom && checkPortal(node->To, door, viewPort, &node->ClipPort))
				{
					RendererRoomNode* childNode = &nodes[nextNode++];
					childNode->From = node->To;
					childNode->To = adjoiningRoom;

					// Push
					stack[stackDepth++] = childNode;
				}

				door += 16;
			}
		}
	}
}

bool Renderer11::checkPortal(__int16 roomIndex, __int16* portal, Vector4* viewPort, Vector4* clipPort)
{
	ROOM_INFO* room = &Rooms[roomIndex];

	Vector3 n = Vector3(*(portal + 1), *(portal + 2), *(portal + 3));
	Vector3 v = Vector3(Camera.pos.x - (room->x + *(portal + 4)),
		Camera.pos.y - (room->y + *(portal + 5)),
		Camera.pos.z - (room->z + *(portal + 6)));

	if (n.Dot(v) <= 0.0f)
		return false;

	int  zClip = 0;
	Vector4 p[4];

	clipPort->x = FLT_MAX;
	clipPort->y = FLT_MAX;
	clipPort->z = FLT_MIN;
	clipPort->w = FLT_MIN;

	for (int i = 0; i < 4; i++) {

		Vector4 tmp = Vector4(*(portal + 4 + 3 * i) + room->x, *(portal + 4 + 3 * i + 1) + room->y,
			*(portal + 4 + 3 * i + 2) + room->z, 1.0f);
		Vector4::Transform(tmp, ViewProjection, p[i]);

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
			Vector4 a = p[i];
			Vector4 b = p[(i + 1) % 4];

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

void Renderer11::collectRooms()
{
	__int16 baseRoomIndex = Camera.pos.roomNumber;

	for (__int32 i = 0; i < NumberRooms; i++)
	{
		m_rooms[i]->Visited = false;
		m_rooms[i]->LightsToDraw.Clear();
	}

	Vector4 vp = Vector4(-1.0f, -1.0f, 1.0f, 1.0f);

	getVisibleRooms(-1, baseRoomIndex, &vp, false, 0);
}

inline void Renderer11::collectItems(__int16 roomNumber)
{
	RendererRoom* room = m_rooms[roomNumber];
	if (room == NULL)
		return;

	ROOM_INFO* r = room->Room;

	__int16 itemNum = NO_ITEM;
	for (itemNum = r->itemNumber; itemNum != NO_ITEM; itemNum = Items[itemNum].nextItem)
	{
		ITEM_INFO* item = &Items[itemNum];

		if (item->objectNumber == ID_LARA && itemNum == Items[itemNum].nextItem)
			break;

		if (item->objectNumber == ID_LARA)
			continue;

		if (item->status == ITEM_INVISIBLE)
			continue;

		if (m_moveableObjects[item->objectNumber] == NULL)
			continue;

		RendererItem* newItem = &m_items[itemNum];

		newItem->Item = item;
		newItem->Id = itemNum;
		newItem->NumMeshes = Objects[item->objectNumber].nmeshes;
		newItem->World = Matrix::CreateFromYawPitchRoll(TR_ANGLE_TO_RAD(item->pos.yPos),
														TR_ANGLE_TO_RAD(item->pos.xPos),
														TR_ANGLE_TO_RAD(item->pos.zPos)) *
						 Matrix::CreateTranslation(item->pos.xPos, item->pos.yPos, item->pos.zPos);

		collectLightsForItem(item->roomNumber, newItem);

		m_itemsToDraw.Add(newItem);
	}
}

inline void Renderer11::collectStatics(__int16 roomNumber)
{
	RendererRoom* room = m_rooms[roomNumber];
	if (room == NULL)
		return;

	ROOM_INFO* r = room->Room;
	
	if (r->numMeshes <= 0)
		return;

	MESH_INFO* mesh = r->mesh;

	__int32 numStatics = room->Statics.size();

	for (__int32 i = 0; i < numStatics; i++)
	{
		RendererStatic* newStatic = &room->Statics[i];

		newStatic->Mesh = mesh;
		newStatic->RoomIndex = roomNumber;
		newStatic->World = Matrix::CreateRotationY(TR_ANGLE_TO_RAD(mesh->yRot)) * Matrix::CreateTranslation(mesh->x, mesh->y, mesh->z);

		m_staticsToDraw.Add(newStatic);

		mesh++;
	}
}

inline void Renderer11::collectLightsForItem(__int16 roomNumber, RendererItem* item)
{
	item->Lights.Clear();

	RendererRoom* room = m_rooms[roomNumber];
	if (room == NULL)
		return;

	ROOM_INFO* r = room->Room;

	if (r->numLights <= 0)
		return;

	m_tempItemLights.Clear();

	Vector3 itemPosition = Vector3(item->Item->pos.xPos, item->Item->pos.yPos, item->Item->pos.zPos);

	// Dynamic lights have the priority
	for (__int32 i = 0; i < m_dynamicLights.Size(); i++)
	{
		RendererLight* light = m_dynamicLights[i];

		Vector3 lightPosition = Vector3(light->Position.x, light->Position.y, light->Position.z);
		
		float distance = (itemPosition - lightPosition).Length();
		if (distance > light->Out)
			continue;

		m_tempItemLights.Add(light);
	}

	__int32 numLights = room->Lights.size();

	m_shadowLight = NULL;
	RendererLight* brightestLight = NULL;
	float brightest = 0.0f;

	for (__int32 j = 0; j < numLights; j++)
	{
		RendererLight* light = &room->Lights[j];

		// Check only lights different from sun
		if (light->Type == LIGHT_TYPE_SUN)
		{
			// Sun is added without checks
		}
		else if (light->Type == LIGHT_TYPE_POINT || light->Type == LIGHT_TYPE_SHADOW)
		{
			Vector3 lightPosition = Vector3(light->Position.x, light->Position.y, light->Position.z);

			float distance = (itemPosition - lightPosition).Length();

			// Collect only lights nearer than 20 sectors
			if (distance >= 20 * WALL_SIZE)
				continue;

			// Check the out radius
			if (distance > light->Out)
				continue;

			// If Lara, try to collect shadow casting light
			if (item->Item->objectNumber == ID_LARA)
			{
				float attenuation = 1.0f - distance / light->Out;
				float intensity = max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

				if (intensity >= brightest)
				{
					brightest = intensity;
					brightestLight = light;
				}
			}
		}     
		else if (light->Type == LIGHT_TYPE_SPOT)
		{
			Vector3 lightPosition = Vector3(light->Position.x, light->Position.y, light->Position.z);

			float distance = (itemPosition - lightPosition).Length();

			// Collect only lights nearer than 20 sectors
			if (distance >= 20 * WALL_SIZE)
				continue;

			// Check the range
			if (distance > light->Range)
				continue;

			// If Lara, try to collect shadow casting light
			if (item->Item->objectNumber == ID_LARA)
			{
				float attenuation = 1.0f - distance / light->Range;
				float intensity = max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

				if (intensity >= brightest)
				{
					brightest = intensity;
					brightestLight = light;
				}
			}
		}
		else
		{ 
			// Invalid light type
			continue;
		}
		       
		m_tempItemLights.Add(light);
	}
	  
	for (__int32 i = 0; i < min(MAX_LIGHTS_PER_ITEM, m_tempItemLights.Size()); i++)
	{
		item->Lights.Add(m_tempItemLights[i]); 
	}

	if (item->Item->objectNumber == ID_LARA)
	{
		m_shadowLight = brightestLight;
	}
}

inline void Renderer11::collectLightsForRoom(__int16 roomNumber)
{
	RendererRoom* room = m_rooms[roomNumber];
	if (room == NULL)
		return;

	ROOM_INFO* r = room->Room;

	if (r->numLights <= 0)
		return;

	__int32 numLights = room->Lights.size();

	// Collect dynamic lights for rooms
	for (__int32 i = 0; i < m_dynamicLights.Size(); i++)
	{
		RendererLight* light = m_dynamicLights[i];

		float left = r->x + WALL_SIZE;
		float bottom = r->z + WALL_SIZE;
		float right = r->x + (r->xSize - 1) * WALL_SIZE;
		float top = r->z + (r->ySize - 1) * WALL_SIZE;

		float closestX = light->Position.x;
		if (closestX < left)
			closestX = left;
		else if (closestX > right)
			closestX = right;

		float closestZ = light->Position.z;
		if (closestZ < bottom)
			closestZ = bottom;
		else if (closestZ > top)
			closestZ = top;

		// Calculate the distance between the circle's center and this closest point
		float distanceX = light->Position.x - closestX;
		float distanceY = light->Position.z - closestZ;

		// If the distance is less than the circle's radius, an intersection occurs
		float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);
		if (distanceSquared < SQUARE(light->Out))
			room->LightsToDraw.Add(light);
	}
}

bool Renderer11::sphereBoxIntersection(Vector3 boxMin, Vector3 boxMax, Vector3 sphereCentre, float sphereRadius)
{
	//Vector3 closestPointInAabb = Vector3::Min(Vector3::Max(sphereCentre, boxMin), boxMax);
	//double distanceSquared = (closestPointInAabb - sphereCentre).LengthSquared();
	//return (distanceSquared < (sphereRadius * sphereRadius));

	/*float x = max(boxMin.x, min(sphereCentre.x, boxMax.x));
	float y = max(boxMin.y, min(sphereCentre.y, boxMax.y));
	float z = max(boxMin.z, min(sphereCentre.z, boxMax.z));

	float distance = sqrt((x - sphereCentre.x) * (x - sphereCentre.x) +
		(y - sphereCentre.y) * (y - sphereCentre.y) +
		(z - sphereCentre.z) * (z - sphereCentre.z));

	return (distance < sphereRadius);*/
	return 0;
}

void Renderer11::prepareLights()
{
	// Add dynamic lights
	for (__int32 i = 0; i < m_dynamicLights.Size(); i++)
		m_lightsToDraw.Add(m_dynamicLights[i]);

	// Now I have a list full of draw. Let's sort them.
	//std::sort(m_lightsToDraw.begin(), m_lightsToDraw.end(), SortLightsFunction);
	//m_lightsToDraw.Sort(SortLightsFunction);

	// Let's draw first 32 lights
	//if (m_lightsToDraw.size() > 32)
	//	m_lightsToDraw.resize(32);

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
			RendererLight* light = &room->Lights[j];

			Vector4 itemPos = Vector4(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, 1.0f);
			Vector4 lightVector = itemPos - light->Position;

			float distance = lightVector.Length();
			lightVector.Normalize();

			float intensity;
			float attenuation;
			float angle;
			float d;
			float attenuationRange;
			float attenuationAngle;

			switch ((int)light->Type)
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

inline void Renderer11::collectEffects(__int16 roomNumber)
{
	RendererRoom* room = m_rooms[roomNumber];
	if (room == NULL)
		return;

	ROOM_INFO* r = room->Room;

	__int16 fxNum = NO_ITEM;
	for (fxNum = r->fxNumber; fxNum != NO_ITEM; fxNum = Effects[fxNum].nextFx)
	{
		FX_INFO* fx = &Effects[fxNum];

		if (fx->objectNumber < 0)
			continue;

		RendererEffect* newEffect = &m_effects[fxNum];
		
		newEffect->Effect = fx;
		newEffect->Id = fxNum;
		newEffect->World = Matrix::CreateTranslation(fx->pos.xPos, fx->pos.yPos, fx->pos.zPos);

		m_effectsToDraw.Add(newEffect);
	}
}

RendererMesh* Renderer11::getRendererMeshFromTrMesh(RendererObject* obj, __int16* meshPtr, __int16* refMeshPtr,
	__int16 boneIndex, __int32 isJoints, __int32 isHairs)
{
	RendererMesh* mesh = new RendererMesh();
	
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

		mesh->Positions.push_back(Vector3(x, y, z));
	}

	__int16 numNormals = *meshPtr++;
	VECTOR* normals = NULL;
	__int16* colors = NULL;
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
	{
		__int16 numLights = -numNormals;
		colors = (__int16*)malloc(sizeof(__int16) * numLights);
		for (__int32 v = 0; v < numLights; v++)
		{
			colors[v] = *meshPtr++;
		}
	}

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
			else
				bucketIndex = RENDERER_BUCKET_SOLID;
		}
		else
		{
			if (texture->attribute == 2 || (effects & 1))
				bucketIndex = RENDERER_BUCKET_TRANSPARENT_DS;
			else
				bucketIndex = RENDERER_BUCKET_SOLID_DS;
		}

		// ColAddHorizon special handling
		if (obj->Id == ID_HORIZON && g_GameFlow->GetLevel(CurrentLevel)->ColAddHorizon)
		{
			if (texture->attribute == 2 || (effects & 1))
				bucketIndex = RENDERER_BUCKET_TRANSPARENT;
			else
				bucketIndex = RENDERER_BUCKET_SOLID;
		}

		bucket = &mesh->Buckets[bucketIndex];
		obj->HasDataInBucket[bucketIndex] = true;

		__int32 baseVertices = bucket->NumVertices;
		for (__int32 v = 0; v < 4; v++)
		{
			RendererVertex vertex;

			vertex.Position.x = vertices[indices[v]].vx;
			vertex.Position.y = vertices[indices[v]].vy;
			vertex.Position.z = vertices[indices[v]].vz;

			if (numNormals > 0)
			{
				vertex.Normal.x = normals[indices[v]].vx / 16300.0f;
				vertex.Normal.y = normals[indices[v]].vy / 16300.0f;
				vertex.Normal.z = normals[indices[v]].vz / 16300.0f;
			}

			vertex.UV.x = (texture->vertices[v].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
			vertex.UV.y = (texture->vertices[v].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;

			vertex.Bone = boneIndex;
			if (isJoints && boneIndex != 0 && m_laraSkinJointRemap[boneIndex][indices[v]] != -1)
				vertex.Bone = m_laraSkinJointRemap[boneIndex][indices[v]];
			if (isHairs)
				vertex.Bone = indices[v];

			if (colors == NULL)
			{
				vertex.Color = Vector4::One * 0.5f;
			}
			else
			{
				__int16 shade = colors[indices[v]];
				shade = (255 - shade * 255 / 8191) & 0xFF;
				vertex.Color = Vector4(shade / 255.0f, shade / 255.0f, shade / 255.0f, 1.0f);
			}

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
		newPolygon.TextureId = textureId;
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
			else
				bucketIndex = RENDERER_BUCKET_SOLID;
		}
		else
		{
			if (texture->attribute == 2 || (effects & 1))
				bucketIndex = RENDERER_BUCKET_TRANSPARENT_DS;
			else
				bucketIndex = RENDERER_BUCKET_SOLID_DS;
		}
		bucket = &mesh->Buckets[bucketIndex];
		obj->HasDataInBucket[bucketIndex] = true;

		__int32 baseVertices = bucket->NumVertices;
		for (__int32 v = 0; v < 3; v++)
		{
			RendererVertex vertex;

			vertex.Position.x = vertices[indices[v]].vx;
			vertex.Position.y = vertices[indices[v]].vy;
			vertex.Position.z = vertices[indices[v]].vz;

			if (numNormals > 0)
			{
				vertex.Normal.x = normals[indices[v]].vx / 16300.0f;
				vertex.Normal.y = normals[indices[v]].vy / 16300.0f;
				vertex.Normal.z = normals[indices[v]].vz / 16300.0f;
			}

			vertex.UV.x = (texture->vertices[v].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
			vertex.UV.y = (texture->vertices[v].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;

			vertex.Bone = boneIndex;
			if (isJoints && boneIndex != 0 && m_laraSkinJointRemap[boneIndex][indices[v]] != -1)
				vertex.Bone = m_laraSkinJointRemap[boneIndex][indices[v]];
			if (isHairs)
				vertex.Bone = indices[v];

			if (colors == NULL)
			{
				vertex.Color = Vector4::One * 0.5f;
			}
			else
			{
				__int16 shade = colors[indices[v]];
				shade = (255 - shade * 255 / 8191) & 0xFF;
				vertex.Color = Vector4(shade / 255.0f, shade / 255.0f, shade / 255.0f, 1.0f);
			}

			bucket->NumVertices++;
			bucket->Vertices.push_back(vertex);
		}

		bucket->Indices.push_back(baseVertices);
		bucket->Indices.push_back(baseVertices + 1);
		bucket->Indices.push_back(baseVertices + 2);
		bucket->NumIndices += 3;

		RendererPolygon newPolygon;
		newPolygon.Shape = SHAPE_TRIANGLE;
		newPolygon.TextureId = textureId;
		newPolygon.Indices[0] = baseVertices;
		newPolygon.Indices[1] = baseVertices + 1;
		newPolygon.Indices[2] = baseVertices + 2;
		bucket->Polygons.push_back(newPolygon);
	}

	free(vertices);
	if (normals != NULL) free(normals);
	if (colors != NULL) free(colors);

	unsigned int castedMeshPtr = reinterpret_cast<unsigned int>(refMeshPtr);
		
	if (m_meshPointersToMesh.find(castedMeshPtr) == m_meshPointersToMesh.end())
		m_meshPointersToMesh.insert(pair<unsigned int, RendererMesh*>(castedMeshPtr, mesh));
	
	m_meshes.push_back(mesh);

	return mesh;
}

void Renderer11::buildHierarchyRecursive(RendererObject* obj, RendererBone* node, RendererBone* parentNode)
{
	node->GlobalTransform = node->Transform * parentNode->GlobalTransform;
	obj->BindPoseTransforms[node->Index] = node->GlobalTransform;
	obj->Skeleton->GlobalTranslation = Vector3(0.0f, 0.0f, 0.0f);
	node->GlobalTranslation = node->Translation + parentNode->GlobalTranslation;

	for (int j = 0; j < node->Children.size(); j++)
	{
		buildHierarchyRecursive(obj, node->Children[j], node);
	}
}

void Renderer11::buildHierarchy(RendererObject* obj)
{
	obj->Skeleton->GlobalTransform = obj->Skeleton->Transform;
	obj->BindPoseTransforms[obj->Skeleton->Index] = obj->Skeleton->GlobalTransform;
	obj->Skeleton->GlobalTranslation = Vector3(0.0f, 0.0f, 0.0f);

	for (int j = 0; j < obj->Skeleton->Children.size(); j++)
	{
		buildHierarchyRecursive(obj, obj->Skeleton->Children[j], obj->Skeleton);
	}
}

void Renderer11::fromTrAngle(Matrix* matrix, __int16* frameptr, __int32 index)
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

		*matrix = Matrix::CreateFromYawPitchRoll(rotY* (360.0f / 1024.0f) * RADIAN,
			rotX* (360.0f / 1024.0f) * RADIAN,
			rotZ* (360.0f / 1024.0f) * RADIAN);
		break;

	case 0x4000:
		*matrix = Matrix::CreateRotationX((rot0 & 0xfff)* (360.0f / 4096.0f) * RADIAN);
		break;

	case 0x8000:
		*matrix = Matrix::CreateRotationY((rot0 & 0xfff)* (360.0f / 4096.0f) * RADIAN);
		break;

	case 0xc000:
		*matrix = Matrix::CreateRotationZ((rot0 & 0xfff)* (360.0f / 4096.0f) * RADIAN);
		break;
	}
}

bool Renderer11::updateConstantBuffer(ID3D11Buffer* buffer, void* data, __int32 size)
{
	HRESULT res;
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Lock the constant buffer so it can be written to.
	res = m_context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(res))
		return false;

	// Get a pointer to the data in the constant buffer.
	char* dataPtr = reinterpret_cast<char*>(mappedResource.pData);
	memcpy(dataPtr, data, size);

	// Unlock the constant buffer.
	m_context->Unmap(buffer, 0);

	return true;
}

void Renderer11::updateItemsAnimations()
{
	Matrix translation;
	Matrix rotation;

	__int32 numItems = m_itemsToDraw.Size();

	for (__int32 i = 0; i < numItems; i++)
	{
		RendererItem* itemToDraw = m_itemsToDraw[i];
		ITEM_INFO* item = itemToDraw->Item;
		CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

		// Lara has her own routine
		if (item->objectNumber == ID_LARA)
			continue;

		OBJECT_INFO* obj = &Objects[item->objectNumber];
		RendererObject* moveableObj = m_moveableObjects[item->objectNumber];

		// Update animation matrices
		if (obj->animIndex != -1 /*&& item->objectNumber != ID_HARPOON*/)
		{
			// Apply extra rotations
			__int32 lastJoint = 0;
			for (__int32 j = 0; j < moveableObj->LinearizedBones.size(); j++)
			{
				RendererBone* currentBone = moveableObj->LinearizedBones[j];
				currentBone->ExtraRotation = Vector3(0.0f, 0.0f, 0.0f);

				if (creature)
				{
					if (currentBone->ExtraRotationFlags & ROT_Y)
					{
						currentBone->ExtraRotation.y = TR_ANGLE_TO_RAD(creature->jointRotation[lastJoint]);
						lastJoint++;
					}

					if (currentBone->ExtraRotationFlags & ROT_X)
					{
						currentBone->ExtraRotation.x = TR_ANGLE_TO_RAD(creature->jointRotation[lastJoint]);
						lastJoint++;
					}

					if (currentBone->ExtraRotationFlags & ROT_Z)
					{
						currentBone->ExtraRotation.z = TR_ANGLE_TO_RAD(creature->jointRotation[lastJoint]);
						lastJoint++;
					}
				}
			}

			__int16	*framePtr[2];
			__int32 rate;
			__int32 frac = GetFrame_D2(item, framePtr, &rate);

			updateAnimation(itemToDraw, moveableObj, framePtr, frac, rate, 0xFFFFFFFF);

			for (__int32 m = 0; m < itemToDraw->NumMeshes; m++)
				itemToDraw->AnimationTransforms[m] = itemToDraw->AnimationTransforms[m].Transpose();
		}

		// Update world matrix
		translation = Matrix::CreateTranslation(item->pos.xPos, item->pos.yPos, item->pos.zPos);
		rotation = Matrix::CreateFromYawPitchRoll(TR_ANGLE_TO_RAD(item->pos.yRot), TR_ANGLE_TO_RAD(item->pos.xRot), TR_ANGLE_TO_RAD(item->pos.zRot));
		itemToDraw->World = rotation * translation;
	}
}

void Renderer11::updateLaraAnimations()
{
	Matrix translation;
	Matrix rotation;
	Matrix lastMatrix;
	Matrix hairMatrix;
	Matrix identity;
	Matrix world;

	RendererObject* laraObj = m_moveableObjects[ID_LARA];

	// Clear extra rotations
	for (__int32 i = 0; i < laraObj->LinearizedBones.size(); i++)
		laraObj->LinearizedBones[i]->ExtraRotation = Vector3(0.0f, 0.0f, 0.0f);

	// Lara world matrix
	translation = Matrix::CreateTranslation(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
	rotation = Matrix::CreateFromYawPitchRoll(
		TR_ANGLE_TO_RAD(LaraItem->pos.yRot),
		TR_ANGLE_TO_RAD(LaraItem->pos.xRot),
		TR_ANGLE_TO_RAD(LaraItem->pos.zRot));

	m_LaraWorldMatrix = rotation * translation;
	
	// Update first Lara's animations
	laraObj->LinearizedBones[TORSO]->ExtraRotation = Vector3(TR_ANGLE_TO_RAD(Lara.torsoXrot),
		TR_ANGLE_TO_RAD(Lara.torsoYrot), TR_ANGLE_TO_RAD(Lara.torsoZrot));
	laraObj->LinearizedBones[HEAD]->ExtraRotation = Vector3(TR_ANGLE_TO_RAD(Lara.headXrot),
		TR_ANGLE_TO_RAD(Lara.headYrot), TR_ANGLE_TO_RAD(Lara.headZrot));

	// First calculate matrices for legs, hips, head and torso
	__int32 mask = (1 << HIPS) | (1 << THIGH_L) | (1 << CALF_L) | (1 << FOOT_L) |
		(1 << THIGH_R) | (1 << CALF_R) | (1 << FOOT_R) | (1 << TORSO) | (1 << HEAD);
	__int16	*framePtr[2];
	__int32 rate;
	__int32 frac = GetFrame_D2(LaraItem, framePtr, &rate);
	updateAnimation(NULL, laraObj, framePtr, frac, rate, mask);

	// Then the arms, based on current weapon status
	if ((Lara.gunStatus == LG_NO_ARMS || Lara.gunStatus == LG_HANDS_BUSY) && Lara.gunType != WEAPON_FLARE)
	{
		// Both arms
		mask = (1 << UARM_L) | (1 << LARM_L) | (1 << HAND_L) | (1 << UARM_R) |
			(1 << LARM_R) | (1 << HAND_R);
		frac = GetFrame_D2(LaraItem, framePtr, &rate);
		updateAnimation(NULL, laraObj, framePtr, frac, rate, mask);
	}
	else
	{ 
		// While handling weapon some extra rotation could be applied to arms
		laraObj->LinearizedBones[UARM_L]->ExtraRotation += Vector3(TR_ANGLE_TO_RAD(Lara.leftArm.xRot),
			TR_ANGLE_TO_RAD(Lara.leftArm.yRot), TR_ANGLE_TO_RAD(Lara.leftArm.zRot));
		laraObj->LinearizedBones[UARM_R]->ExtraRotation += Vector3(TR_ANGLE_TO_RAD(Lara.rightArm.xRot),
			TR_ANGLE_TO_RAD(Lara.rightArm.yRot), TR_ANGLE_TO_RAD(Lara.rightArm.zRot));

		if (Lara.gunType != WEAPON_FLARE)
		{
			// HACK: backguns handles differently
			if (Lara.gunType == WEAPON_SHOTGUN || Lara.gunType == WEAPON_GRENADE_LAUNCHER ||
				Lara.gunType == WEAPON_CROSSBOW || Lara.gunType == WEAPON_ROCKET_LAUNCHER ||
				Lara.gunType == WEAPON_HARPOON_GUN)
			{
				// Left arm
				mask = (1 << UARM_L) | (1 << LARM_L) | (1 << HAND_L);
				__int16* shotgunFramePtr = Lara.leftArm.frameBase + (Lara.leftArm.frameNumber) * (Anims[Lara.leftArm.animNumber].interpolation >> 8);
				updateAnimation(NULL, laraObj, &shotgunFramePtr, 0, 1, mask);

				// Right arm
				mask = (1 << UARM_R) | (1 << LARM_R) | (1 << HAND_R);
				shotgunFramePtr = Lara.rightArm.frameBase + (Lara.rightArm.frameNumber) * (Anims[Lara.rightArm.animNumber].interpolation >> 8);
				updateAnimation(NULL, laraObj, &shotgunFramePtr, 0, 1, mask);
			}
			else
			{
				// Left arm
				mask = (1 << UARM_L) | (1 << LARM_L) | (1 << HAND_L);
				frac = getFrame(Lara.leftArm.animNumber, Lara.leftArm.frameNumber, framePtr, &rate);
				updateAnimation(NULL, laraObj, framePtr, frac, rate, mask);

				// Right arm
				mask = (1 << UARM_R) | (1 << LARM_R) | (1 << HAND_R);
				frac = getFrame(Lara.rightArm.animNumber, Lara.rightArm.frameNumber, framePtr, &rate);
				updateAnimation(NULL, laraObj, framePtr, frac, rate, mask);
			}
		}
		else
		{
			// Left arm
			mask = (1 << UARM_L) | (1 << LARM_L) | (1 << HAND_L);
			frac = getFrame(Lara.leftArm.animNumber, Lara.leftArm.frameNumber, framePtr, &rate);
			updateAnimation(NULL, laraObj, framePtr, frac, rate, mask);

			// Right arm
			mask = (1 << UARM_R) | (1 << LARM_R) | (1 << HAND_R);
			frac = GetFrame_D2(LaraItem, framePtr, &rate);
			updateAnimation(NULL, laraObj, framePtr, frac, rate, mask);
		}

	}		 

	// At this point, Lara's matrices are ready. Now let's do ponytails...
	if (m_moveableObjects[ID_HAIR] != NULL)
	{
		RendererObject* hairsObj = m_moveableObjects[ID_HAIR];

		lastMatrix = Matrix::Identity; 
		identity = Matrix::Identity;

		Vector3 parentVertices[6][4];
		Matrix headMatrix;

		RendererObject* objSkin = m_moveableObjects[ID_LARA_SKIN];
		RendererObject* objLara = m_moveableObjects[ID_LARA];
		RendererMesh* parentMesh = objSkin->ObjectMeshes[HEAD];
		RendererBone* parentBone = objSkin->LinearizedBones[HEAD];

		world = objLara->AnimationTransforms[HEAD] * m_LaraWorldMatrix;

		__int32 lastVertex = 0;
		__int32 lastIndex = 0;

		GameScriptLevel* level = g_GameFlow->GetLevel(CurrentLevel);

		for (__int32 p = 0; p < ((level->LaraType == LARA_YOUNG) ? 2 : 1); p++)
		{
			// We can't use hardware skinning here, however hairs have just a few vertices so 
			// it's not so bad doing skinning in software
			if (level->LaraType == LARA_YOUNG)
			{
				if (p == 1)
				{
					parentVertices[0][0] = Vector3::Transform(parentMesh->Positions[68], world);
					parentVertices[0][1] = Vector3::Transform(parentMesh->Positions[69], world);
					parentVertices[0][2] = Vector3::Transform(parentMesh->Positions[70], world);
					parentVertices[0][3] = Vector3::Transform(parentMesh->Positions[71], world);
				}
				else
				{
					parentVertices[0][0] = Vector3::Transform(parentMesh->Positions[78], world);
					parentVertices[0][1] = Vector3::Transform(parentMesh->Positions[78], world);
					parentVertices[0][2] = Vector3::Transform(parentMesh->Positions[77], world);
					parentVertices[0][3] = Vector3::Transform(parentMesh->Positions[76], world);
				}
			}
			else
			{
				parentVertices[0][0] = Vector3::Transform(parentMesh->Positions[37], world);
				parentVertices[0][1] = Vector3::Transform(parentMesh->Positions[39], world);
				parentVertices[0][2] = Vector3::Transform(parentMesh->Positions[40], world);
				parentVertices[0][3] = Vector3::Transform(parentMesh->Positions[38], world);
			}

			for (__int32 i = 0; i < 6; i++)
			{
				RendererMesh* mesh = hairsObj->ObjectMeshes[i];
				RendererBucket* bucket = &mesh->Buckets[RENDERER_BUCKET_SOLID];

				translation = Matrix::CreateTranslation(Hairs[7 * p + i].pos.xPos, Hairs[7 * p + i].pos.yPos, Hairs[7 * p + i].pos.zPos);
				rotation = Matrix::CreateFromYawPitchRoll(
					TR_ANGLE_TO_RAD(Hairs[7 * p + i].pos.yRot),
					TR_ANGLE_TO_RAD(Hairs[7 * p + i].pos.xRot),
					TR_ANGLE_TO_RAD(Hairs[7 * p + i].pos.zRot));
				m_hairsMatrices[6 * p + i] = rotation * translation;

				__int32 baseVertex = lastVertex;

				for (__int32 j = 0; j < bucket->Vertices.size(); j++)
				{
					__int32 oldVertexIndex = (__int32)bucket->Vertices[j].Bone;
					if (oldVertexIndex < 4)
					{
						m_hairVertices[lastVertex].Position.x = parentVertices[i][oldVertexIndex].x;
						m_hairVertices[lastVertex].Position.y = parentVertices[i][oldVertexIndex].y;
						m_hairVertices[lastVertex].Position.z = parentVertices[i][oldVertexIndex].z;
						m_hairVertices[lastVertex].UV.x = bucket->Vertices[j].UV.x;
						m_hairVertices[lastVertex].UV.y = bucket->Vertices[j].UV.y;

						Vector3 n = Vector3(bucket->Vertices[j].Normal.x, bucket->Vertices[j].Normal.y, bucket->Vertices[j].Normal.z);
						n.Normalize();
						n = Vector3::TransformNormal(n, m_hairsMatrices[6 * p + i]);
						n.Normalize();

						m_hairVertices[lastVertex].Normal.x = n.x;
						m_hairVertices[lastVertex].Normal.y = n.y;
						m_hairVertices[lastVertex].Normal.z = n.z;

						m_hairVertices[lastVertex].Color = Vector4::One * 0.5f;

						lastVertex++;
					}
					else
					{
						Vector3 in = Vector3(bucket->Vertices[j].Position.x, bucket->Vertices[j].Position.y, bucket->Vertices[j].Position.z);
						Vector3 out = Vector3::Transform(in, m_hairsMatrices[6 * p + i]);

						if (i < 5)
						{
							parentVertices[i + 1][oldVertexIndex - 4].x = out.x;
							parentVertices[i + 1][oldVertexIndex - 4].y = out.y;
							parentVertices[i + 1][oldVertexIndex - 4].z = out.z;
						}

						m_hairVertices[lastVertex].Position.x = out.x;
						m_hairVertices[lastVertex].Position.y = out.y;
						m_hairVertices[lastVertex].Position.z = out.z;
						m_hairVertices[lastVertex].UV.x = bucket->Vertices[j].UV.x;
						m_hairVertices[lastVertex].UV.y = bucket->Vertices[j].UV.y;

						Vector3 n = Vector3(bucket->Vertices[j].Normal.x, bucket->Vertices[j].Normal.y, bucket->Vertices[j].Normal.z);
						n.Normalize();
						n = Vector3::TransformNormal(n, m_hairsMatrices[6 * p + i]);
						n.Normalize();

						m_hairVertices[lastVertex].Normal.x = n.x;
						m_hairVertices[lastVertex].Normal.y = n.y;
						m_hairVertices[lastVertex].Normal.z = n.z;

						m_hairVertices[lastVertex].Color = Vector4::One * 0.5f;

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

	// Transpose matrices for shaders
	for (__int32 m = 0; m < 15; m++)
		laraObj->AnimationTransforms[m] = laraObj->AnimationTransforms[m].Transpose();
}

__int32 Renderer11::getFrame(__int16 animation, __int16 frame, __int16** framePtr, __int32* rate)
{
	ITEM_INFO item;
	item.animNumber = animation;
	item.frameNumber = frame;

	return GetFrame_D2(&item, framePtr, rate);
}

void Renderer11::updateEffects()
{
	for (__int32 i = 0; i < m_effectsToDraw.Size(); i++)
	{
		RendererEffect* fx = m_effectsToDraw[i];

		Matrix translation = Matrix::CreateTranslation(fx->Effect->pos.xPos, fx->Effect->pos.yPos, fx->Effect->pos.zPos);
		Matrix rotation = Matrix::CreateFromYawPitchRoll(
			TR_ANGLE_TO_RAD(fx->Effect->pos.yRot),
			TR_ANGLE_TO_RAD(fx->Effect->pos.xRot),
			TR_ANGLE_TO_RAD(fx->Effect->pos.zRot));
		m_effectsToDraw[i]->World = rotation * translation;
	}
}

void Renderer11::updateAnimation(RendererItem* item, RendererObject* obj, __int16** frmptr, __int16 frac, __int16 rate, __int32 mask)
{
	RendererBone* bones[32];
	__int32 nextBone = 0;

	Matrix rotation;

	Matrix* transforms = (item == NULL ? obj->AnimationTransforms.data() : &item->AnimationTransforms[0]);

	// Push
	bones[nextBone++] = obj->Skeleton;

	while (nextBone != 0)
	{
		// Pop the last bone in the stack
		RendererBone* bone = bones[--nextBone];

		bool calculateMatrix = (mask >> bone->Index) & 1;

		if (calculateMatrix)
		{
			Vector3 p = Vector3((int)*(frmptr[0] + 6), (int)*(frmptr[0] + 7), (int)*(frmptr[0] + 8));

			fromTrAngle(&rotation, frmptr[0], bone->Index);

			if (frac)
			{
				Vector3 p2 = Vector3((int)*(frmptr[1] + 6), (int)*(frmptr[1] + 7), (int)*(frmptr[1] + 8));
				p = Vector3::Lerp(p, p2, frac / ((float)rate));

				Matrix rotation2;
				fromTrAngle(&rotation2, frmptr[1], bone->Index);

				Quaternion q1, q2, q3;

				q1 = Quaternion::CreateFromRotationMatrix(rotation);
				q2 = Quaternion::CreateFromRotationMatrix(rotation2);
				q3 = Quaternion::Slerp(q1, q2, frac / ((float)rate));

				rotation = Matrix::CreateFromQuaternion(q3);
			}

			Matrix translation;
			if (bone == obj->Skeleton)
				translation = Matrix::CreateTranslation(p.x, p.y, p.z);

			Matrix extraRotation;
			extraRotation = Matrix::CreateFromYawPitchRoll(bone->ExtraRotation.y, bone->ExtraRotation.x, bone->ExtraRotation.z);

			rotation = extraRotation * rotation;

			if (bone != obj->Skeleton)
				transforms[bone->Index] = rotation * bone->Transform;
			else
				transforms[bone->Index] = rotation * translation;

			if (bone != obj->Skeleton)
				transforms[bone->Index] = transforms[bone->Index] * transforms[bone->Parent->Index];
		}

		for (__int32 i = 0; i < bone->Children.size(); i++)
		{
			// Push
			bones[nextBone++] = bone->Children[i];
		}
	}
}

bool Renderer11::printDebugMessage(__int32 x, __int32 y, __int32 alpha, byte r, byte g, byte b, LPCSTR Message)
{

	return true;
}

void Renderer11::printDebugMessage(char* message, ...)
{
	char buffer[255];
	ZeroMemory(buffer, 255);

	va_list args;
	va_start(args, message);
	_vsprintf_l(buffer, message, NULL, args);
	va_end(args);

	PrintString(10, m_currentY, buffer, 0xFFFFFFFF, PRINTSTRING_OUTLINE);

	m_currentY += 20;
}

void Renderer11::drawBlood()
{
	for (__int32 i = 0; i < 32; i++)
	{
		BLOOD_STRUCT* blood = &Blood[i];
		if (blood->On)
		{
			addSpriteBillboard(m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + 15],
				blood->x, blood->y, blood->z,
				blood->Shade * 244, blood->Shade * 0, blood->Shade * 0,
				TR_ANGLE_TO_RAD(blood->RotAng), 1.0f, blood->Size * 8.0f, blood->Size * 8.0f,
				BLENDMODE_ALPHABLEND);
		}
	}
}

void Renderer11::drawSparks()
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
					TR_ANGLE_TO_RAD(spark->rotAng), spark->scalar, spark->size * 12.0f, spark->size * 12.0f,
					BLENDMODE_ALPHABLEND);
			}
			else
			{
				Vector3 v = Vector3(spark->xVel, spark->yVel, spark->zVel);
				v.Normalize();
				addLine3D(spark->x, spark->y, spark->z, spark->x + v.x * 24.0f, spark->y + v.y * 24.0f, spark->z + v.z * 24.0f, spark->r, spark->g, spark->b);
			}
		}
	}
}

void Renderer11::drawFires()
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
						TR_ANGLE_TO_RAD(spark->rotAng), spark->scalar, spark->size * 4.0f, spark->size * 4.0f,
						BLENDMODE_ALPHABLEND);
				}
			}
		}
	}
}

void Renderer11::addSpriteBillboard(RendererSprite* sprite, float x, float y, float z, byte r, byte g, byte b, float rotation, float scale, float width, float height, BLEND_MODES blendMode)
{
	if (m_nextSprite >= MAX_SPRITES)
		return;

	scale = 1.0f;

	width *= scale;
	height *= scale;

	RendererSpriteToDraw* spr = &m_spritesBuffer[m_nextSprite++];

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
	spr->BlendMode = blendMode;

	m_spritesToDraw.Add(spr);
}

void Renderer11::drawSmokes()
{
	for (__int32 i = 0; i < 32; i++)
	{
		SMOKE_SPARKS* spark = &SmokeSparks[i];
		if (spark->On)
		{
			addSpriteBillboard(m_sprites[spark->Def],
				spark->x, spark->y, spark->z,
				spark->Shade, spark->Shade, spark->Shade,
				TR_ANGLE_TO_RAD(spark->RotAng), spark->Scalar, spark->Size * 4.0f, spark->Size * 4.0f,
				BLENDMODE_ALPHABLEND);
		}
	}
}

void Renderer11::addLine3D(__int32 x1, __int32 y1, __int32 z1, __int32 x2, __int32 y2, __int32 z2, byte r, byte g, byte b)
{
	if (m_nextLine3D >= MAX_LINES_3D)
		return;

	RendererLine3D* line = &m_lines3DBuffer[m_nextLine3D++];

	line->X1 = x1;
	line->Y1 = y1;
	line->Z1 = z1;
	line->X2 = x2;
	line->Y2 = y2;
	line->Z2 = z2;
	line->R = r;
	line->G = g;
	line->B = b;

	m_lines3DToDraw.Add(line);
}

void Renderer11::addSprite3D(RendererSprite* sprite, float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3, float x4, float y4, float z4, byte r, byte g, byte b, float rotation, float scale, float width, float height, BLEND_MODES blendMode)
{
	if (m_nextSprite >= MAX_SPRITES)
		return;

	scale = 1.0f;

	width *= scale;
	height *= scale;

	RendererSpriteToDraw* spr = &m_spritesBuffer[m_nextSprite++];

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
	spr->BlendMode = blendMode;

	m_spritesToDraw.Add(spr);
}

void Renderer11::drawShockwaves()
{
	for (__int32 i = 0; i < 16; i++)
	{
		SHOCKWAVE_STRUCT* shockwave = &ShockWaves[i];

		if (shockwave->life)
		{
			byte color = shockwave->life * 8;

			// Inner circle
			float angle = PI / 32.0f;
			float c = cos(angle);
			float s = sin(angle);
			float x1 = shockwave->x + (shockwave->innerRad * c);
			float z1 = shockwave->z + (shockwave->innerRad * s);
			float x4 = shockwave->x + (shockwave->outerRad * c);
			float z4 = shockwave->z + (shockwave->outerRad * s);
			angle -= PI / 8.0f;

			for (__int32 j = 0; j < 16; j++)
			{
				c = cos(angle);
				s = sin(angle);
				float x2 = shockwave->x + (shockwave->innerRad * c);
				float z2 = shockwave->z + (shockwave->innerRad * s);
				float x3 = shockwave->x + (shockwave->outerRad * c);
				float z3 = shockwave->z + (shockwave->outerRad * s);
				angle -= PI / 8.0f;

				addSprite3D(m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + 8],
					x1, shockwave->y, z1,
					x2, shockwave->y, z2,
					x3, shockwave->y, z3,
					x4, shockwave->y, z4,
					color, color, color, 0, 1, 0, 0, BLENDMODE_ALPHABLEND);

				x1 = x2;
				z1 = z2;
				x4 = x3;
				z4 = z3;
			}
		}
	}
}

void Renderer11::drawRipples()
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
				x1, y, z2, x2, y, z2, x2, y, z1, x1, y, z1, color, color, color, 0.0f, 1.0f, ripple->size, ripple->size,
				BLENDMODE_ALPHABLEND);
		}
	}
}

void Renderer11::drawDrips()
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

void Renderer11::drawBubbles()
{
	for (__int32 i = 0; i < 40; i++)
	{
		BUBBLE_STRUCT* bubble = &Bubbles[i];

		if (bubble->size)
		{
			addSpriteBillboard(m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + 13],
				bubble->pos.x, bubble->pos.y, bubble->pos.z,
				bubble->shade * 255, bubble->shade * 255, bubble->shade * 255,
				0.0f, 1.0f, bubble->size * 0.5f, bubble->size * 0.5f,
				BLENDMODE_ALPHABLEND);
		}
	}
}

void Renderer11::drawSplahes()
{
	for (__int32 i = 0; i < 4; i++)
	{
		SPLASH_STRUCT* splash = &Splashes[i];

		if (splash->flags & 1)
		{
			byte color = (splash->life >= 32 ? 255 : splash->life << 5);

			// Inner circle
			float angle = PI / 16.0f;
			float c = cos(angle);
			float s = sin(angle);
			float dx = splash->innerRad * c;
			float dz = splash->innerRad * s;
			float x1 = splash->x + dx;
			float z1 = splash->z + dz;
			angle -= PI / 4.0f;

			for (__int32 j = 0; j < 8; j++)
			{
				c = cos(angle);
				s = sin(angle);
				dx = splash->innerRad * c;
				dz = splash->innerRad * s;
				float x2 = splash->x + dx;
				float z2 = splash->z + dz;
				angle -= PI / 4.0f;

				addSprite3D(m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + 8],
					x1, splash->y + splash->innerY, z1,
					x2, splash->y + splash->innerY, z2,
					x2, splash->y, z2,
					x1, splash->y, z1,
					color, color, color, 0, 1, 0, 0, BLENDMODE_ALPHABLEND);

				x1 = x2;
				z1 = z2;
			}

			// Medium circle
			angle = PI / 16.0f;
			c = cos(angle);
			s = sin(angle);
			dx = splash->middleRad * c;
			dz = splash->middleRad * s;
			x1 = splash->x + dx;
			z1 = splash->z + dz;
			angle -= PI / 4.0f;

			for (__int32 j = 0; j < 8; j++)
			{
				c = cos(angle);
				s = sin(angle);
				dx = splash->middleRad * c;
				dz = splash->middleRad * s;
				float x2 = splash->x + dx;
				float z2 = splash->z + dz;
				angle -= PI / 4.0f;

				addSprite3D(m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + 8],
					x1, splash->y + splash->middleY, z1,
					x2, splash->y + splash->middleY, z2,
					x2, splash->y, z2,
					x1, splash->y, z1,
					color, color, color, 0, 1, 0, 0, BLENDMODE_ALPHABLEND);

				x1 = x2;
				z1 = z2;
			}

			// Large circle
			angle = PI / 16.0f;
			c = cos(angle);
			s = sin(angle);
			dx = splash->outerRad * c;
			dz = splash->outerRad * s;
			x1 = splash->x + dx;
			z1 = splash->z + dz;
			angle -= PI / 4.0f;

			for (__int32 j = 0; j < 8; j++)
			{
				c = cos(angle);
				s = sin(angle);
				dx = splash->outerRad * c;
				dz = splash->outerRad * s;
				float x2 = splash->x + dx;
				float z2 = splash->z + dz;
				angle -= PI / 4.0f;

				addSprite3D(m_sprites[Objects[ID_DEFAULT_SPRITES].meshIndex + 8],
					x1, splash->y - splash->outerSize, z1,
					x2, splash->y - splash->outerSize, z2,
					x2, splash->y, z2,
					x1, splash->y, z1,
					color, color, color, 0, 1, 0, 0, BLENDMODE_ALPHABLEND);

				x1 = x2;
				z1 = z2;
			}
		}
	}
}

bool Renderer11::drawSprites()
{
	m_context->RSSetState(m_states->CullNone());
	m_context->OMSetDepthStencilState(m_states->DepthRead(), 0);

	m_context->VSSetShader(m_vsSprites, NULL, 0);
	m_context->PSSetShader(m_psSprites, NULL, 0);

	m_stCameraMatrices.View = View.Transpose();
	m_stCameraMatrices.Projection = Projection.Transpose();
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	m_stMisc.AlphaTest = true;
	updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
	m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

	m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
	ID3D11SamplerState* sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);

	for (__int32 b = 0; b < 3; b++)
	{
		BLEND_MODES currentBlendMode = (BLEND_MODES)b;

		__int32 numSpritesToDraw = m_spritesToDraw.Size();
		__int32 lastSprite = 0;

		m_primitiveBatch->Begin();

		for (__int32 i = 0; i < numSpritesToDraw; i++)
		{
			RendererSpriteToDraw* spr = m_spritesToDraw[i];

			if (spr->BlendMode != currentBlendMode)
				continue;

			if (currentBlendMode == BLENDMODE_OPAQUE)
			{
				m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
			}
			else
			{
				m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);
			}

			if (spr->Type == RENDERER_SPRITE_TYPE::SPRITE_TYPE_BILLBOARD)
			{
				float halfWidth = spr->Width / 2.0f;
				float halfHeight = spr->Height / 2.0f;

				Matrix billboardMatrix;
				createBillboardMatrix(&billboardMatrix, &Vector3(spr->X, spr->Y, spr->Z),
					&Vector3(Camera.pos.x, Camera.pos.y, Camera.pos.z), spr->Rotation);

				Vector3 p0 = Vector3(-halfWidth, -halfHeight, 0);
				Vector3 p1 = Vector3(halfWidth, -halfHeight, 0);
				Vector3 p2 = Vector3(halfWidth, halfHeight, 0);
				Vector3 p3 = Vector3(-halfWidth, halfHeight, 0);

				Vector3 p0t = Vector3::Transform(p0, billboardMatrix);
				Vector3 p1t = Vector3::Transform(p1, billboardMatrix);
				Vector3 p2t = Vector3::Transform(p2, billboardMatrix);
				Vector3 p3t = Vector3::Transform(p3, billboardMatrix);

				RendererVertex v0;				
				v0.Position.x = p0t.x;
				v0.Position.y = p0t.y;
				v0.Position.z = p0t.z;
				v0.UV.x = spr->Sprite->UV[0].x;
				v0.UV.y = spr->Sprite->UV[0].y;
				v0.Color.x = spr->R / 255.0f;
				v0.Color.y = spr->G / 255.0f;
				v0.Color.z = spr->B / 255.0f;
				v0.Color.w = 1.0f;

				RendererVertex v1;
				v1.Position.x = p1t.x;
				v1.Position.y = p1t.y;
				v1.Position.z = p1t.z;
				v1.UV.x = spr->Sprite->UV[1].x;
				v1.UV.y = spr->Sprite->UV[1].y;
				v1.Color.x = spr->R / 255.0f;
				v1.Color.y = spr->G / 255.0f;
				v1.Color.z = spr->B / 255.0f;
				v1.Color.w = 1.0f;

				RendererVertex v2;
				v2.Position.x = p2t.x;
				v2.Position.y = p2t.y;
				v2.Position.z = p2t.z;
				v2.UV.x = spr->Sprite->UV[2].x;
				v2.UV.y = spr->Sprite->UV[2].y;
				v2.Color.x = spr->R / 255.0f;
				v2.Color.y = spr->G / 255.0f;
				v2.Color.z = spr->B / 255.0f;
				v2.Color.w = 1.0f;

				RendererVertex v3;
				v3.Position.x = p3t.x;
				v3.Position.y = p3t.y;
				v3.Position.z = p3t.z;
				v3.UV.x = spr->Sprite->UV[3].x;
				v3.UV.y = spr->Sprite->UV[3].y;
				v3.Color.x = spr->R / 255.0f;
				v3.Color.y = spr->G / 255.0f;
				v3.Color.z = spr->B / 255.0f;
				v3.Color.w = 1.0f;

				m_primitiveBatch->DrawQuad(v0, v1, v2, v3);
			}
			else if (spr->Type == RENDERER_SPRITE_TYPE::SPRITE_TYPE_3D)
			{
				Vector3 p0t = Vector3(spr->X1, spr->Y1, spr->Z1);
				Vector3 p1t = Vector3(spr->X2, spr->Y2, spr->Z2);
				Vector3 p2t = Vector3(spr->X3, spr->Y3, spr->Z3);
				Vector3 p3t = Vector3(spr->X4, spr->Y4, spr->Z4);

				RendererVertex v0;
				v0.Position.x = p0t.x;
				v0.Position.y = p0t.y;
				v0.Position.z = p0t.z;
				v0.UV.x = spr->Sprite->UV[0].x;
				v0.UV.y = spr->Sprite->UV[0].y;
				v0.Color.x = spr->R / 255.0f;
				v0.Color.y = spr->G / 255.0f;
				v0.Color.z = spr->B / 255.0f;
				v0.Color.w = 1.0f;

				RendererVertex v1;
				v1.Position.x = p1t.x;
				v1.Position.y = p1t.y;
				v1.Position.z = p1t.z;
				v1.UV.x = spr->Sprite->UV[1].x;
				v1.UV.y = spr->Sprite->UV[1].y;
				v1.Color.x = spr->R / 255.0f;
				v1.Color.y = spr->G / 255.0f;
				v1.Color.z = spr->B / 255.0f;
				v1.Color.w = 1.0f;

				RendererVertex v2;
				v2.Position.x = p2t.x;
				v2.Position.y = p2t.y;
				v2.Position.z = p2t.z;
				v2.UV.x = spr->Sprite->UV[2].x;
				v2.UV.y = spr->Sprite->UV[2].y;
				v2.Color.x = spr->R / 255.0f;
				v2.Color.y = spr->G / 255.0f;
				v2.Color.z = spr->B / 255.0f;
				v2.Color.w = 1.0f;

				RendererVertex v3;
				v3.Position.x = p3t.x;
				v3.Position.y = p3t.y;
				v3.Position.z = p3t.z;
				v3.UV.x = spr->Sprite->UV[3].x;
				v3.UV.y = spr->Sprite->UV[3].y;
				v3.Color.x = spr->R / 255.0f;
				v3.Color.y = spr->G / 255.0f;
				v3.Color.z = spr->B / 255.0f;
				v3.Color.w = 1.0f;

				m_primitiveBatch->DrawQuad(v0, v1, v2, v3);
			}			
		}

		m_primitiveBatch->End();
	}

	m_context->RSSetState(m_states->CullCounterClockwise());
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	return true;
}

void Renderer11::createBillboardMatrix(Matrix* out, Vector3* particlePos, Vector3* cameraPos, float rotation)
{
	Vector3 look = *particlePos;
	look = look - *cameraPos;
	look.Normalize();

	Vector3 cameraUp = Vector3(0.0f, -1.0f, 0.0f);

	Vector3 right;
	right = cameraUp.Cross(look);
	right.Normalize();
	
	// Rotate right vector
	Matrix rightTransform = Matrix::CreateFromAxisAngle(look, rotation);
	right = Vector3::Transform(right, rightTransform);

	Vector3 up;
	up = look.Cross(right);
	up.Normalize();

	*out = Matrix::Identity;

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

void Renderer11::updateAnimatedTextures()
{
	// Update room's animated textures
	for (__int32 i = 0; i < NumberRooms; i++)
	{
		RendererRoom* room = m_rooms[i];
		if (room == NULL)
			continue;

		for (__int32 bucketIndex = 0; bucketIndex < NUM_BUCKETS; bucketIndex++)
		{
			RendererBucket* bucket = &room->AnimatedBuckets[bucketIndex];

			if (bucket->Vertices.size() == 0)
				continue;

			for (__int32 p = 0; p < bucket->Polygons.size(); p++)
			{
				RendererPolygon* polygon = &bucket->Polygons[p];
				RendererAnimatedTextureSet* set = m_animatedTextureSets[polygon->AnimatedSet];
				__int32 textureIndex = -1;
				for (__int32 j = 0; j < set->NumTextures; j++)
				{
					if (set->Textures[j]->Id == polygon->TextureId)
					{
						textureIndex = j;
						break;
					}
				}
				if (textureIndex == -1)
					continue;

				if (textureIndex == set->NumTextures - 1)
					textureIndex = 0;
				else
					textureIndex++;

				polygon->TextureId = set->Textures[textureIndex]->Id;

				for (__int32 v = 0; v < (polygon->Shape == SHAPE_RECTANGLE ? 4 : 3); v++)
				{
					bucket->Vertices[polygon->Indices[v]].UV.x = set->Textures[textureIndex]->UV[v].x;
					bucket->Vertices[polygon->Indices[v]].UV.y = set->Textures[textureIndex]->UV[v].y;
				}
			}
		}
	}

	// Update waterfalls textures
	for (__int32 i = ID_WATERFALL1; i <= ID_WATERFALLSS2; i++)
	{
		OBJECT_INFO* obj = &Objects[i];

		if (obj->loaded)
		{
			RendererObject* waterfall = m_moveableObjects[i];

			for (__int32 m = 0; m < waterfall->ObjectMeshes.size(); m++)
			{
				RendererMesh* mesh = waterfall->ObjectMeshes[m];
				RendererBucket* bucket = &mesh->Buckets[RENDERER_BUCKET_TRANSPARENT_DS];

				for (__int32 v = 0; v < bucket->Vertices.size(); v++)
				{
					RendererVertex* vertex = &bucket->Vertices[v];
					int y = vertex->UV.y * TEXTURE_ATLAS_SIZE + 64;
					y %= 128;
					vertex->UV.y = (float)y / TEXTURE_ATLAS_SIZE;
				}
			}
		}
	}
}

bool Renderer11::drawLines3D()
{
	m_context->RSSetState(m_states->CullNone());
	m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthRead(), 0);
	
	m_context->VSSetShader(m_vsSolid, NULL, 0);
	m_context->PSSetShader(m_psSolid, NULL, 0);

	m_stCameraMatrices.View = View.Transpose();
	m_stCameraMatrices.Projection = Projection.Transpose();
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_context->IASetInputLayout(m_inputLayout);

	m_primitiveBatch->Begin();

	for (__int32 i = 0; i < m_lines3DToDraw.Size(); i++)
	{
		RendererLine3D* line = m_lines3DToDraw[i];

		RendererVertex v1;
		v1.Position.x = line->X1;
		v1.Position.y = line->Y1;
		v1.Position.z = line->Z1;
		v1.Color.x = line->R / 255.0f;
		v1.Color.y = line->G / 255.0f;
		v1.Color.z = line->B / 255.0f;
		v1.Color.w = 1.0f;

		RendererVertex v2;
		v2.Position.x = line->X2;
		v2.Position.y = line->Y2;
		v2.Position.z = line->Z2;
		v2.Color.x = line->R / 255.0f;
		v2.Color.y = line->G / 255.0f;
		v2.Color.z = line->B / 255.0f;
		v2.Color.w = 1.0f;

		m_primitiveBatch->DrawLine(v1, v2);
	}

	m_primitiveBatch->End();

	m_context->RSSetState(m_states->CullCounterClockwise());
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	return true;
}

bool Renderer11::doRain()
{
	if (m_firstWeather)
	{
		for (__int32 i = 0; i < NUM_RAIN_DROPS; i++)
		{
			m_rain[i].Reset = true;
			m_rain[i].Draw = true;
		}
	}

	for (__int32 i = 0; i < NUM_RAIN_DROPS; i++)
	{
		RendererWeatherParticle* drop = &m_rain[i];

		if (drop->Reset)
		{
			drop->Draw = true;

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

		if (drop->Draw)
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

bool Renderer11::doSnow()
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
			0.0f, 1.0f, SNOW_SIZE, SNOW_SIZE,
			BLENDMODE_ALPHABLEND);

		__int16 roomNumber = Camera.pos.roomNumber;
		FLOOR_INFO* floor = GetFloor(snow->X, snow->Y, snow->Z, &roomNumber);
		ROOM_INFO* room = &Rooms[roomNumber];
		if (snow->Y >= room->y + room->minfloor)
			snow->Reset = true;
	}

	m_firstWeather = false;

	return true;
}

bool Renderer11::drawDebris(bool transparent)
{
	UINT cPasses = 1;

	// First collect debrises
	vector<RendererVertex> vertices;

	for (__int32 i = 0; i < NUM_DEBRIS; i++)
	{
		DEBRIS_STRUCT* debris = &Debris[i];

		if (debris->On)
		{
			Matrix translation = Matrix::CreateTranslation(debris->x, debris->y, debris->z);
			Matrix rotation = Matrix::CreateFromYawPitchRoll(TR_ANGLE_TO_RAD(debris->YRot), TR_ANGLE_TO_RAD(debris->XRot), 0);
			Matrix world = rotation * translation;

			OBJECT_TEXTURE* texture = &ObjectTextures[(__int32)(debris->textInfo) & 0x7FFF];
			__int32 tile = texture->tileAndFlag & 0x7FFF;

			// Draw only debris of the current bucket
			if (texture->attribute == 0 && transparent
				||
				texture->attribute == 1 && transparent
				||
				texture->attribute == 2 && !transparent
				)
				continue;

			RendererVertex vertex;

			// Prepare the triangle
			Vector3 p = Vector3(debris->XYZOffsets1[0], debris->XYZOffsets1[1], debris->XYZOffsets1[2]);
			p = Vector3::Transform(p, world);
			vertex.Position.x = p.x;
			vertex.Position.y = p.y;
			vertex.Position.z = p.z;
			vertex.UV.x = (texture->vertices[0].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
			vertex.UV.y = (texture->vertices[0].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;
			vertex.Color.x = debris->Pad[2] / 255.0f;
			vertex.Color.y = debris->Pad[3] / 255.0f;
			vertex.Color.z = debris->Pad[4] / 255.0f;
			vertices.push_back(vertex);

			p = Vector3(debris->XYZOffsets2[0], debris->XYZOffsets2[1], debris->XYZOffsets2[2]);
			p = Vector3::Transform(p, world);
			vertex.Position.x = p.x;
			vertex.Position.y = p.y;
			vertex.Position.z = p.z;
			vertex.UV.x = (texture->vertices[1].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
			vertex.UV.y = (texture->vertices[1].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;
			vertex.Color.x = debris->Pad[6] / 255.0f;
			vertex.Color.y = debris->Pad[7] / 255.0f;
			vertex.Color.z = debris->Pad[8] / 255.0f;
			vertices.push_back(vertex);

			p = Vector3(debris->XYZOffsets3[0], debris->XYZOffsets3[1], debris->XYZOffsets3[2]);
			p = Vector3::Transform(p, world); 
			vertex.Position.x = p.x;
			vertex.Position.y = p.y;
			vertex.Position.z = p.z;
			vertex.UV.x = (texture->vertices[2].x * 256.0f + 0.5f + GET_ATLAS_PAGE_X(tile)) / (float)TEXTURE_ATLAS_SIZE;
			vertex.UV.y = (texture->vertices[2].y * 256.0f + 0.5f + GET_ATLAS_PAGE_Y(tile)) / (float)TEXTURE_ATLAS_SIZE;
			vertex.Color.x = debris->Pad[10] / 255.0f;
			vertex.Color.y = debris->Pad[11] / 255.0f;
			vertex.Color.z = debris->Pad[12] / 255.0f;
			vertices.push_back(vertex);
		}
	}

	// Check if no debris have to be drawn
	if (vertices.size() == 0)
		return true;

	m_primitiveBatch->Begin();

	// Set shaders
	m_context->VSSetShader(m_vsStatics, NULL, 0);
	m_context->PSSetShader(m_psStatics, NULL, 0);

	// Set texture
	m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
	ID3D11SamplerState* sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	// Set camera matrices
	m_stCameraMatrices.View = View.Transpose();
	m_stCameraMatrices.Projection = Projection.Transpose();
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	m_stMisc.AlphaTest = !transparent;
	updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
	m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

	m_stStatic.World = Matrix::Identity;
	m_stStatic.Color = Vector4::One;
	updateConstantBuffer(m_cbStatic, &m_stStatic, sizeof(CStaticBuffer));
	m_context->VSSetConstantBuffers(1, 1, &m_cbStatic);

	// Draw vertices
	m_primitiveBatch->Draw(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST, vertices.data(), vertices.size());
	m_numDrawCalls++;

	m_primitiveBatch->End();

	return true;
}

bool Renderer11::drawBats()
{
	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	m_context->IASetVertexBuffers(0, 1, &m_moveablesVertexBuffer->Buffer, &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetIndexBuffer(m_moveablesIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

	if (Objects[ID_BATS].loaded)
	{
		OBJECT_INFO* obj = &Objects[ID_BATS];
		RendererObject* moveableObj = m_moveableObjects[ID_BATS];
		__int16* meshPtr = Meshes[Objects[ID_BATS].meshIndex + 2 * (-GlobalCounter & 3)];
		RendererMesh* mesh = m_meshPointersToMesh[reinterpret_cast<unsigned int>(meshPtr)];
		
		for (__int32 m = 0; m < 32; m++)
			memcpy(&m_stItem.BonesMatrices[m], &Matrix::Identity, sizeof(Matrix));

		for (__int32 b = 0; b < 2; b++)
		{
			RendererBucket* bucket = &mesh->Buckets[b];

			if (bucket->NumVertices == 0)
				continue;

			for (__int32 i = 0; i < 64; i++)
			{
				BAT_STRUCT* bat = &Bats[i];

				if (bat->on)
				{
					Matrix translation = Matrix::CreateTranslation(bat->pos.xPos, bat->pos.yPos, bat->pos.zPos);
					Matrix rotation = Matrix::CreateFromYawPitchRoll(bat->pos.yRot, bat->pos.xRot, bat->pos.zRot);
					Matrix world = rotation * translation;

					m_stItem.World = world.Transpose();
					m_stItem.Position = Vector4(bat->pos.xPos, bat->pos.yPos, bat->pos.zPos, 1.0f);
					m_stItem.AmbientLight = m_rooms[bat->roomNumber]->AmbientLight;
					updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));

					m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
					m_numDrawCalls++;
				}
			}
		}
	}

	return true;
}

bool Renderer11::drawRats()
{
	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	m_context->IASetVertexBuffers(0, 1, &m_moveablesVertexBuffer->Buffer, &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetIndexBuffer(m_moveablesIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

	if (Objects[ID_RATS].loaded)
	{
		OBJECT_INFO* obj = &Objects[ID_BATS];
		RendererObject* moveableObj = m_moveableObjects[ID_BATS];

		for (__int32 m = 0; m < 32; m++)
			memcpy(&m_stItem.BonesMatrices[m], &Matrix::Identity, sizeof(Matrix));

		for (__int32 i = 0; i < NUM_RATS; i += 4)
		{
			RAT_STRUCT* rat = &Rats[i];

			if (rat->on)
			{
				__int16* meshPtr = Meshes[Objects[ID_BATS].meshIndex + (((i + Wibble) >> 2) & 0xE)];
				RendererMesh* mesh = m_meshPointersToMesh[reinterpret_cast<unsigned int>(meshPtr)];
				
				Matrix translation = Matrix::CreateTranslation(rat->pos.xPos, rat->pos.yPos, rat->pos.zPos);
				Matrix rotation = Matrix::CreateFromYawPitchRoll(rat->pos.yRot, rat->pos.xRot, rat->pos.zRot);
				Matrix world = rotation * translation;

				m_stItem.World = world.Transpose();
				m_stItem.Position = Vector4(rat->pos.xPos, rat->pos.yPos, rat->pos.zPos, 1.0f);
				m_stItem.AmbientLight = m_rooms[rat->roomNumber]->AmbientLight;
				updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));

				for (__int32 b = 0; b < 2; b++)
				{
					RendererBucket* bucket = &mesh->Buckets[b];

					if (bucket->NumVertices == 0)
						continue;

					m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
					m_numDrawCalls++;
				}
			}
		}
	}

	return true;
}

bool Renderer11::drawSpiders()
{
	/*XMMATRIX world;
	UINT cPasses = 1;

	if (Objects[ID_SPIDER].loaded)
	{
		OBJECT_INFO* obj = &Objects[ID_SPIDER];
		RendererObject* moveableObj = m_moveableObjects[ID_SPIDER].get();
		__int16* meshPtr = Meshes[Objects[ID_SPIDER].meshIndex + ((Wibble >> 2) & 2)];
		RendererMesh* mesh = m_meshPointersToMesh[meshPtr];
		RendererBucket* bucket = mesh->GetBucket(bucketIndex);

		if (bucket->NumVertices == 0)
			return true;

		setGpuStateForBucket(bucketIndex);

		m_device->SetStreamSource(0, bucket->GetVertexBuffer(), 0, sizeof(RendererVertex));
		m_device->SetIndices(bucket->GetIndexBuffer());

		LPD3DXEFFECT effect;
		if (pass == RENDERER_PASS_SHADOW_MAP)
			effect = m_shaderDepth->GetEffect();
		else if (pass == RENDERER_PASS_RECONSTRUCT_DEPTH)
			effect = m_shaderReconstructZBuffer->GetEffect();
		else if (pass == RENDERER_PASS_GBUFFER)
			effect = m_shaderFillGBuffer->GetEffect();
		else
			effect = m_shaderTransparent->GetEffect();

		effect->SetBool(effect->GetParameterByName(NULL, "UseSkinning"), false);
		effect->SetInt(effect->GetParameterByName(NULL, "ModelType"), MODEL_TYPE_MOVEABLE);

		if (bucketIndex == RENDERER_BUCKET_SOLID || bucketIndex == RENDERER_BUCKET_SOLID_DS)
			effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLENDMODE_OPAQUE);
		else
			effect->SetInt(effect->GetParameterByName(NULL, "BlendMode"), BLENDMODE_ALPHATEST);

		for (__int32 i = 0; i < NUM_SPIDERS; i++)
		{
			SPIDER_STRUCT* spider = &Spiders[i];

			if (spider->on)
			{
				XMMATRIXTranslation(&m_tempTranslation, spider->pos.xPos, spider->pos.yPos, spider->pos.zPos);
				XMMATRIXRotationYawPitchRoll(&m_tempRotation, spider->pos.yRot, spider->pos.xRot, spider->pos.zRot);
				XMMATRIXMultiply(&m_tempWorld, &m_tempRotation, &m_tempTranslation);
				effect->SetMatrix(effect->GetParameterByName(NULL, "World"), &m_tempWorld);

				effect->SetVector(effect->GetParameterByName(NULL, "AmbientLight"), &m_rooms[spider->roomNumber]->AmbientLight);

				for (int iPass = 0; iPass < cPasses; iPass++)
				{
					effect->BeginPass(iPass);
					effect->CommitChanges();

					drawPrimitives(D3DPT_TRIANGLELIST, 0, 0, bucket->NumVertices, 0, bucket->NumIndices / 3);

					effect->EndPass();
				}
			}
		}
	}*/

	return true;
}

__int32 Renderer11::drawInventoryScene()
{
	char stringBuffer[255];

	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = ScreenWidth;
	rect.bottom = ScreenHeight;

	m_lines2DToDraw.Clear(); 
	m_strings.clear();

	m_nextLine2D = 0; 
	 
	// Set basic render states
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	m_context->RSSetState(m_states->CullCounterClockwise());
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);

	// Bind and clear render target
	m_context->ClearRenderTargetView(m_renderTarget->RenderTargetView, Colors::Black);
	m_context->ClearDepthStencilView(m_renderTarget->DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &m_renderTarget->RenderTargetView, m_renderTarget->DepthStencilView);
	m_context->RSSetViewports(1, &m_viewport);

	// Clear the Z-Buffer after drawing the background	
	if (g_Inventory->GetType() == INV_TYPE_TITLE)
	{
		drawFullScreenQuad(m_titleScreen->ShaderResourceView, Vector3(m_fadeFactor, m_fadeFactor, m_fadeFactor), false);
	}
	else
	{
		drawFullScreenQuad(m_dumpScreenRenderTarget->ShaderResourceView, Vector3(0.2f, 0.2f, 0.2f), false);
	}

	m_context->ClearDepthStencilView(m_renderTarget->DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	// Set vertex buffer
	m_context->IASetVertexBuffers(0, 1, &m_moveablesVertexBuffer->Buffer, &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetIndexBuffer(m_moveablesIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

	// Set shaders
	m_context->VSSetShader(m_vsInventory, NULL, 0);
	m_context->PSSetShader(m_psInventory, NULL, 0);

	// Set texture
	m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
	ID3D11SamplerState* sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	InventoryRing* activeRing = g_Inventory->GetRing(g_Inventory->GetActiveRing());
	__int32 lastRing = 0;

	float cameraX = INV_CAMERA_DISTANCE * cos(g_Inventory->GetCameraTilt() * RADIAN);
	float cameraY = g_Inventory->GetCameraY() - INV_CAMERA_DISTANCE * sin(g_Inventory->GetCameraTilt() * RADIAN);
	float cameraZ = 0.0f;

	m_stCameraMatrices.View = Matrix::CreateLookAt(Vector3(cameraX, cameraY, cameraZ),
		Vector3(0.0f, g_Inventory->GetCameraY(), 0.0f), Vector3(0.0f, -1.0f, 0.0f)).Transpose();
	m_stCameraMatrices.Projection = Matrix::CreatePerspectiveFieldOfView(80.0f * RADIAN,
		g_Renderer->ScreenWidth / (float)g_Renderer->ScreenHeight, 1.0f, 200000.0f).Transpose();

	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	for (__int32 k = 0; k < NUM_INVENTORY_RINGS; k++)
	{
		InventoryRing* ring = g_Inventory->GetRing(k);
		if (ring->draw == false || ring->numObjects == 0)
			continue;

		__int16 numObjects = ring->numObjects;
		float deltaAngle = 360.0f / numObjects;
		__int32 objectIndex = 0;
		objectIndex = ring->currentObject;

		// Yellow title
		if (ring->focusState == INV_FOCUS_STATE_NONE)
			PrintString(400, 20, g_GameFlow->GetString(activeRing->titleStringIndex), PRINTSTRING_COLOR_YELLOW, PRINTSTRING_CENTER);

		for (__int32 i = 0; i < numObjects; i++)
		{
			__int16 inventoryObject = ring->objects[objectIndex].inventoryObject;
			__int16 objectNumber = g_Inventory->GetInventoryObject(ring->objects[objectIndex].inventoryObject)->objectNumber;
			
			// Calculate the inventory object position and rotation
			float currentAngle = 0.0f;
			__int16 steps = -objectIndex + ring->currentObject;
			if (steps < 0) steps += numObjects;
			currentAngle = steps * deltaAngle;
			currentAngle += ring->rotation;

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

			__int32 x = ring->distance * cos(currentAngle * RADIAN);
			__int32 y = g_Inventory->GetRing(k)->y;
			__int32 z = ring->distance * sin(currentAngle * RADIAN);

			// Prepare the object transform
			Matrix scale = Matrix::CreateScale(ring->objects[objectIndex].scale, ring->objects[objectIndex].scale, ring->objects[objectIndex].scale);
			Matrix translation = Matrix::CreateTranslation(x, y, z);
			Matrix rotation = Matrix::CreateRotationY(TR_ANGLE_TO_RAD(ring->objects[objectIndex].rotation + 16384 + g_Inventory->GetInventoryObject(inventoryObject)->rotY));
			Matrix transform = (scale * rotation) * translation;

			OBJECT_INFO* obj = &Objects[objectNumber];
			RendererObject* moveableObj = m_moveableObjects[objectNumber];

			// Build the object animation matrices
			if (ring->focusState == INV_FOCUS_STATE_FOCUSED && obj->animIndex != -1 &&
				objectIndex == ring->currentObject && k == g_Inventory->GetActiveRing())
			{
				__int16* framePtr[2];
				__int32 rate = 0;
				getFrame(obj->animIndex, ring->frameIndex, framePtr, &rate);
				updateAnimation(NULL, moveableObj, framePtr, 0, 1, 0xFFFFFFFF);
			}
			else
			{
				if (obj->animIndex != -1)
					updateAnimation(NULL, moveableObj, &Anims[obj->animIndex].framePtr, 0, 1, 0xFFFFFFFF);
			}

			for (__int32 n = 0; n < moveableObj->ObjectMeshes.size(); n++)
			{
				RendererMesh* mesh = moveableObj->ObjectMeshes[n];

				// HACK: revolver and crossbow + lasersight
				if (moveableObj->Id == ID_REVOLVER_ITEM && !g_LaraExtra.Weapons[WEAPON_REVOLVER].HasLasersight && n > 0)
					break;

				if (moveableObj->Id == ID_CROSSBOW_ITEM && !g_LaraExtra.Weapons[WEAPON_CROSSBOW].HasLasersight && n > 0)
					break;

				// Finish the world matrix
				if (obj->animIndex != -1)
					m_stItem.World = (moveableObj->AnimationTransforms[n] * transform).Transpose();
				else
					m_stItem.World = (moveableObj->BindPoseTransforms[n].Transpose() * transform).Transpose();
				m_stItem.AmbientLight = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
				updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));
				m_context->VSSetConstantBuffers(1, 1, &m_cbItem);
				m_context->PSSetConstantBuffers(1, 1, &m_cbItem);

				for (__int32 m = 0; m < NUM_BUCKETS; m++)
				{
					RendererBucket* bucket = &mesh->Buckets[m];
					if (bucket->NumVertices == 0)
						continue;

					if (m < 2)
						m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
					else
						m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);

					m_stMisc.AlphaTest = (m < 2);
					updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
					m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

					m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
				}
			}

			__int16 inventoryItem = ring->objects[objectIndex].inventoryObject;

			// Draw special stuff if needed
			if (objectIndex == ring->currentObject && k == g_Inventory->GetActiveRing())
			{
				if (g_Inventory->GetActiveRing() == INV_RING_OPTIONS)
				{
					/* **************** PASSAPORT ************* */
					if (inventoryItem == INV_OBJECT_PASSAPORT && ring->focusState == INV_FOCUS_STATE_FOCUSED)
					{
						/* **************** LOAD AND SAVE MENU ************* */
						if (ring->passportAction == INV_WHAT_PASSPORT_LOAD_GAME || ring->passportAction == INV_WHAT_PASSPORT_SAVE_GAME)
						{
							y = 44;

							for (__int32 n = 0; n < MAX_SAVEGAMES; n++)
							{
								if (!g_NewSavegameInfos[n].Present)
									PrintString(400, y, g_GameFlow->GetString(45), D3DCOLOR_ARGB(255, 255, 255, 255),
										PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == n ? PRINTSTRING_BLINK : 0));
								else
								{
									sprintf(stringBuffer, "%05d", g_NewSavegameInfos[n].Count);
									PrintString(200, y, stringBuffer, D3DCOLOR_ARGB(255, 255, 255, 255), PRINTSTRING_OUTLINE |
										(ring->selectedIndex == n ? PRINTSTRING_BLINK | PRINTSTRING_DONT_UPDATE_BLINK : 0));

									PrintString(250, y, (char*)g_NewSavegameInfos[n].LevelName.c_str(), D3DCOLOR_ARGB(255, 255, 255, 255), PRINTSTRING_OUTLINE |
										(ring->selectedIndex == n ? PRINTSTRING_BLINK | PRINTSTRING_DONT_UPDATE_BLINK : 0));

									sprintf(stringBuffer, g_GameFlow->GetString(44), g_NewSavegameInfos[n].Days, g_NewSavegameInfos[n].Hours, g_NewSavegameInfos[n].Minutes, g_NewSavegameInfos[n].Seconds);
									PrintString(475, y, stringBuffer, D3DCOLOR_ARGB(255, 255, 255, 255),
										PRINTSTRING_OUTLINE | (ring->selectedIndex == n ? PRINTSTRING_BLINK : 0));
								}

								y += 24;
							}

							drawColoredQuad(180, 24, 440, y + 20 - 24, Vector4(0.0f, 0.0f, 0.25f, 0.5f));
						}
						/* **************** SELECT LEVEL ************* */
						else if (ring->passportAction == INV_WHAT_PASSPORT_SELECT_LEVEL)
						{
							drawColoredQuad(200, 24, 400, 24 * (g_GameFlow->GetNumLevels() - 1) + 40, Vector4(0.0f, 0.0f, 0.25f, 0.5f));

							__int16 lastY = 50;

							for (__int32 n = 1; n < g_GameFlow->GetNumLevels(); n++)
							{
								GameScriptLevel* levelScript = g_GameFlow->GetLevel(n);
								PrintString(400, lastY, g_GameFlow->GetString(levelScript->NameStringIndex), D3DCOLOR_ARGB(255, 255, 255, 255),
									PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == n - 1 ? PRINTSTRING_BLINK : 0));

								lastY += 24;
							}
						}
						char* string = (char*)"";
						switch (ring->passportAction)
						{
						case INV_WHAT_PASSPORT_NEW_GAME:
							string = g_GameFlow->GetString(STRING_NEW_GAME);
							break;
						case INV_WHAT_PASSPORT_SELECT_LEVEL:
							string = g_GameFlow->GetString(STRING_SELECT_LEVEL);
							break;
						case INV_WHAT_PASSPORT_LOAD_GAME:
							string = g_GameFlow->GetString(STRING_LOAD_GAME);
							break;
						case INV_WHAT_PASSPORT_SAVE_GAME:
							string = g_GameFlow->GetString(STRING_SAVE_GAME);
							break;
						case INV_WHAT_PASSPORT_EXIT_GAME:
							string = g_GameFlow->GetString(STRING_EXIT_GAME);
							break;
						case INV_WHAT_PASSPORT_EXIT_TO_TITLE:
							string = g_GameFlow->GetString(STRING_EXIT_TO_TITLE);
							break;
						}

						PrintString(400, 550, string, PRINTSTRING_COLOR_ORANGE, PRINTSTRING_CENTER | PRINTSTRING_OUTLINE);
					}
					/* **************** GRAPHICS SETTINGS ************* */
					else if (inventoryItem == INV_OBJECT_SUNGLASSES && ring->focusState == INV_FOCUS_STATE_FOCUSED)
					{
						// Draw settings menu
						RendererVideoAdapter* adapter = &m_adapters[g_Configuration.Adapter];

						__int32 y = 200;

						PrintString(400, y, g_GameFlow->GetString(STRING_DISPLAY),
							PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);
						
						y += 25;

						// Screen resolution
						PrintString(200, y, g_GameFlow->GetString(STRING_SCREEN_RESOLUTION),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 0 ? PRINTSTRING_BLINK : 0));

						RendererDisplayMode* mode = &adapter->DisplayModes[ring->SelectedVideoMode];
						char buffer[255];
						ZeroMemory(buffer, 255);
						sprintf(buffer, "%d x %d (%d Hz)", mode->Width, mode->Height, mode->RefreshRate);

						PrintString(400, y, buffer, PRINTSTRING_COLOR_WHITE,
							PRINTSTRING_OUTLINE | (ring->selectedIndex == 0 ? PRINTSTRING_BLINK : 0));

						y += 25;

						// Windowed mode
						PrintString(200, y, g_GameFlow->GetString(STRING_WINDOWED),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 1 ? PRINTSTRING_BLINK : 0));
						PrintString(400, y, g_GameFlow->GetString(ring->Configuration.Windowed ? STRING_ENABLED : STRING_DISABLED),
							PRINTSTRING_COLOR_WHITE,
							PRINTSTRING_OUTLINE | (ring->selectedIndex == 1 ? PRINTSTRING_BLINK : 0));

						y += 25;

						// Enable dynamic shadows
						PrintString(200, y, g_GameFlow->GetString(STRING_SHADOWS),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 2 ? PRINTSTRING_BLINK : 0));
						PrintString(400, y, g_GameFlow->GetString(ring->Configuration.EnableShadows ? STRING_ENABLED : STRING_DISABLED),
							PRINTSTRING_COLOR_WHITE,
							PRINTSTRING_OUTLINE | (ring->selectedIndex == 2 ? PRINTSTRING_BLINK : 0));

						y += 25;

						// Enable caustics
						PrintString(200, y, g_GameFlow->GetString(STRING_CAUSTICS),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 3 ? PRINTSTRING_BLINK : 0));
						PrintString(400, y, g_GameFlow->GetString(ring->Configuration.EnableCaustics ? STRING_ENABLED : STRING_DISABLED),
							PRINTSTRING_COLOR_WHITE,
							PRINTSTRING_OUTLINE | (ring->selectedIndex == 3 ? PRINTSTRING_BLINK : 0));

						y += 25;

						// Enable volumetric fog
						PrintString(200, y, g_GameFlow->GetString(STRING_VOLUMETRIC_FOG),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 4 ? PRINTSTRING_BLINK : 0));
						PrintString(400, y, g_GameFlow->GetString(ring->Configuration.EnableVolumetricFog ? STRING_ENABLED : STRING_DISABLED),
							PRINTSTRING_COLOR_WHITE,
							PRINTSTRING_OUTLINE | (ring->selectedIndex == 4 ? PRINTSTRING_BLINK : 0));

						y += 25;

						// Apply and cancel
						PrintString(400, y, g_GameFlow->GetString(STRING_APPLY),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == 5 ? PRINTSTRING_BLINK : 0));
						
						y += 25;
						
						PrintString(400, y, g_GameFlow->GetString(STRING_CANCEL),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == 6 ? PRINTSTRING_BLINK : 0));

						y += 25;

						drawColoredQuad(180, 180, 440, y + 20 - 180, Vector4(0.0f, 0.0f, 0.25f, 0.5f));
					}
					/* **************** AUDIO SETTINGS ************* */
					else if (inventoryItem == INV_OBJECT_HEADPHONES && ring->focusState == INV_FOCUS_STATE_FOCUSED)
					{
						// Draw sound menu

						y = 200;

						PrintString(400, y, g_GameFlow->GetString(STRING_SOUND),
							PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);

						y += 25;

						// Enable sound
						PrintString(200, y, g_GameFlow->GetString(STRING_ENABLE_SOUND),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 0 ? PRINTSTRING_BLINK : 0));
						PrintString(400, y, g_GameFlow->GetString(ring->Configuration.EnableSound ? STRING_ENABLED : STRING_DISABLED),
							PRINTSTRING_COLOR_WHITE,
							PRINTSTRING_OUTLINE | (ring->selectedIndex == 0 ? PRINTSTRING_BLINK : 0));

						y += 25;
						
						// Enable sound special effects
						PrintString(200, y, g_GameFlow->GetString(STRING_SPECIAL_SOUND_FX),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_DONT_UPDATE_BLINK | PRINTSTRING_OUTLINE | (ring->selectedIndex == 1 ? PRINTSTRING_BLINK : 0));
						PrintString(400, y, g_GameFlow->GetString(ring->Configuration.EnableAudioSpecialEffects ? STRING_ENABLED : STRING_DISABLED),
							PRINTSTRING_COLOR_WHITE,
							PRINTSTRING_OUTLINE | (ring->selectedIndex == 1 ? PRINTSTRING_BLINK : 0));

						y += 25;

						// Music volume
						PrintString(200, y, g_GameFlow->GetString(STRING_MUSIC_VOLUME),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_OUTLINE | (ring->selectedIndex == 2 ? PRINTSTRING_BLINK : 0));
						drawBar(400, y, 150, 12, ring->Configuration.MusicVolume, 0x0000FF, 0x0000FF);

						y += 25;

						// Sound FX volume
						PrintString(200, y, g_GameFlow->GetString(STRING_SFX_VOLUME),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_OUTLINE | (ring->selectedIndex == 3 ? PRINTSTRING_BLINK : 0));
						drawBar(400, y, 150, 12, ring->Configuration.SfxVolume, 0x0000FF, 0x0000FF);

						y += 25;

						// Apply and cancel
						PrintString(400, y, g_GameFlow->GetString(STRING_APPLY),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == 4 ? PRINTSTRING_BLINK : 0));

						y += 25;

						PrintString(400, y, g_GameFlow->GetString(STRING_CANCEL),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == 5 ? PRINTSTRING_BLINK : 0));

						y += 25;

						drawColoredQuad(180, 180, 440, y + 20 - 180, Vector4(0.0f, 0.0f, 0.25f, 0.5f));
					}
					/* **************** CONTROLS SETTINGS ************* */
					else if (inventoryItem == INV_OBJECT_KEYS && ring->focusState == INV_FOCUS_STATE_FOCUSED)
					{
						// Draw sound menu
						y = 40;

						PrintString(400, y, g_GameFlow->GetString(STRING_CONTROLS),
							PRINTSTRING_COLOR_YELLOW, PRINTSTRING_OUTLINE | PRINTSTRING_CENTER);

						y += 25;

						for (__int32 k = 0; k < 18; k++)
						{
							PrintString(200, y, g_GameFlow->GetString(STRING_CONTROLS_MOVE_FORWARD + k),
								PRINTSTRING_COLOR_WHITE,
								PRINTSTRING_OUTLINE | (ring->selectedIndex == k ? PRINTSTRING_BLINK : 0) |
								(ring->waitingForKey ? PRINTSTRING_DONT_UPDATE_BLINK : 0));

							if (ring->waitingForKey && k == ring->selectedIndex)
							{
								PrintString(400, y, g_GameFlow->GetString(STRING_WAITING_FOR_KEY),
									PRINTSTRING_COLOR_YELLOW,
									PRINTSTRING_OUTLINE | PRINTSTRING_BLINK);
							}
							else
							{
								PrintString(400, y, g_KeyNames[KeyboardLayout1[k]],
									PRINTSTRING_COLOR_ORANGE,
									PRINTSTRING_OUTLINE);
							}

							y += 25;
						}

						// Apply and cancel
						PrintString(400, y, g_GameFlow->GetString(STRING_APPLY),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == NUM_CONTROLS + 0 ? PRINTSTRING_BLINK : 0));

						y += 25;

						PrintString(400, y, g_GameFlow->GetString(STRING_CANCEL),
							PRINTSTRING_COLOR_ORANGE,
							PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == NUM_CONTROLS + 1 ? PRINTSTRING_BLINK : 0));

						y += 25;

						drawColoredQuad(180, 20, 440, y + 20 - 20, Vector4(0.0f, 0.0f, 0.25f, 0.5f));
					}
					else
					{
						// Draw the description below the object
						char* string = g_GameFlow->GetString(g_Inventory->GetInventoryObject(inventoryItem)->objectName); // (char*)g_NewStrings[g_Inventory->GetInventoryObject(inventoryItem)->objectName].c_str(); // &AllStrings[AllStringsOffsets[g_Inventory->GetInventoryObject(inventoryItem)->objectName]];
						PrintString(400, 550, string, PRINTSTRING_COLOR_ORANGE, PRINTSTRING_CENTER | PRINTSTRING_OUTLINE);
					}
				}
				else
				{
					__int16 inventoryItem = g_Inventory->GetRing(k)->objects[objectIndex].inventoryObject;
					char* string = g_GameFlow->GetString(g_Inventory->GetInventoryObject(inventoryItem)->objectName); // &AllStrings[AllStringsOffsets[InventoryObjectsList[inventoryItem].objectName]];

					if (g_Inventory->IsCurrentObjectWeapon() && ring->focusState == INV_FOCUS_STATE_FOCUSED)
					{
						y = 100;

						for (__int32 a = 0; a < ring->numActions; a++)
						{
							__int32 stringIndex = 0;
							if (ring->actions[a] == INV_ACTION_USE) stringIndex = STRING_USE;
							if (ring->actions[a] == INV_ACTION_COMBINE) stringIndex = STRING_COMBINE;
							if (ring->actions[a] == INV_ACTION_SEPARE) stringIndex = STRING_SEPARE;
							if (ring->actions[a] == INV_ACTION_SELECT_AMMO) stringIndex = STRING_CHOOSE_AMMO;

							// Apply and cancel
							PrintString(400, y, g_GameFlow->GetString(stringIndex),
								PRINTSTRING_COLOR_WHITE,
								PRINTSTRING_CENTER | PRINTSTRING_OUTLINE | (ring->selectedIndex == a ? PRINTSTRING_BLINK : 0));

							y += 25;
						}
						
						drawColoredQuad(300, 80, 200, y + 20 - 80, Vector4(0.0f, 0.0f, 0.25f, 0.5f));
					}

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
						quantity = g_LaraExtra.Weapons[WEAPON_SHOTGUN].Ammo[0];
						if (quantity != -1)
							quantity /= 6;
						break;
					case ID_SHOTGUN_AMMO2_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_SHOTGUN].Ammo[1];
						if (quantity != -1)
							quantity /= 6;
						break;
					case ID_HK_AMMO_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_HK].Ammo[0];
						break;
					case ID_CROSSBOW_AMMO1_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[0];
						break;
					case ID_CROSSBOW_AMMO2_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[1];
						break;
					case ID_CROSSBOW_AMMO3_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_CROSSBOW].Ammo[2];
						break;
					case ID_REVOLVER_AMMO_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_REVOLVER].Ammo[0];
						break;
					case ID_UZI_AMMO_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_UZI].Ammo[0];
						break;
					case ID_PISTOLS_AMMO_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_PISTOLS].Ammo[0];
						break;
					case ID_GRENADE_AMMO1_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[0];
						break;
					case ID_GRENADE_AMMO2_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[1];
						break;
					case ID_GRENADE_AMMO3_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_GRENADE_LAUNCHER].Ammo[2];
						break;
					case ID_HARPOON_AMMO_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_HARPOON_GUN].Ammo[0];
						break;
					case ID_ROCKET_LAUNCHER_AMMO_ITEM:
						quantity = g_LaraExtra.Weapons[WEAPON_ROCKET_LAUNCHER].Ammo[0];
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


	drawLines2D();   
	drawAllStrings();
	
	return 0;
}

bool Renderer11::drawColoredQuad(__int32 x, __int32 y, __int32 w, __int32 h, Vector4 color)
{
	//return true;

	float factorW = ScreenWidth / 800.0f;
	float factorH = ScreenHeight / 600.0f;

	RECT rect;
	rect.top = y * factorH;
	rect.left = x * factorW;
	rect.bottom = (y + h) * factorH;
	rect.right = (x + w) * factorW;

	m_spriteBatch->Begin(SpriteSortMode_Immediate);
	m_spriteBatch->Draw(m_whiteTexture->ShaderResourceView, rect, color);
	m_spriteBatch->End();

	int shiftW = 4 * factorW;
	int shiftH = 4 * factorH;

	insertLine2D(rect.left + shiftW, rect.top + shiftH, rect.right - shiftW, rect.top + shiftH, 128, 128, 128, 128);
	insertLine2D(rect.right - shiftW, rect.top + shiftH, rect.right - shiftW, rect.bottom - shiftH, 128, 128, 128, 128);
	insertLine2D(rect.left + shiftW, rect.bottom - shiftH, rect.right - shiftW, rect.bottom - shiftH, 128, 128, 128, 128);
	insertLine2D(rect.left + shiftW, rect.top + shiftH, rect.left + shiftW, rect.bottom - shiftH, 128, 128, 128, 128);

	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	return true;
}

bool Renderer11::drawFullScreenQuad(ID3D11ShaderResourceView* texture, Vector3 color, bool cinematicBars)
{
	RendererVertex vertices[4];
	 
	if (!cinematicBars)
	{
		vertices[0].Position.x = -1.0f;
		vertices[0].Position.y = 1.0f;
		vertices[0].Position.z = 0.0f;
		vertices[0].UV.x = 0.0f;
		vertices[0].UV.y = 0.0f;
		vertices[0].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[1].Position.x = 1.0f;
		vertices[1].Position.y = 1.0f;
		vertices[1].Position.z = 0.0f;
		vertices[1].UV.x = 1.0f;
		vertices[1].UV.y = 0.0f;
		vertices[1].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[2].Position.x = 1.0f;
		vertices[2].Position.y = -1.0f;
		vertices[2].Position.z = 0.0f;
		vertices[2].UV.x = 1.0f;
		vertices[2].UV.y = 1.0f;
		vertices[2].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[3].Position.x = -1.0f;
		vertices[3].Position.y = -1.0f;
		vertices[3].Position.z = 0.0f;
		vertices[3].UV.x = 0.0f;
		vertices[3].UV.y = 1.0f;
		vertices[3].Color = Vector4(color.x, color.y, color.z, 1.0f);
	}
	else
	{
		float cinematicFactor = 0.12f;

		vertices[0].Position.x = -1.0f;
		vertices[0].Position.y = 1.0f - cinematicFactor * 2;
		vertices[0].Position.z = 0.0f;
		vertices[0].UV.x = 0.0f;
		vertices[0].UV.y = cinematicFactor;
		vertices[0].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[1].Position.x = 1.0f;
		vertices[1].Position.y = 1.0f - cinematicFactor * 2;
		vertices[1].Position.z = 0.0f;
		vertices[1].UV.x = 1.0f;
		vertices[1].UV.y = cinematicFactor;
		vertices[1].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[2].Position.x = 1.0f;
		vertices[2].Position.y = -(1.0f - cinematicFactor * 2);
		vertices[2].Position.z = 0.0f;
		vertices[2].UV.x = 1.0f;
		vertices[2].UV.y = 1.0f - cinematicFactor;
		vertices[2].Color = Vector4(color.x, color.y, color.z, 1.0f);

		vertices[3].Position.x = -1.0f;
		vertices[3].Position.y = -(1.0f - cinematicFactor * 2);
		vertices[3].Position.z = 0.0f;
		vertices[3].UV.x = 0.0f;
		vertices[3].UV.y = 1.0f - cinematicFactor;
		vertices[3].Color = Vector4(color.x, color.y, color.z, 1.0f);
	}

	m_context->VSSetShader(m_vsFullScreenQuad, NULL, 0);
	m_context->PSSetShader(m_psFullScreenQuad, NULL, 0);

	m_context->PSSetShaderResources(0, 1, &texture);
	ID3D11SamplerState* sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);

	m_primitiveBatch->Begin();
	m_primitiveBatch->DrawQuad(vertices[0], vertices[1], vertices[2], vertices[3]);
	m_primitiveBatch->End();

	return true;
}

bool Renderer11::drawRopes()
{
	for (__int32 n = 0; n < NumRopes; n++)
	{
		ROPE_STRUCT* rope = &Ropes[n];

		if (rope->active)
		{
			// Original algorithm:
			// 1) Transform segment coordinates from 3D to 2D + depth
			// 2) Get dx, dy and the segment length
			// 3) Get sine and cosine from dx / length and dy / length
			// 4) Calculate a scale factor 
			// 5) Get the coordinates of the 4 corners of each sprite iteratively
			// 6) Last step only for us, unproject back to 3D coordinates

			// Tranform rope points
			Vector3 projected[24];
			Matrix world = Matrix::Identity;

			for (__int32 i = 0; i < 24; i++)
			{
				Vector3 absolutePosition = Vector3(rope->position.x + rope->segment[i].x / 65536.0f,
					rope->position.y + rope->segment[i].y / 65536.0f,
					rope->position.z + rope->segment[i].z / 65536.0f);

				projected[i] = m_viewportToolkit->Project(absolutePosition, Projection, View, world);
			}

			// Now each rope point is transformed in screen X, Y and Z depth
			// Let's calculate dx, dy corrections and scaling
			float dx = projected[1].x - projected[0].x;
			float dy = projected[1].y - projected[0].y;
			float length = sqrt(dx * dx + dy * dy);
			float s = 0;
			float c = 0;

			if (length != 0)
			{
				s = -dy / length;
				c = dx / length;
			}

			float w = 6.0f;
			if (projected[0].z)
			{
				w = 6.0f * PhdPerspective / projected[0].z / 65536.0f;
				if (w < 3)
					w = 3;
			}

			float sdx = s * w;
			float sdy = c * w;

			float x1 = projected[0].x - sdx;
			float y1 = projected[0].y - sdy;

			float x2 = projected[0].x + sdx;
			float y2 = projected[0].y + sdy;

			float depth = projected[0].z;

			for (__int32 j = 0; j < 24; j++)
			{
				Vector3 p1 = m_viewportToolkit->Unproject(Vector3(x1, y1, depth), Projection, View, world);
				Vector3 p2 = m_viewportToolkit->Unproject(Vector3(x2, y2, depth), Projection, View, world);
				
				dx = projected[j].x - projected[j - 1].x;
				dy = projected[j].y - projected[j - 1].y;
				length = sqrt(dx * dx + dy * dy);
				s = 0;
				c = 0;

				if (length != 0)
				{
					s = -dy / length;
					c = dx / length;
				}

				w = 6.0f;
				if (projected[j].z)
				{
					w = 6.0f * PhdPerspective / projected[j].z / 65536.0f;
					if (w < 3)
						w = 3;
				}

				float sdx = s * w;
				float sdy = c * w;

				float x3 = projected[j].x - sdx;
				float y3 = projected[j].y - sdy;

				float x4 = projected[j].x + sdx;
				float y4 = projected[j].y + sdy;

				depth = projected[j].z;

				Vector3 p3 = m_viewportToolkit->Unproject(Vector3(x3, y3, depth), Projection, View, world);
				Vector3 p4 = m_viewportToolkit->Unproject(Vector3(x4, y4, depth), Projection, View, world);

				addSprite3D(m_sprites[20],
					p1.x, p1.y, p1.z,
					p2.x, p2.y, p2.z,
					p3.x, p3.y, p3.z,
					p4.x, p4.y, p4.z,
					128, 128, 128, 0, 1, 0, 0, BLENDMODE_OPAQUE);

				x1 = x4;
				y1 = y4;
				x2 = x3;
				y2 = y3;
			}

		}
	}

	return true;
}

bool Renderer11::drawLines2D()
{
	m_context->RSSetState(m_states->CullNone());
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthRead(), 0);

	m_context->VSSetShader(m_vsSolid, NULL, 0);
	m_context->PSSetShader(m_psSolid, NULL, 0);

	Matrix world = Matrix::CreateOrthographicOffCenter(0, ScreenWidth, ScreenHeight, 0, m_viewport.MinDepth, m_viewport.MaxDepth);

	m_stCameraMatrices.View = Matrix::Identity;
	m_stCameraMatrices.Projection = Matrix::Identity;
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	m_context->IASetInputLayout(m_inputLayout);

	m_primitiveBatch->Begin();

	for (__int32 i = 0; i < m_lines2DToDraw.Size(); i++)
	{
		RendererLine2D* line = m_lines2DToDraw[i];

		RendererVertex v1;
		v1.Position.x = line->Vertices[0].x;
		v1.Position.y = line->Vertices[0].y;
		v1.Position.z = 1.0f;
		v1.Color.x = line->Color.x / 255.0f;
		v1.Color.y = line->Color.y / 255.0f;
		v1.Color.z = line->Color.z / 255.0f;
		v1.Color.w = line->Color.w / 255.0f;

		RendererVertex v2;
		v2.Position.x = line->Vertices[1].x;
		v2.Position.y = line->Vertices[1].y;
		v2.Position.z = 1.0f;
		v2.Color.x = line->Color.x / 255.0f;
		v2.Color.y = line->Color.y / 255.0f;
		v2.Color.z = line->Color.z / 255.0f;
		v2.Color.w = line->Color.w / 255.0f;

		v1.Position = Vector3::Transform(v1.Position, world);
		v2.Position = Vector3::Transform(v2.Position, world);

		v1.Position.z = 0.5f;
		v2.Position.z = 0.5f;

		m_primitiveBatch->DrawLine(v1, v2);
	}

	m_primitiveBatch->End();

	m_context->RSSetState(m_states->CullCounterClockwise());
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	return true;
}

bool Renderer11::drawOverlays()
{
	if (!BinocularRange && !SpotcamOverlay)
		return true;

	m_context->OMSetBlendState(m_states->AlphaBlend(), NULL, 0xFFFFFFFF);
	drawFullScreenQuad(m_binocularsTexture->ShaderResourceView, Vector3::One, false);
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);

	return true;
}

bool Renderer11::drawBar(__int32 x, __int32 y, __int32 w, __int32 h, __int32 percent, __int32 color1, __int32 color2)
{
	byte r1 = (color1 >> 16) & 0xFF;
	byte g1 = (color1 >> 8) & 0xFF;
	byte b1 = (color1 >> 0) & 0xFF;

	byte r2 = (color2 >> 16) & 0xFF;
	byte g2 = (color2 >> 8) & 0xFF;
	byte b2 = (color2 >> 0) & 0xFF;

	float factorX = ScreenWidth / 800.0f;
	float factorY = ScreenHeight / 600.0f;

	__int32 realX = x * factorX;
	__int32 realY = y * factorY;
	__int32 realW = w * factorX;
	__int32 realH = h * factorY;

	__int32 realPercent = percent / 100.0f * realW;

	for (__int32 i = 0; i < realH; i++)
		insertLine2D(realX, realY + i, realX + realW, realY + i, 0, 0, 0, 255);

	for (__int32 i = 0; i < realH; i++)
		insertLine2D(realX, realY + i, realX + realPercent, realY + i, r1, g1, b1, 255);

	insertLine2D(realX, realY, realX + realW, realY, 255, 255, 255, 255);
	insertLine2D(realX, realY + realH, realX + realW, realY + realH, 255, 255, 255, 255);
	insertLine2D(realX, realY, realX, realY + realH, 255, 255, 255, 255);
	insertLine2D(realX + realW, realY, realX + realW, realY + realH + 1, 255, 255, 255, 255);

	return true;
}

void Renderer11::insertLine2D(__int32 x1, __int32 y1, __int32 x2, __int32 y2, byte r, byte g, byte b, byte a)
{
	RendererLine2D* line = &m_lines2DBuffer[m_nextLine2D++];

	line->Vertices[0] = Vector2(x1, y1);
	line->Vertices[1] = Vector2(x2, y2);
	line->Color = Vector4(r, g, b, a);

	m_lines2DToDraw.Add(line);
}

bool Renderer11::drawGunFlashes()
{
	if (!Lara.rightArm.flash_gun && !Lara.leftArm.flash_gun)
		return true;

	Matrix world;
	Matrix translation;
	Matrix rotation;
	
	RendererObject* laraObj = m_moveableObjects[ID_LARA];
	RendererObject* laraSkin = m_moveableObjects[ID_LARA_SKIN];

	OBJECT_INFO* obj = &Objects[0];
	RendererRoom* room = m_rooms[LaraItem->roomNumber];
	RendererItem* item = &m_items[Lara.itemNumber];

	m_stItem.AmbientLight = room->AmbientLight;
	memcpy(m_stItem.BonesMatrices, &Matrix::Identity, sizeof(Matrix));

	m_stLights.NumLights = item->Lights.Size();
	for (__int32 j = 0; j < item->Lights.Size(); j++)
		memcpy(&m_stLights.Lights[j], item->Lights[j], sizeof(ShaderLight));
	updateConstantBuffer(m_cbLights, &m_stLights, sizeof(CLightBuffer));
	m_context->PSSetConstantBuffers(2, 1, &m_cbLights);

	m_stMisc.AlphaTest = true;
	updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
	m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

	__int16 length = 0;
	__int16 zOffset = 0;
	__int16 rotationX = 0;

	m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthNone(), 0);

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
			RendererBucket* flashBucket = &flashMesh->Buckets[b];

			if (flashBucket->NumVertices != 0)
			{
				Matrix offset = Matrix::CreateTranslation(0, length, zOffset);
				Matrix rotation2 = Matrix::CreateRotationX(TR_ANGLE_TO_RAD(rotationX));

				if (Lara.leftArm.flash_gun)
				{
					world = laraObj->AnimationTransforms[HAND_L].Transpose() * m_LaraWorldMatrix;
					world = offset * world;
					world = rotation2 * world;

					m_stItem.World = world.Transpose();
					updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));
					m_context->VSSetConstantBuffers(1, 1, &m_cbItem);

					m_context->DrawIndexed(flashBucket->NumIndices, flashBucket->StartIndex, 0);
					m_numDrawCalls++;
				}

				if (Lara.rightArm.flash_gun)
				{
					world = laraObj->AnimationTransforms[HAND_R].Transpose() * m_LaraWorldMatrix;
					world = offset * world;
					world = rotation2 * world;

					m_stItem.World = world.Transpose();
					updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));
					m_context->VSSetConstantBuffers(1, 1, &m_cbItem);

					m_context->DrawIndexed(flashBucket->NumIndices, flashBucket->StartIndex, 0);
					m_numDrawCalls++;
				}
			}
		}
	}

	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	return true;
}

bool Renderer11::drawGunShells()
{
	RendererRoom* room = m_rooms[LaraItem->roomNumber];
	RendererItem* item = &m_items[Lara.itemNumber];

	m_stItem.AmbientLight = room->AmbientLight;
	memcpy(m_stItem.BonesMatrices, &Matrix::Identity, sizeof(Matrix));

	m_stLights.NumLights = item->Lights.Size();
	for (__int32 j = 0; j < item->Lights.Size(); j++)
		memcpy(&m_stLights.Lights[j], item->Lights[j], sizeof(ShaderLight));
	updateConstantBuffer(m_cbLights, &m_stLights, sizeof(CLightBuffer));
	m_context->PSSetConstantBuffers(2, 1, &m_cbLights);

	m_stMisc.AlphaTest = true;
	updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
	m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

	for (__int32 i = 0; i < 24; i++)
	{
		GUNSHELL_STRUCT* gunshell = &GunShells[i];

		if (gunshell->counter > 0)
		{
			OBJECT_INFO* obj = &Objects[gunshell->objectNumber];
			RendererObject* moveableObj = m_moveableObjects[gunshell->objectNumber];

			Matrix translation = Matrix::CreateTranslation(gunshell->pos.xPos, gunshell->pos.yPos, gunshell->pos.zPos);
			Matrix rotation = Matrix::CreateFromYawPitchRoll(TR_ANGLE_TO_RAD(gunshell->pos.yRot),
				TR_ANGLE_TO_RAD(gunshell->pos.xRot),
				TR_ANGLE_TO_RAD(gunshell->pos.zRot));
			Matrix world = rotation * translation;

			m_stItem.World = world.Transpose();
			updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));
			m_context->VSSetConstantBuffers(1, 1, &m_cbItem);

			RendererMesh* mesh = moveableObj->ObjectMeshes[0];

			for (__int32 b = 0; b < NUM_BUCKETS; b++)
			{
				RendererBucket* bucket = &mesh->Buckets[b];

				if (bucket->NumVertices == 0)
					continue;

				m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
				m_numDrawCalls++;
			}
		}
	}

	return true;
}

void Renderer11::drawUnderwaterDust()
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
			0.0f, 1.0f, UNDERWATER_DUST_PARTICLES_SIZE, UNDERWATER_DUST_PARTICLES_SIZE,
			BLENDMODE_ALPHABLEND);

		if (dust->Life >= 32)
			dust->Reset = true;
	}

	m_firstUnderwaterDustParticles = false;

	return;
}

bool Renderer11::isRoomUnderwater(__int16 roomNumber)
{
	return (m_rooms[roomNumber]->Room->flags & 1);
}

bool Renderer11::isInRoom(__int32 x, __int32 y, __int32 z, __int16 roomNumber)
{
	RendererRoom* room = m_rooms[roomNumber];
	ROOM_INFO* r = room->Room;

	return (x >= r->x && x <= r->x + r->xSize * 1024.0f &&
		y >= r->maxceiling && y <= r->minfloor &&
		z >= r->z && z <= r->z + r->ySize * 1024.0f);
}

vector<RendererVideoAdapter>* Renderer11::GetAdapters()
{
	return &m_adapters;
}

__int32 Renderer11::DrawPickup(__int16 objectNum)
{
	drawObjectOn2DPosition(700 + PickupX, 450, objectNum, 0, m_pickupRotation, 0);
	m_pickupRotation += 45 * 360 / 30;
	return 0;
}

bool Renderer11::drawObjectOn2DPosition(__int16 x, __int16 y, __int16 objectNum, __int16 rotX, __int16 rotY, __int16 rotZ)
{
	Matrix translation;
	Matrix rotation;
	Matrix world;
	Matrix view;
	Matrix projection;
	Matrix scale;

	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	x *= (ScreenWidth / 800.0f);
	y *= (ScreenHeight / 600.0f);
	
	view = Matrix::CreateLookAt(Vector3(0.0f, 0.0f, 2048.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, -1.0f, 0.0f));
	projection = Matrix::CreateOrthographic(ScreenWidth, ScreenHeight, -1024.0f, 1024.0f);

	OBJECT_INFO* obj = &Objects[objectNum];
	RendererObject* moveableObj = m_moveableObjects[objectNum];

	if (obj->animIndex != -1)
	{
		updateAnimation(NULL, moveableObj, &Anims[obj->animIndex].framePtr, 0, 0, 0xFFFFFFFF);
	}

	Vector3 pos = m_viewportToolkit->Unproject(Vector3(x, y, 1), projection, view, Matrix::Identity);

	// Clear just the Z-buffer so we can start drawing on top of the scene
	m_context->ClearDepthStencilView(m_currentRenderTarget->DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Set vertex buffer
	m_context->IASetVertexBuffers(0, 1, &m_moveablesVertexBuffer->Buffer, &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetIndexBuffer(m_moveablesIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

	// Set shaders
	m_context->VSSetShader(m_vsInventory, NULL, 0);
	m_context->PSSetShader(m_psInventory, NULL, 0);

	// Set texture
	m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
	ID3D11SamplerState* sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	// Set matrices
	m_stCameraMatrices.View = view.Transpose();
	m_stCameraMatrices.Projection = projection.Transpose();
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	for (__int32 n = 0; n < moveableObj->ObjectMeshes.size(); n++)
	{
		RendererMesh* mesh = moveableObj->ObjectMeshes[n];

		// Finish the world matrix
		translation = Matrix::CreateTranslation(pos.x, pos.y, pos.z + 1024.0f);
		rotation = Matrix::CreateFromYawPitchRoll(TR_ANGLE_TO_RAD(rotY), TR_ANGLE_TO_RAD(rotX), TR_ANGLE_TO_RAD(rotZ));
		scale = Matrix::CreateScale(0.5f);

		world = scale * rotation;
		world = world * translation;

		if (obj->animIndex != -1)
			m_stItem.World = (moveableObj->AnimationTransforms[n] * world).Transpose();
		else
			m_stItem.World = (moveableObj->BindPoseTransforms[n].Transpose() * world).Transpose();
		m_stItem.AmbientLight = Vector4(0.5f, 0.5f, 0.5f, 1.0f);
		updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));
		m_context->VSSetConstantBuffers(1, 1, &m_cbItem);
		m_context->PSSetConstantBuffers(1, 1, &m_cbItem);

		for (__int32 m = 0; m < NUM_BUCKETS; m++)
		{
			RendererBucket* bucket = &mesh->Buckets[m];
			if (bucket->NumVertices == 0)
				continue;

			if (m < 2)
				m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
			else
				m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);

			m_stMisc.AlphaTest = (m < 2);
			updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
			m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

			m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
		}
	}

	return true;
}

bool Renderer11::drawShadowMap()
{
	m_shadowLight = NULL;
	RendererLight* brightestLight = NULL;
	float brightest = 0.0f;
	Vector3 itemPosition = Vector3(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);

	for (__int32 k = 0; k < m_roomsToDraw.Size(); k++)
	{
		RendererRoom* room = m_roomsToDraw[k];
		__int32 numLights = room->Lights.size();

		for (__int32 j = 0; j < numLights; j++)
		{
			RendererLight* light = &room->Lights[j];

			// Check only lights different from sun
			if (light->Type == LIGHT_TYPE_SUN)
			{
				// Sun is added without checks
			}
			else if (light->Type == LIGHT_TYPE_POINT || light->Type == LIGHT_TYPE_SHADOW)
			{
				Vector3 lightPosition = Vector3(light->Position.x, light->Position.y, light->Position.z);

				float distance = (itemPosition - lightPosition).Length();

				// Collect only lights nearer than 20 sectors
				if (distance >= 20 * WALL_SIZE)
					continue;

				// Check the out radius
				if (distance > light->Out)
					continue;

				float attenuation = 1.0f - distance / light->Out;
				float intensity = max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

				if (intensity >= brightest)
				{
					brightest = intensity;
					brightestLight = light;
				}
			}
			else if (light->Type == LIGHT_TYPE_SPOT)
			{
				Vector3 lightPosition = Vector3(light->Position.x, light->Position.y, light->Position.z);

				float distance = (itemPosition - lightPosition).Length();

				// Collect only lights nearer than 20 sectors
				if (distance >= 20 * WALL_SIZE)
					continue;

				// Check the range
				if (distance > light->Range)
					continue;

				// If Lara, try to collect shadow casting light
				float attenuation = 1.0f - distance / light->Range;
				float intensity = max(0.0f, attenuation * (light->Color.x + light->Color.y + light->Color.z) / 3.0f);

				if (intensity >= brightest)
				{
					brightest = intensity;
					brightestLight = light;
				}
			}
			else
			{
				// Invalid light type
				continue;
			}
		}
	}

	m_shadowLight = brightestLight;
	  
	if (m_shadowLight == NULL)
		return true;

	// Reset GPU state
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->RSSetState(m_states->CullCounterClockwise());
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	// Bind and clear render target
	m_context->ClearRenderTargetView(m_shadowMap->RenderTargetView, Colors::White);
	m_context->ClearDepthStencilView(m_shadowMap->DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_context->OMSetRenderTargets(1, &m_shadowMap->RenderTargetView, m_shadowMap->DepthStencilView);

	m_context->RSSetViewports(1, &m_shadowMapViewport);

	//drawLara(false, true);

	Vector3 lightPos = Vector3(m_shadowLight->Position.x, m_shadowLight->Position.y, m_shadowLight->Position.z);
	Vector3 itemPos = Vector3(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
	if (lightPos == itemPos)
		return true;

	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	// Set shaders
	m_context->VSSetShader(m_vsShadowMap, NULL, 0);
	m_context->PSSetShader(m_psShadowMap, NULL, 0);
	  
	m_context->IASetVertexBuffers(0, 1, &m_moveablesVertexBuffer->Buffer, &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetIndexBuffer(m_moveablesIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

	// Set texture
	m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
	ID3D11SamplerState* sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);
	  
	// Set camera matrices
	Matrix view = Matrix::CreateLookAt(lightPos,
		itemPos,
		Vector3(0.0f, -1.0f, 0.0f));
	Matrix projection = Matrix::CreatePerspectiveFieldOfView(90.0f* RADIAN, 1.0f, 1.0f,
		m_shadowLight->Out * 1.5f);

	m_stCameraMatrices.View = view.Transpose();
	m_stCameraMatrices.Projection = projection.Transpose();
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);
	  
	m_stShadowMap.LightViewProjection = (view*projection).Transpose();

	m_stMisc.AlphaTest = true;
	updateConstantBuffer(m_cbMisc, &m_stMisc, sizeof(CMiscBuffer));
	m_context->PSSetConstantBuffers(3, 1, &m_cbMisc);

	RendererObject* laraObj = m_moveableObjects[ID_LARA];
	RendererObject* laraSkin = m_moveableObjects[ID_LARA_SKIN];
	RendererRoom* room = m_rooms[LaraItem->roomNumber];

	m_stItem.World = m_LaraWorldMatrix.Transpose();
	m_stItem.Position = Vector4(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos, 1.0f);
	m_stItem.AmbientLight = room->AmbientLight;
	memcpy(m_stItem.BonesMatrices, laraObj->AnimationTransforms.data(), sizeof(Matrix) * 32);
	updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));
	m_context->VSSetConstantBuffers(1, 1, &m_cbItem);
	m_context->PSSetConstantBuffers(1, 1, &m_cbItem);

	for (__int32 k = 0; k < laraSkin->ObjectMeshes.size(); k++)
	{
		RendererMesh* mesh = m_meshPointersToMesh[reinterpret_cast<unsigned int>(Lara.meshPtrs[k])];

		for (__int32 j = 0; j < 2; j++)
		{
			RendererBucket* bucket = &mesh->Buckets[j];

			if (bucket->Vertices.size() == 0)
				continue;

			if (j == RENDERER_BUCKET_SOLID_DS || j == RENDERER_BUCKET_TRANSPARENT_DS)
				m_context->RSSetState(m_states->CullNone());
			else
				m_context->RSSetState(m_states->CullCounterClockwise());

			// Draw vertices
			m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
			m_numDrawCalls++;
		}
	}

	if (m_moveableObjects[ID_LARA_SKIN_JOINTS] != NULL)
	{
		RendererObject* laraSkinJoints = m_moveableObjects[ID_LARA_SKIN_JOINTS];

		for (__int32 k = 0; k < laraSkinJoints->ObjectMeshes.size(); k++)
		{
			RendererMesh* mesh = laraSkinJoints->ObjectMeshes[k];

			for (__int32 j = 0; j < 2; j++)
			{
				RendererBucket* bucket = &mesh->Buckets[j];

				if (bucket->Vertices.size() == 0)
					continue;

				// Draw vertices
				m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
				m_numDrawCalls++;
			}
		}
	}

	for (__int32 k = 0; k < laraSkin->ObjectMeshes.size(); k++)
	{
		RendererMesh* mesh = laraSkin->ObjectMeshes[k];

		for (__int32 j = 0; j < NUM_BUCKETS; j++)
		{
			RendererBucket* bucket = &mesh->Buckets[j];

			if (bucket->Vertices.size() == 0)
				continue;

			// Draw vertices
			m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
			m_numDrawCalls++;
		}
	}

	// Hairs are pre-transformed
	Matrix matrices[8] = { Matrix::Identity, Matrix::Identity, Matrix::Identity, Matrix::Identity,
						   Matrix::Identity, Matrix::Identity, Matrix::Identity, Matrix::Identity };
	memcpy(m_stItem.BonesMatrices, matrices, sizeof(Matrix) * 8);
	m_stItem.World = Matrix::Identity;
	updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));

	if (m_moveableObjects[ID_HAIR] != NULL)
	{
		m_primitiveBatch->Begin();
		m_primitiveBatch->DrawIndexed(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			(const unsigned __int16*)m_hairIndices.data(), m_numHairIndices,
			m_hairVertices.data(), m_numHairVertices);
		m_primitiveBatch->End();
	}

	return true;
}

bool Renderer11::DoTitleImage()
{
	Texture2D* texture = Texture2D::LoadFromFile(m_device, (char*)"Title.png");
	if (!texture)
		return false;

	float currentFade = 0;
	while (currentFade <= 1.0f)
	{
		drawFullScreenImage(texture->ShaderResourceView, currentFade);
		SyncRenderer();
		currentFade += FADE_FACTOR;
	}

	for (__int32 i = 0; i < 30 * 1.5f; i++)
	{
		drawFullScreenImage(texture->ShaderResourceView, 1.0f);
		SyncRenderer();
	}

	currentFade = 1.0f;
	while (currentFade >= 0.0f)
	{
		drawFullScreenImage(texture->ShaderResourceView, currentFade);
		SyncRenderer();
		currentFade -= FADE_FACTOR;
	}

	delete texture;

	return true;
}