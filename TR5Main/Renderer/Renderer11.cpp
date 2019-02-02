#include "Renderer11.h"

#include "..\Game\camera.h"
#include "..\Game\draw.h"
#include "..\Game\effects.h"
#include "..\Game\effect2.h"
#include "..\Global\global.h"
#include "..\Specific\config.h"
#include "..\Scripting\GameFlowScript.h"
#include "..\Specific\roomload.h"

#include <D3Dcompiler.h>
#include <chrono> 
#include <stack>

using ns = chrono::nanoseconds;
using get_time = chrono::steady_clock;

extern GameConfiguration g_Configuration;
extern GameFlow* g_GameFlow;
extern __int32 NumTextureTiles;

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

	DX11_DELETE(m_spriteBatch);
	DX11_DELETE(m_gameFont);
	DX11_DELETE(m_states);

	FreeRendererData();
}

void Renderer11::FreeRendererData()
{
	DX11_DELETE(m_textureAtlas);
	DX11_DELETE(m_binocularsTexture);
	for (__int32 i = 0; i < NUM_CAUSTICS_TEXTURES; i++)
		DX11_DELETE(m_caustics[i]);
}

bool Renderer11::Create()
{
	return true;
}

bool Renderer11::EnumerateVideoModes()
{
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

	D3D_FEATURE_LEVEL levels[1] = { D3D_FEATURE_LEVEL_10_1 };
	D3D_FEATURE_LEVEL featureLevel;

	res = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, levels, 1, D3D11_SDK_VERSION,
		&m_device, &featureLevel, &m_context);
	if (FAILED(res))
		return false;

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = ScreenWidth;
	sd.BufferDesc.Height = ScreenHeight;
	sd.BufferDesc.RefreshRate.Numerator = refreshRate;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.Windowed = g_Configuration.Windowed;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.OutputWindow = handle;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferCount = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	IDXGIDevice* dxgiDevice = NULL;
	res = m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	if (FAILED(res))
		return false;

	IDXGIAdapter* dxgiAdapter = NULL;
	res = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
	if (FAILED(res))
		return false;

	IDXGIFactory* dxgiFactory = NULL;
	res = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
	if (FAILED(res))
		return false;

	m_swapChain = NULL;
	res = dxgiFactory->CreateSwapChain(m_device, &sd, &m_swapChain);
	if (FAILED(res))
		return false;

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
	depthStencilDesc.Width = ScreenWidth;
	depthStencilDesc.Height = ScreenHeight;
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

	m_binocularsTexture = Texture2D::LoadFromFile(m_device, "Binoculars.png");
	if (m_binocularsTexture == NULL)
		return false;

	// Initialise viewport
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.Width = ScreenWidth;
	m_viewport.Height = ScreenHeight;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;

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

	// Initialise constant buffers
	m_cbCameraMatrices = createConstantBuffer(sizeof(CCameraMatrixBuffer));
	m_cbItem = createConstantBuffer(sizeof(CItemBuffer));
	m_cbStatic = createConstantBuffer(sizeof(CStaticBuffer));
	m_cbRoomLights = createConstantBuffer(sizeof(CLightBuffer));

	// Initialise the ambient cube map
	D3D11_TEXTURE2D_DESC texDesc; 
	ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));
	texDesc.Width = AMBIENT_CUBE_MAP_SIZE; 
	texDesc.Height = AMBIENT_CUBE_MAP_SIZE; 
	texDesc.MipLevels = 0; 
	texDesc.ArraySize = 6;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0; 
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; 
	texDesc.Usage = D3D11_USAGE_DEFAULT; 
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; 
	texDesc.CPUAccessFlags = 0; 
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE; 
	
	m_ambientCubeMapTexture = NULL;
	res = m_device->CreateTexture2D(&texDesc, NULL, &m_ambientCubeMapTexture);
	if (FAILED(res))
		return false;

	D3D11_RENDER_TARGET_VIEW_DESC descRT;
	ZeroMemory(&descRT, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
	descRT.Format = DXGI_FORMAT_R8G8B8A8_UNORM; 
	descRT.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY; 
	descRT.Texture2DArray.ArraySize = 1; 
	descRT.Texture2DArray.MipSlice = 0; 

	for (__int32 i = 0; i < 6; i++)
	{
		descRT.Texture2DArray.FirstArraySlice = i;
		m_ambientCubeMapRTV[i] = NULL;
		res = m_device->CreateRenderTargetView(m_ambientCubeMapTexture, &descRT, &m_ambientCubeMapRTV[i]);
		if (FAILED(res))
			return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = texDesc.Format; 
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE; 
	srvDesc.TextureCube.MostDetailedMip = 0; 
	srvDesc.TextureCube.MipLevels = -1; 
	
	m_ambientCubeMapSRV = NULL;
	res = m_device->CreateShaderResourceView(m_ambientCubeMapTexture, &srvDesc, &m_ambientCubeMapSRV);
	if (FAILED(res))
		return false;

	D3D11_TEXTURE2D_DESC depthTexDesc;
	ZeroMemory(&depthTexDesc, sizeof(D3D11_TEXTURE2D_DESC));
	depthTexDesc.Width = AMBIENT_CUBE_MAP_SIZE; 
	depthTexDesc.Height = AMBIENT_CUBE_MAP_SIZE; 
	depthTexDesc.MipLevels = 1; 
	depthTexDesc.ArraySize = 1; 
	depthTexDesc.SampleDesc.Count = 1; 
	depthTexDesc.SampleDesc.Quality = 0; 
	depthTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; 
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL; 
	depthTexDesc.CPUAccessFlags = 0; 
	depthTexDesc.MiscFlags = 0; 
	
	ID3D11Texture2D* m_ambientCubeMapDepthTexture = NULL;
	res = m_device->CreateTexture2D(&depthTexDesc, NULL, &m_ambientCubeMapDepthTexture);
	if (FAILED(res))
		return false;

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	ZeroMemory(&dsvDesc, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	dsvDesc.Format = depthTexDesc.Format; 
	dsvDesc.Flags = 0; 
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D; 
	dsvDesc.Texture2D.MipSlice = 0; 
	
	m_ambientCubeMapDSV = NULL;
	res = m_device->CreateDepthStencilView(m_ambientCubeMapDepthTexture, &dsvDesc, &m_ambientCubeMapDSV);
	if (FAILED(res))
		return false;

	m_testRT = RenderTarget2D::Create(m_device, AMBIENT_CUBE_MAP_SIZE, AMBIENT_CUBE_MAP_SIZE, DXGI_FORMAT_R8G8B8A8_UNORM);

	return true;
}

__int32	Renderer11::Draw()
{
	drawScene(false);
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
	ROOM_INFO* r = &Rooms[roomNumber];

	Vector3 laraPos = Vector3(LaraItem->pos.xPos, LaraItem->pos.yPos - 384, LaraItem->pos.zPos); //Vector3(r->x+r->xSize*1024/2, (r->RoomYBottom+r->RoomYTop)/2, r->z + r->ySize * 1024 / 2);
	Vector3 targets[6] = { laraPos + Vector3::UnitX * WALL_SIZE,
						laraPos - Vector3::UnitX * WALL_SIZE,
						laraPos + Vector3::UnitY * WALL_SIZE,
						laraPos - Vector3::UnitY * WALL_SIZE,
						laraPos + Vector3::UnitZ * WALL_SIZE,
						laraPos - Vector3::UnitZ * WALL_SIZE };
	Vector3 ups[6] = { -Vector3::UnitY, -Vector3::UnitY, Vector3::UnitX, -Vector3::UnitX, -Vector3::UnitY, -Vector3::UnitY };


	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->RSSetState(m_states->CullCounterClockwise());
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	for (__int32 i = 0; i < 6; i++)
	{
		// Clear screen
		m_context->ClearRenderTargetView(m_ambientCubeMapRTV[i], Colors::CornflowerBlue);
		m_context->ClearDepthStencilView(m_ambientCubeMapDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// Bind the back buffer
		m_context->OMSetRenderTargets(1, &m_ambientCubeMapRTV[i], m_ambientCubeMapDSV);
		m_context->RSSetViewports(1, &m_viewport);
		 
		// Set vertex buffer
		UINT stride = sizeof(RendererVertex);
		UINT offset = 0;
		m_context->IASetVertexBuffers(0, 1, &m_roomsVertexBuffer->Buffer, &stride, &offset);
		m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_context->IASetInputLayout(m_inputLayout);
		m_context->IASetIndexBuffer(m_roomsIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

		// Reset viewport
		D3D11_VIEWPORT viewPort;
		viewPort.TopLeftX = 0.0f;
		viewPort.TopLeftY = 0.0f;
		viewPort.Width = AMBIENT_CUBE_MAP_SIZE;
		viewPort.Height = AMBIENT_CUBE_MAP_SIZE;
		viewPort.MinDepth = 0.0f;
		viewPort.MaxDepth = 1.0f;
		m_context->RSSetViewports(1, &viewPort);
		    
		// Set shaders
		m_context->VSSetShader(m_vsAmbientCubeMap, NULL, 0);
		m_context->PSSetShader(m_psAmbientCubeMap, NULL, 0);
		  
		// Set texture
		/*m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
		ID3D11SamplerState* sampler = m_states->AnisotropicClamp();
		m_context->PSSetSamplers(0, 1, &sampler);*/

		// Set camera matrices
		Matrix projection = Matrix::CreatePerspectiveFieldOfView(PI / 2.0f, 1.0f, 20.0f, 200.0f * WALL_SIZE);
		m_stCameraMatrices.View = Matrix::CreateLookAt(laraPos, targets[i], ups[i]).Transpose();
		m_stCameraMatrices.Projection = projection.Transpose();

		updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
		m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

		for (__int32 i = 0; i < NumberRooms; i++)
		{
			RendererRoom* room = m_rooms[i];
			if (room == NULL)
				continue;

			for (__int32 j = 0; j < NUM_BUCKETS; j++)
			{
				RendererBucket* bucket = &room->Buckets[j];

				if (bucket->Vertices.size() == 0)
					continue;

				// Draw vertices
				m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
				m_numDrawCalls++;
			}
		}
	}

	D3DX11SaveTextureToFile(m_context, m_ambientCubeMapTexture, D3DX11_IMAGE_FILE_FORMAT::D3DX11_IFF_DDS, "H:\\cubetest.dds");
	//D3DX11SaveTextureToFile(m_context, m_testRT->Texture, D3DX11_IFF_PNG, "H:\\provart.png");

	/*
	D3D11_VIEWPORT viewPort; 
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	viewPort.Width = AMBIENT_CUBE_MAP_SIZE;
	viewPort.Height = AMBIENT_CUBE_MAP_SIZE;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	
	Vector3 laraPos = Vector3(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
	Vector3 targets[6] = { laraPos + Vector3::UnitX * WALL_SIZE,
						laraPos - Vector3::UnitX * WALL_SIZE,
						laraPos + Vector3::UnitY * WALL_SIZE,
						laraPos - Vector3::UnitY * WALL_SIZE,
						laraPos + Vector3::UnitZ * WALL_SIZE,
						laraPos - Vector3::UnitZ * WALL_SIZE };
	Vector3 ups[6] = { -Vector3::UnitY, -Vector3::UnitY, Vector3::UnitX, -Vector3::UnitX, -Vector3::UnitY, -Vector3::UnitY };

	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->RSSetState(m_states->CullCounterClockwise());
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	// Set shaders
	m_context->VSSetShader(m_vsAmbientCubeMap, NULL, 0);
	m_context->PSSetShader(m_psAmbientCubeMap, NULL, 0);

	// Set texture
	m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
	ID3D11SamplerState* sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	Matrix projection = Matrix::CreatePerspectiveFieldOfView(PI / 2.0f, 1.0f, 20.0f, 200.0f * WALL_SIZE);
	
	// Set vertex buffer
	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;
	m_context->IASetVertexBuffers(0, 1, &m_roomsVertexBuffer->Buffer, &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetIndexBuffer(m_roomsIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

	for (__int32 i = 0; i < 6; i++)
	{
		m_stCameraMatrices.View = Matrix::CreateLookAt(laraPos, targets[i], ups[i]).Transpose();
		m_stCameraMatrices.Projection = projection.Transpose();
		updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
		m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

		m_context->ClearRenderTargetView(m_ambientCubeMapRTV[i], Colors::CornflowerBlue);
		m_context->ClearDepthStencilView(m_ambientCubeMapDSV, D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 1.0f, 0);
		
		m_context->OMSetRenderTargets(1, &m_ambientCubeMapRTV[i], m_ambientCubeMapDSV);
		m_context->RSSetViewports(1, &viewPort);

		for (__int32 n = 0; n < m_roomsToDraw.Size(); n++)
		{
			RendererRoom* room = m_roomsToDraw[n];
			 
			for (__int32 j = 0; j < NUM_BUCKETS; j++)
			{
				RendererBucket* bucket = &room->Buckets[j];

				if (bucket->Vertices.size() == 0)
					continue;

				// Draw vertices
				m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
				m_numDrawCalls++;
			}
		}
	}

	//D3DX11SaveTextureToFile(m_context, m_ambientCubeMapTexture, D3DX11_IMAGE_FILE_FORMAT::D3DX11_IFF_JPG, "H:\\cubemap.png");
	D3DX11SaveTextureToFile(m_context, m_ambientCubeMapTexture, D3DX11_IFF_DDS, "H:\\cubemap.dds");*/

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

	vertices[1].Position.x = size / 2.0f;
	vertices[1].Position.y = 0.0f;
	vertices[1].Position.z = size / 2.0f;
	vertices[1].UV.x = 1.0f;
	vertices[1].UV.y = 0.0f;

	vertices[2].Position.x = size / 2.0f;
	vertices[2].Position.y = 0.0f;
	vertices[2].Position.z = -size / 2.0f;
	vertices[2].UV.x = 1.0f;
	vertices[2].UV.y = 1.0f;

	vertices[3].Position.x = -size / 2.0f;
	vertices[3].Position.y = 0.0f;
	vertices[3].Position.z = -size / 2.0f;
	vertices[3].UV.x = 0.0f;
	vertices[3].UV.y = 1.0f;

	m_context->VSSetShader(m_vsSky, NULL, 0);
	m_context->PSSetShader(m_psSky, NULL, 0);

	m_stCameraMatrices.View = View.Transpose();
	m_stCameraMatrices.Projection = Projection.Transpose();
	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

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
		updateConstantBuffer(m_cbStatic, &m_stStatic, sizeof(CItemBuffer));
		m_context->VSSetConstantBuffers(1, 1, &m_cbStatic);
		m_context->PSSetConstantBuffers(1, 1, &m_cbStatic);

		for (__int32 k = 0; k < moveableObj->ObjectMeshes.size(); k++)
		{
			RendererMesh* mesh = moveableObj->ObjectMeshes[k];

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
	}

	// Clear just the Z-buffer so we can start drawing on top of the horizon
	m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

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
	ID3D11SamplerState* sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	// Set camera matrices
	m_stCameraMatrices.View = View.Transpose();
	m_stCameraMatrices.Projection = Projection.Transpose();

	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);
	 
	if (animated)
		m_primitiveBatch->Begin();

	for (__int32 i = 0; i < m_roomsToDraw.Size(); i++)
	{ 
		RendererRoom* room = m_roomsToDraw[i];

		m_stRoomLights.NumLights = room->LightsToDraw.Size();
		for (__int32 j = 0; j < room->LightsToDraw.Size(); j++)
			memcpy(&m_stRoomLights.Lights[j], room->LightsToDraw[j], 64);
		updateConstantBuffer(m_cbRoomLights, &m_stRoomLights, sizeof(CLightBuffer));
		m_context->PSSetConstantBuffers(1, 1, &m_cbRoomLights);

		for (__int32 j = firstBucket; j < lastBucket; j++)
		{
			if (!animated)
			{
				RendererBucket* bucket = &room->Buckets[j];

				if (bucket->Vertices.size() == 0)
					continue;

				m_context->DrawIndexed(bucket->NumIndices, bucket->StartIndex, 0);
				m_numDrawCalls++;
			}
			else
			{
				RendererBucket* bucket = &room->AnimatedBuckets[j];

				if (bucket->Vertices.size() == 0)
					continue;
				
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

	for (__int32 i = 0; i < m_staticsToDraw.Size(); i++)
	{
		MESH_INFO* msh = m_staticsToDraw[i]->Mesh;
		RendererRoom* room = m_rooms[m_staticsToDraw[i]->RoomIndex];

		RendererObject* staticObj = m_staticObjects[msh->staticNumber];
		RendererMesh* mesh = staticObj->ObjectMeshes[0];

		m_stStatic.World = (Matrix::CreateRotationY(TR_ANGLE_TO_RAD(msh->yRot)) * Matrix::CreateTranslation(msh->x, msh->y, msh->z)).Transpose();
		m_stStatic.Color = Vector4(((msh->shade >> 10) & 0xFF) / 255.0f, ((msh->shade >> 5) & 0xFF) / 255.0f, ((msh->shade >> 0) & 0xFF) / 255.0f, 1.0f);
		updateConstantBuffer(m_cbStatic, &m_stStatic, sizeof(CStaticBuffer));
		m_context->VSSetConstantBuffers(1, 1, &m_cbStatic);

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

	return true;
}

bool Renderer11::drawItems(bool transparent, bool animated)
{
	__int32 firstBucket = (transparent ? 2 : 0);
	__int32 lastBucket = (transparent ? 4 : 2);

	for (__int32 i = 0; i < m_itemsToDraw.Size(); i++)
	{
		RendererItem* item = m_itemsToDraw[i];
		RendererRoom* room = m_rooms[item->Item->roomNumber];
		RendererObject* moveableObj = m_moveableObjects[item->Item->objectNumber];

		if (moveableObj->DoNotDraw)
			continue;

		m_stItem.World = item->World.Transpose();
		m_stItem.Position = Vector4(item->Item->pos.xPos, item->Item->pos.yPos, item->Item->pos.zPos, 1.0f);
		m_stItem.AmbientLight = room->AmbientLight;
		memcpy(m_stItem.BonesMatrices, item->AnimationTransforms, sizeof(Matrix) * 32);
		updateConstantBuffer(m_cbItem, &m_stItem, sizeof(CItemBuffer));
		m_context->VSSetConstantBuffers(1, 1, &m_cbItem);

		for (__int32 k = 0; k < moveableObj->ObjectMeshes.size(); k++)
		{
			RendererMesh* mesh = moveableObj->ObjectMeshes[k];

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

	return true;
}

bool Renderer11::drawLara(bool transparent)
{
	UINT stride = sizeof(RendererVertex);
	UINT offset = 0;

	__int32 firstBucket = (transparent ? 2 : 0);
	__int32 lastBucket = (transparent ? 4 : 2);

	m_context->IASetVertexBuffers(0, 1, &m_moveablesVertexBuffer->Buffer, &stride, &offset);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_context->IASetInputLayout(m_inputLayout);
	m_context->IASetIndexBuffer(m_moveablesIndexBuffer->Buffer, DXGI_FORMAT_R32_UINT, 0);

	// Set shaders
	m_context->VSSetShader(m_vsItems, NULL, 0);
	m_context->PSSetShader(m_psItems, NULL, 0);

	// Set texture
	m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
	ID3D11SamplerState* sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	// Set camera matrices
	m_stCameraMatrices.View = View.Transpose();
	m_stCameraMatrices.Projection = Projection.Transpose();

	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

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
		RendererMesh* mesh = m_meshPointersToMesh[Lara.meshPtrs[k]];

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

	m_strings.clear();

	ViewProjection = View * Projection;

	// Prepare the scene to draw
	auto time1 = chrono::high_resolution_clock::now();

	clearSceneItems();
	collectRooms();
	prepareLights();
	updateLaraAnimations();
	updateItemsAnimations();
	updateEffects();

	// Update animated textures every 2 frames
	if (GnFrameCounter % 2 == 0)
		updateAnimatedTextures();

	auto time2 = chrono::high_resolution_clock::now();
	m_timeUpdate = (chrono::duration_cast<ns>(time2 - time1)).count() / 1000000;
	time1 = time2;

	// Prepare thr ambient cube map
	//drawAmbientCubeMap(LaraItem->roomNumber);

	// Reset GPU state
	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->RSSetState(m_states->CullCounterClockwise());
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	// Clear screen
	m_context->ClearRenderTargetView(m_backBufferRTV, Colors::CornflowerBlue);
	m_context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// Bind the back buffer
	m_context->OMSetRenderTargets(1, &m_backBufferRTV, m_depthStencilView);
	m_context->RSSetViewports(1, &m_viewport);

	// Draw stuff
	drawHorizonAndSky();

	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	
	drawRooms(false, false);
	drawRooms(false, true);
	drawStatics(false);
	drawLara(false);
	drawItems(false, false);
	drawItems(false, true);

	m_context->OMSetBlendState(m_states->Additive(), NULL, 0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthRead(), 0);

	drawRooms(true, false);
	drawRooms(true, true);
	drawStatics(true);
	drawLara(true);
	drawItems(true, false);
	drawItems(true, true);

	m_context->OMSetBlendState(m_states->Opaque(), NULL, 0xFFFFFFFF);
	m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	// Draw sprites
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

	doRain();

	drawSprites();
	drawLines3D();

	/*

	

	

	// Set shaders
	m_context->VSSetShader(m_vsHairs, NULL, 0);
	m_context->PSSetShader(m_psHairs, NULL, 0);

	// Set texture
	m_context->PSSetShaderResources(0, 1, &m_textureAtlas->ShaderResourceView);
	sampler = m_states->AnisotropicClamp();
	m_context->PSSetSamplers(0, 1, &sampler);

	// Set camera matrices
	m_stCameraMatrices.View = View.Transpose();
	m_stCameraMatrices.Projection = Projection.Transpose();

	updateConstantBuffer(m_cbCameraMatrices, &m_stCameraMatrices, sizeof(CCameraMatrixBuffer));
	m_context->VSSetConstantBuffers(0, 1, &m_cbCameraMatrices);

	if (m_moveableObjects[ID_HAIR] != NULL)
	{
		m_primitiveBatch->Begin();
		m_primitiveBatch->DrawIndexed(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
			(const unsigned __int16*)m_hairIndices.data(), m_numHairIndices,
			m_hairVertices.data(), m_numHairVertices);
		m_primitiveBatch->End();
	}*/

	time2 = chrono::high_resolution_clock::now();
	m_timeFrame = (chrono::duration_cast<ns>(time2 - time1)).count() / 1000000;
	time1 = time2;

	m_currentY = 10;

	ROOM_INFO* r = &Rooms[LaraItem->roomNumber];

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
	return 0;
}

__int32 Renderer11::DrawPickup(__int16 objectNum)
{
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

bool Renderer11::drawAllStrings()
{
	m_spriteBatch->Begin();

	for (__int32 i = 0; i < m_strings.size(); i++)
	{
		RendererStringToDraw* str = &m_strings[i];

		// Draw shadow if needed
		if (str->Flags & PRINTSTRING_OUTLINE)
			m_gameFont->DrawString(m_spriteBatch, str->String.c_str(), Vector2(str->X + 1, str->Y + 1), Vector3(0, 0, 0));
		
		// Draw string
		m_gameFont->DrawString(m_spriteBatch, str->String.c_str(), Vector2(str->X, str->Y), str->Color);
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

	m_rooms = (RendererRoom**)malloc(sizeof(RendererRoom*) * 1024);
	ZeroMemory(m_rooms, sizeof(RendererRoom*) * 1024);

	// Step 0: prepare animated textures
	__int16 numSets = *AnimatedTextureRanges;
	__int16* animatedPtr = AnimatedTextureRanges;
	animatedPtr++;
	
	m_animatedTextureSets = (RendererAnimatedTextureSet**)malloc(sizeof(RendererAnimatedTextureSet*) * numSets);
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

	
	if (g_GameFlow->GetLevel(CurrentLevel)->LaraType == LARA_DRAW_TYPE::LARA_YOUNG)
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

	D3DX11SaveTextureToFileA(m_context, m_skyTexture->Texture, D3DX11_IFF_PNG, "H:\\sky.png");

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
		r->AmbientLight = Vector4(room->ambient.r / 255.0f, room->ambient.g / 255.0f, room->ambient.b / 255.0f, 1.0f);
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
					light.Color = Vector4(oldLight->r, oldLight->g, oldLight->b, 1.0f);
					light.Direction = Vector4(oldLight->dx, oldLight->dy, oldLight->dz, 1.0f);
					light.Type = LIGHT_TYPES::LIGHT_TYPE_SUN;

					r->Lights.push_back(light);
				}
				else if (oldLight->LightType == LIGHT_TYPE_POINT)
				{
					light.Position = Vector4(oldLight->x, oldLight->y, oldLight->z, 1.0f);
					light.Color = Vector4(oldLight->r, oldLight->g, oldLight->b, 1.0f);
					light.Direction = Vector4(oldLight->dx, oldLight->dy, oldLight->dz, 1.0f);
					light.Intensity = 1.0f;
					light.In = oldLight->In;
					light.Out = oldLight->Out;
					light.Type = LIGHT_TYPE_POINT;

					r->Lights.push_back(light);
				}
				else if (oldLight->LightType == LIGHT_TYPE_SHADOW)
				{
					light.Position = Vector4(oldLight->x, oldLight->y, oldLight->z, 1.0f);
					light.Color = Vector4(oldLight->r, oldLight->g, oldLight->b, 1.0f);
					light.Direction = Vector4(oldLight->dx, oldLight->dy, oldLight->dz, 1.0f);
					light.In = oldLight->In;
					light.Out = oldLight->Out;
					light.Type = LIGHT_TYPE_SHADOW;

					r->Lights.push_back(light);
				}
				else if (oldLight->LightType == LIGHT_TYPE_SPOT)
				{
					light.Position = Vector4(oldLight->x, oldLight->y, oldLight->z, 1.0f);
					light.Color = Vector4(oldLight->r, oldLight->g, oldLight->b, 1.0f);
					light.Direction = Vector4(oldLight->dx, oldLight->dy, oldLight->dz, 1.0f);
					light.Intensity = 1.0f;
					light.In = oldLight->In;
					light.Out = oldLight->Range;   
					light.Range = oldLight->Range;
					light.Type = LIGHT_TYPE_SPOT;

					r->Lights.push_back(light);
				}
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
	m_sprites = (RendererSprite**)malloc(sizeof(RendererSprite*) * 4);
	ZeroMemory(m_sprites, sizeof(RendererSprite*) * 4);

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

	// Preallocate lists
	m_roomsToDraw.Reserve(NumberRooms);
	m_itemsToDraw.Reserve(NUM_ITEMS);
	m_effectsToDraw.Reserve(NUM_ITEMS);
	m_lightsToDraw.Reserve(16384);
	m_dynamicLights.Reserve(16384);
	m_staticsToDraw.Reserve(16384);
	m_spritesToDraw.Reserve(MAX_SPRITES);
	m_lines3DToDraw.Reserve(MAX_LINES_3D);
	m_spritesBuffer = (RendererSpriteToDraw*)malloc(sizeof(RendererSpriteToDraw) * MAX_SPRITES);
	m_lines3DBuffer = (RendererLine3DToDraw*)malloc(sizeof(RendererLine3DToDraw) * MAX_LINES_3D);

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

}

void Renderer11::DrawHealthBar(__int32 percentual)
{

}

void Renderer11::DrawAirBar(__int32 percentual)
{

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

	dynamicLight->Position = Vector4(x, y, z, 1.0f);
	dynamicLight->Color = Vector4(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
	dynamicLight->Out = falloff * 256.0f;
	dynamicLight->Type = LIGHT_TYPES::LIGHT_TYPE_POINT;
	dynamicLight->Dynamic = true;
	dynamicLight->Intensity = 2.0f;

	m_dynamicLights.Add(dynamicLight);
	NumDynamics++;
}

void Renderer11::EnableCinematicBars(bool value)
{

}

void Renderer11::FadeIn()
{

}

void Renderer11::FadeOut()
{

}

void Renderer11::DrawLoadingScreen(char* fileName)
{
	
}

void Renderer11::UpdateProgress(float value)
{

}

bool Renderer11::IsFading()
{
	return false;
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
	return false;
}

bool Renderer11::ChangeScreenResolution(__int32 width, __int32 height, __int32 frequency, bool windowed)
{
	return true;
}

void Renderer11::Test()
{
	
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

		collectItems(node->To);
		collectStatics(node->To);
		collectEffects(node->To);
		collectLights(node->To);
				
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

		if (item->status == ITEM_DEACTIVATED || item->status == ITEM_INVISIBLE)
			continue;

		//if (m_moveableObjects.find(item->objectNumber) == m_moveableObjects.end())
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

inline void Renderer11::collectLights(__int16 roomNumber)
{
	RendererRoom* room = m_rooms[roomNumber];
	if (room == NULL)
		return;

	ROOM_INFO* r = room->Room;

	if (r->numLights <= 0)
		return;

	__int32 numLights = room->Lights.size();

	for (__int32 j = 0; j < numLights; j++)
	{
		Vector3 laraPos = Vector3(LaraItem->pos.xPos, LaraItem->pos.yPos, LaraItem->pos.zPos);
		Vector3 lightPos = Vector3(room->Lights[j].Position.x, room->Lights[j].Position.y, room->Lights[j].Position.z);

		// Collect only lights nearer than 20 sectors
		if ((laraPos - lightPos).Length() >= 20 * WALL_SIZE)
			continue;

		// Check only lights different from sun
		/*if (room->Lights[j]->Type != LIGHT_TYPES::LIGHT_TYPE_SUN)
		{
			// Now check if lights are touching items
			bool isTouchingItem = false;
			for (__int32 k = 0; k < m_itemsToDraw.size(); k++)
			{
				Vector3 itemPos = Vector3(m_itemsToDraw[k]->Item->pos.xPos, m_itemsToDraw[k]->Item->pos.yPos, m_itemsToDraw[k]->Item->pos.zPos);
				float distance = D3DXVec3Length(&(itemPos - lightPos));

				if (room->Lights[j]->Type == LIGHT_TYPES::LIGHT_TYPE_SPOT)
				{
					if (distance < room->Lights[j]->Range)
					{
						isTouchingItem = true;
						break;
					}
				}
				else
				{
					if (distance < room->Lights[j]->Out)
					{
						isTouchingItem = true;
						break;
					}
				}
			}

			// If the light is not touching an item, then discard it
			if (!isTouchingItem)
				continue;
		}*/

		RendererLight* light = &room->Lights[j];
		m_lightsToDraw.Add(light);
	}

	// Collect dynamic lights
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
		newPolygon.Indices[0] = baseVertices;
		newPolygon.Indices[1] = baseVertices + 1;
		newPolygon.Indices[2] = baseVertices + 2;
		bucket->Polygons.push_back(newPolygon);
	}

	free(vertices);
	if (normals != NULL) free(normals);
	if (colors != NULL) free(colors);

	if (m_meshPointersToMesh.find(refMeshPtr) == m_meshPointersToMesh.end())
		m_meshPointersToMesh.insert(pair<__int16*, RendererMesh*>(refMeshPtr, mesh));

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

		for (__int32 p = 0; p < ((level->LaraType == LARA_DRAW_TYPE::LARA_YOUNG) ? 2 : 1); p++)
		{
			// We can't use hardware skinning here, however hairs have just a few vertices so 
			// it's not so bad doing skinning in software
			if (level->LaraType == LARA_DRAW_TYPE::LARA_YOUNG)
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

	RendererLine3DToDraw* line = &m_lines3DBuffer[m_nextLine3D++];

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

void Renderer11::drawUnderwaterDust()
{
	/*if (m_firstUnderwaterDustParticles)
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
	*/
	return;
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
		RendererLine3DToDraw* line = m_lines3DToDraw[i];

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