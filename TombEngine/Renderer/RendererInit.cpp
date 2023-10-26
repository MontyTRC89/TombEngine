#include "framework.h"

#include <string>
#include <memory>
#include <filesystem>

#include "Renderer/Renderer.h"
#include "Renderer/Quad/RenderQuad.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/configuration.h"
#include "Specific/memory/Vector.h"
#include "Specific/trutils.h"
#include "Specific/winmain.h"

extern GameConfiguration g_Configuration;

static std::wstring GetAssetPath(const wchar_t* fileName)
{
	return TEN::Utils::ToWString(g_GameFlow->GetGameDir()) + fileName;
}

namespace TEN::Renderer
{
	void Renderer::Initialize(int w, int h, bool windowed, HWND handle)
	{
		TENLog("Initializing DX11...", LogLevel::Info);

		screenWidth = w;
		screenHeight = h;
		isWindowed = windowed;
		InitializeScreen(w, h, handle, false);
		InitializeCommonTextures();

		// Initialize render states
		renderStates = std::make_unique<CommonStates>(device.Get());

		// Load shaders
		ComPtr<ID3D10Blob> blob;
		const D3D_SHADER_MACRO roomDefinesAnimated[] = { "ANIMATED", "", nullptr, nullptr };
		const D3D_SHADER_MACRO roomDefinesShadowMap[] = { "SHADOW_MAP", "", nullptr, nullptr };

		vsRooms = Utils::compileVertexShader(device.Get(), GetAssetPath(L"Shaders\\DX11_Rooms.fx"), "VS", "vs_4_0", nullptr, blob);

		// Initialize input layout using first vertex shader.
		D3D11_INPUT_ELEMENT_DESC inputLayoutItems[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "ANIMATIONFRAMEOFFSET", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "EFFECTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "POLYINDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "DRAWINDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "HASH", 0, DXGI_FORMAT_R32_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		Utils::throwIfFailed(device->CreateInputLayout(inputLayoutItems, 12, blob->GetBufferPointer(), blob->GetBufferSize(), &inputLayout));

		vsRooms_Anim = Utils::compileVertexShader(device.Get(), GetAssetPath(L"Shaders\\DX11_Rooms.fx"), "VS", "vs_4_0", &roomDefinesAnimated[0], blob);
		psRooms = Utils::compilePixelShader(device.Get(), GetAssetPath(L"Shaders\\DX11_Rooms.fx"), "PS", "ps_4_1", nullptr, blob);
		vsItems = Utils::compileVertexShader(device.Get(), GetAssetPath(L"Shaders\\DX11_Items.fx"), "VS", "vs_4_0", nullptr, blob);
		psItems = Utils::compilePixelShader(device.Get(), GetAssetPath(L"Shaders\\DX11_Items.fx"), "PS", "ps_4_0", nullptr, blob);
		vsStatics = Utils::compileVertexShader(device.Get(), GetAssetPath(L"Shaders\\DX11_Statics.fx"), "VS", "vs_4_0", nullptr, blob);
		psStatics = Utils::compilePixelShader(device.Get(), GetAssetPath(L"Shaders\\DX11_Statics.fx"), "PS", "ps_4_0", nullptr, blob);
		vsHairs = Utils::compileVertexShader(device.Get(), GetAssetPath(L"Shaders\\DX11_Hairs.fx"), "VS", "vs_4_0", nullptr, blob);
		psHairs = Utils::compilePixelShader(device.Get(), GetAssetPath(L"Shaders\\DX11_Hairs.fx"), "PS", "ps_4_0", nullptr, blob);
		vsSky = Utils::compileVertexShader(device.Get(), GetAssetPath(L"Shaders\\DX11_Sky.fx"), "VS", "vs_4_0", nullptr, blob);
		psSky = Utils::compilePixelShader(device.Get(), GetAssetPath(L"Shaders\\DX11_Sky.fx"), "PS", "ps_4_0", nullptr, blob);
		vsSprites = Utils::compileVertexShader(device.Get(), GetAssetPath(L"Shaders\\DX11_Sprites.fx"), "VS", "vs_4_0", nullptr, blob);
		psSprites = Utils::compilePixelShader(device.Get(), GetAssetPath(L"Shaders\\DX11_Sprites.fx"), "PS", "ps_4_0", nullptr, blob);
		vsSolid = Utils::compileVertexShader(device.Get(), GetAssetPath(L"Shaders\\DX11_Solid.fx"), "VS", "vs_4_0", nullptr, blob);
		psSolid = Utils::compilePixelShader(device.Get(), GetAssetPath(L"Shaders\\DX11_Solid.fx"), "PS", "ps_4_0", nullptr, blob);
		vsInventory = Utils::compileVertexShader(device.Get(), GetAssetPath(L"Shaders\\DX11_Inventory.fx"), "VS", "vs_4_0", nullptr, blob);
		psInventory = Utils::compilePixelShader(device.Get(), GetAssetPath(L"Shaders\\DX11_Inventory.fx"), "PS", "ps_4_0", nullptr, blob);
		vsFullScreenQuad = Utils::compileVertexShader(device.Get(), GetAssetPath(L"Shaders\\DX11_FullScreenQuad.fx"), "VS", "vs_4_0", nullptr, blob);
		psFullScreenQuad = Utils::compilePixelShader(device.Get(), GetAssetPath(L"Shaders\\DX11_FullScreenQuad.fx"), "PS", "ps_4_0", nullptr, blob);
		vsShadowMap = Utils::compileVertexShader(device.Get(), GetAssetPath(L"Shaders\\DX11_ShadowMap.fx"), "VS", "vs_4_0", nullptr, blob);
		psShadowMap = Utils::compilePixelShader(device.Get(), GetAssetPath(L"Shaders\\DX11_ShadowMap.fx"), "PS", "ps_4_0", nullptr, blob);
		vsHUD = Utils::compileVertexShader(device.Get(), GetAssetPath(L"Shaders\\HUD\\DX11_VS_HUD.hlsl"), "VS", "vs_4_0", nullptr, blob);
		psHUDColor = Utils::compilePixelShader(device.Get(), GetAssetPath(L"Shaders\\HUD\\DX11_PS_HUD.hlsl"), "PSColored", "ps_4_0", nullptr, blob);
		psHUDTexture = Utils::compilePixelShader(device.Get(), GetAssetPath(L"Shaders\\HUD\\DX11_PS_HUD.hlsl"), "PSTextured", "ps_4_0", nullptr, blob);
		psHUDBarColor = Utils::compilePixelShader(device.Get(), GetAssetPath(L"Shaders\\HUD\\DX11_PS_HUDBar.hlsl"), "PSTextured", "ps_4_0", nullptr, blob);
		vsFinalPass = Utils::compileVertexShader(device.Get(), GetAssetPath(L"Shaders\\DX11_FinalPass.fx"), "VS", "vs_4_0", nullptr, blob);
		psFinalPass = Utils::compilePixelShader(device.Get(), GetAssetPath(L"Shaders\\DX11_FinalPass.fx"), "PS", "ps_4_0", nullptr, blob);
		vsInstancedStaticMeshes = Utils::compileVertexShader(device.Get(), GetAssetPath(L"Shaders\\DX11_InstancedStatics.fx"), "VS", "vs_4_0", nullptr, blob);
		psInstancedStaticMeshes = Utils::compilePixelShader(device.Get(), GetAssetPath(L"Shaders\\DX11_InstancedStatics.fx"), "PS", "ps_4_0", nullptr, blob);
		vsInstancedSprites = Utils::compileVertexShader(device.Get(), GetAssetPath(L"Shaders\\DX11_InstancedSprites.fx"), "VS", "vs_4_0", nullptr, blob);
		psInstancedSprites = Utils::compilePixelShader(device.Get(), GetAssetPath(L"Shaders\\DX11_InstancedSprites.fx"), "PS", "ps_4_0", nullptr, blob);

		// Initialize constant buffers
		cbCameraMatrices = CreateConstantBuffer<CCameraMatrixBuffer>();
		cbItem = CreateConstantBuffer<CItemBuffer>();
		cbStatic = CreateConstantBuffer<CStaticBuffer>();
		cbLights = CreateConstantBuffer<CLightBuffer>();
		cbMisc = CreateConstantBuffer<CMiscBuffer>();
		cbShadowMap = CreateConstantBuffer<CShadowLightBuffer>();
		cbRoom = CreateConstantBuffer<CRoomBuffer>();
		cbAnimated = CreateConstantBuffer<CAnimatedBuffer>();
		cbPostProcessBuffer = CreateConstantBuffer<CPostProcessBuffer>();
		cbBlending = CreateConstantBuffer<CBlendingBuffer>();
		cbInstancedSpriteBuffer = CreateConstantBuffer<CInstancedSpriteBuffer>();
		cbInstancedStaticMeshBuffer = CreateConstantBuffer<CInstancedStaticMeshBuffer>();
		cbSky = CreateConstantBuffer<CSkyBuffer>();

		//Prepare HUD Constant buffer  
		cbHUDBar = CreateConstantBuffer<CHUDBarBuffer>();
		cbHUD = CreateConstantBuffer<CHUDBuffer>();
		cbSprite = CreateConstantBuffer<CSpriteBuffer>();
		stHUD.View = Matrix::CreateLookAt(Vector3::Zero, Vector3(0, 0, 1), Vector3(0, -1, 0));
		stHUD.Projection = Matrix::CreateOrthographicOffCenter(0, SCREEN_SPACE_RES.x, 0, SCREEN_SPACE_RES.y, 0, 1.0f);
		cbHUD.updateData(stHUD, context.Get());
		currentCausticsFrame = 0;

		// Preallocate lists
		dynamicLights = createVector<RendererLight>(MAX_DYNAMIC_LIGHTS);
		lines3DToDraw = createVector<RendererLine3D>(MAX_LINES_3D);
		lines2DToDraw = createVector<RendererLine2D>(MAX_LINES_2D);
		transparentFaces = createVector<RendererTransparentFace>(MAX_TRANSPARENT_FACES);
		transparentFacesVertices = createVector<Vertex>(MAX_TRANSPARENT_VERTICES);
		transparentFacesIndices.reserve(MAX_TRANSPARENT_VERTICES); // = createVector<int>(MAX_TRANSPARENT_VERTICES);

		for (int i = 0; i < NUM_ITEMS; i++)
		{
			items[i].LightsToDraw = createVector<RendererLight*>(MAX_LIGHTS_PER_ITEM);
			effects[i].LightsToDraw = createVector<RendererLight*>(MAX_LIGHTS_PER_ITEM);
		}

		transparentFacesVertexBuffer = VertexBuffer(device.Get(), TRANSPARENT_BUCKET_SIZE);
		transparentFacesIndexBuffer = IndexBuffer(device.Get(), TRANSPARENT_BUCKET_SIZE);

		D3D11_BLEND_DESC blendStateDesc{};
		blendStateDesc.AlphaToCoverageEnable = false;
		blendStateDesc.IndependentBlendEnable = false;
		blendStateDesc.RenderTarget[0].BlendEnable = true;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		Utils::throwIfFailed(device->CreateBlendState(&blendStateDesc, subtractiveBlendState.GetAddressOf()));

		blendStateDesc.AlphaToCoverageEnable = false;
		blendStateDesc.IndependentBlendEnable = false;
		blendStateDesc.RenderTarget[0].BlendEnable = true;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_INV_DEST_COLOR;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		Utils::throwIfFailed(device->CreateBlendState(&blendStateDesc, screenBlendState.GetAddressOf()));

		blendStateDesc.AlphaToCoverageEnable = false;
		blendStateDesc.IndependentBlendEnable = false;
		blendStateDesc.RenderTarget[0].BlendEnable = true;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_MAX;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		Utils::throwIfFailed(device->CreateBlendState(&blendStateDesc, lightenBlendState.GetAddressOf()));

		blendStateDesc.AlphaToCoverageEnable = false;
		blendStateDesc.IndependentBlendEnable = false;
		blendStateDesc.RenderTarget[0].BlendEnable = true;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_DEST_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		Utils::throwIfFailed(device->CreateBlendState(&blendStateDesc, excludeBlendState.GetAddressOf()));

		D3D11_SAMPLER_DESC shadowSamplerDesc = {};
		shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		Utils::throwIfFailed(device->CreateSamplerState(&shadowSamplerDesc, m_shadowSampler.GetAddressOf()));
		m_shadowSampler->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("ShadowSampler") + 1, "ShadowSampler");

		D3D11_RASTERIZER_DESC rasterizerStateDesc = {};

		rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
		rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerStateDesc.DepthClipEnable = true;
		rasterizerStateDesc.MultisampleEnable = true;
		rasterizerStateDesc.AntialiasedLineEnable = true;
		rasterizerStateDesc.ScissorEnable = true;
		Utils::throwIfFailed(device->CreateRasterizerState(&rasterizerStateDesc, cullCounterClockwiseRasterizerState.GetAddressOf()));

		rasterizerStateDesc.CullMode = D3D11_CULL_FRONT;
		rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerStateDesc.DepthClipEnable = true;
		rasterizerStateDesc.MultisampleEnable = true;
		rasterizerStateDesc.AntialiasedLineEnable = true;
		rasterizerStateDesc.ScissorEnable = true;
		Utils::throwIfFailed(device->CreateRasterizerState(&rasterizerStateDesc, cullClockwiseRasterizerState.GetAddressOf()));

		rasterizerStateDesc.CullMode = D3D11_CULL_NONE;
		rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerStateDesc.DepthClipEnable = true;
		rasterizerStateDesc.MultisampleEnable = true;
		rasterizerStateDesc.AntialiasedLineEnable = true;
		rasterizerStateDesc.ScissorEnable = true;
		Utils::throwIfFailed(device->CreateRasterizerState(&rasterizerStateDesc, cullNoneRasterizerState.GetAddressOf()));

		InitializeGameBars();
		initQuad(device.Get());
		InitializeSky();
	}

	void Renderer::InitializeSky()
	{
		Vertex vertices[SKY_VERTICES_COUNT];
		int indices[SKY_INDICES_COUNT];
		int size = SKY_SIZE;

		int lastVertex = 0;
		int lastIndex = 0;

		for (int x = 0; x < SKY_TILES_COUNT; x++)
		{
			for (int z = 0; z < SKY_TILES_COUNT; z++)
			{
				indices[lastIndex + 0] = lastVertex + 0;
				indices[lastIndex + 1] = lastVertex + 1;
				indices[lastIndex + 2] = lastVertex + 2;
				indices[lastIndex + 3] = lastVertex + 0;
				indices[lastIndex + 4] = lastVertex + 2;
				indices[lastIndex + 5] = lastVertex + 3;

				lastIndex += 6;

				vertices[lastVertex].Position.x = -size / 2.0f + x * 512.0f;
				vertices[lastVertex].Position.y = 0.0f;
				vertices[lastVertex].Position.z = -size / 2.0f + (z + 1) * 512.0f;
				vertices[lastVertex].UV.x = x / 20.0f;
				vertices[lastVertex].UV.y = (z + 1) / 20.0f;
				vertices[lastVertex].Color.x = 1.0f;
				vertices[lastVertex].Color.y = 1.0f;
				vertices[lastVertex].Color.z = 1.0f;
				vertices[lastVertex].Color.w = 1.0f;

				lastVertex++;

				vertices[lastVertex].Position.x = -size / 2.0f + (x + 1) * 512.0f;
				vertices[lastVertex].Position.y = 0.0f;
				vertices[lastVertex].Position.z = -size / 2.0f + (z + 1) * 512.0f;
				vertices[lastVertex].UV.x = (x + 1) / 20.0f;
				vertices[lastVertex].UV.y = (z + 1) / 20.0f;
				vertices[lastVertex].Color.x = 1.0f;
				vertices[lastVertex].Color.y = 1.0f;
				vertices[lastVertex].Color.z = 1.0f;
				vertices[lastVertex].Color.w = 1.0f;

				lastVertex++;

				vertices[lastVertex].Position.x = -size / 2.0f + (x + 1) * 512.0f;
				vertices[lastVertex].Position.y = 0.0f;
				vertices[lastVertex].Position.z = -size / 2.0f + z * 512.0f;
				vertices[lastVertex].UV.x = (x + 1) / 20.0f;
				vertices[lastVertex].UV.y = z / 20.0f;
				vertices[lastVertex].Color.x = 1.0f;
				vertices[lastVertex].Color.y = 1.0f;
				vertices[lastVertex].Color.z = 1.0f;
				vertices[lastVertex].Color.w = 1.0f;

				lastVertex++;

				vertices[lastVertex].Position.x = -size / 2.0f + x * 512.0f;
				vertices[lastVertex].Position.y = 0.0f;
				vertices[lastVertex].Position.z = -size / 2.0f + z * 512.0f;
				vertices[lastVertex].UV.x = x / 20.0f;
				vertices[lastVertex].UV.y = z / 20.0f;
				vertices[lastVertex].Color.x = 1.0f;
				vertices[lastVertex].Color.y = 1.0f;
				vertices[lastVertex].Color.z = 1.0f;
				vertices[lastVertex].Color.w = 1.0f;

				lastVertex++;
			}
		}

		skyVertexBuffer = VertexBuffer(device.Get(), SKY_VERTICES_COUNT, vertices);
		skyIndexBuffer = IndexBuffer(device.Get(), SKY_INDICES_COUNT, indices);
	}

	void Renderer::InitializeScreen(int w, int h, HWND handle, bool reset)
	{
		DXGI_SWAP_CHAIN_DESC sd;
		sd.BufferDesc.Width = w;
		sd.BufferDesc.Height = h;
		sd.BufferDesc.RefreshRate.Numerator = 0;
		sd.BufferDesc.RefreshRate.Denominator = 0;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
		sd.Windowed = true;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sd.Flags = 0;
		sd.OutputWindow = handle;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferCount = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		ComPtr<IDXGIDevice> dxgiDevice;
		Utils::throwIfFailed(device.As(&dxgiDevice));

		ComPtr<IDXGIAdapter> dxgiAdapter;
		Utils::throwIfFailed(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), &dxgiAdapter));

		ComPtr<IDXGIFactory> dxgiFactory;
		Utils::throwIfFailed(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), &dxgiFactory));

		Utils::throwIfFailed(dxgiFactory->CreateSwapChain(device.Get(), &sd, &swapChain));

		dxgiFactory->MakeWindowAssociation(handle, DXGI_MWA_NO_ALT_ENTER);

		// Initialize render targets
		ID3D11Texture2D* backBufferTexture = NULL;
		Utils::throwIfFailed(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast <void**>(&backBufferTexture)));
		backBuffer = RenderTarget2D(device.Get(), backBufferTexture, DXGI_FORMAT_D24_UNORM_S8_UINT);

		renderTarget = RenderTarget2D(device.Get(), w, h, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_D24_UNORM_S8_UINT);
		dumpScreenRenderTarget = RenderTarget2D(device.Get(), w, h, DXGI_FORMAT_R8G8B8A8_UNORM);
		depthMap = RenderTarget2D(device.Get(), w, h, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D16_UNORM);
		reflectionCubemap = RenderTargetCube(device.Get(), 128, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		shadowMap = Texture2DArray(device.Get(), g_Configuration.ShadowMapSize, 6, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT);

		// Initialize sprite and primitive batches
		spriteBatch = std::make_unique<SpriteBatch>(context.Get());
		primitiveBatch = std::make_unique<PrimitiveBatch<Vertex>>(context.Get());

		// Initialize viewport
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = w;
		viewport.Height = h;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		shadowMapViewport.TopLeftX = 0;
		shadowMapViewport.TopLeftY = 0;
		shadowMapViewport.Width = g_Configuration.ShadowMapSize;
		shadowMapViewport.Height = g_Configuration.ShadowMapSize;
		shadowMapViewport.MinDepth = 0.0f;
		shadowMapViewport.MaxDepth = 1.0f;

		viewportToolkit = Viewport(viewport.TopLeftX, viewport.TopLeftY, viewport.Width, viewport.Height,
			viewport.MinDepth, viewport.MaxDepth);

		SetFullScreen();
	}

	void Renderer::InitializeCommonTextures()
	{
		// Initialize font.
		auto fontPath = GetAssetPath(L"Textures/Font.spritefont");
		if (!std::filesystem::is_regular_file(fontPath))
			throw std::runtime_error("Font not found; path " + TEN::Utils::ToString(fontPath) + " is missing.");

		gameFont = std::make_unique<SpriteFont>(device.Get(), fontPath.c_str());

		// Initialize common textures.
		SetTextureOrDefault(logoTexture, GetAssetPath(L"Textures/Logo.png"));
		SetTextureOrDefault(loadingBarBorder, GetAssetPath(L"Textures/LoadingBarBorder.png"));
		SetTextureOrDefault(loadingBarInner, GetAssetPath(L"Textures/LoadingBarInner.png"));
		SetTextureOrDefault(whiteTexture, GetAssetPath(L"Textures/WhiteSprite.png"));

		whiteSprite.Height = whiteTexture.Height;
		whiteSprite.Width = whiteTexture.Width;
		whiteSprite.UV[0] = Vector2(0.0f, 0.0f);
		whiteSprite.UV[1] = Vector2(1.0f, 0.0f);
		whiteSprite.UV[2] = Vector2(1.0f, 1.0f);
		whiteSprite.UV[3] = Vector2(0.0f, 1.0f);
		whiteSprite.Texture = &whiteTexture;
	}

	void Renderer::Create()
	{
		TENLog("Creating DX11 renderer device...", LogLevel::Info);

		D3D_FEATURE_LEVEL levels[] = { D3D_FEATURE_LEVEL_11_0 };
		D3D_FEATURE_LEVEL featureLevel;
		HRESULT res;

		if constexpr (DebugBuild)
		{
			res = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG,
				levels, 1, D3D11_SDK_VERSION, &device, &featureLevel, &context);
		}
		else
		{
			res = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL,
				levels, 1, D3D11_SDK_VERSION, &device, &featureLevel, &context);
		}

		Utils::throwIfFailed(res);
	}

	void Renderer::ToggleFullScreen(bool force)
	{
		isWindowed = force ? false : !isWindowed;
		SetFullScreen();
	}

	void Renderer::SetFullScreen()
	{
		if (!isWindowed)
		{
			SetWindowLongPtr(WindowsHandle, GWL_STYLE, 0);
			SetWindowLongPtr(WindowsHandle, GWL_EXSTYLE, WS_EX_TOPMOST);
			SetWindowPos(WindowsHandle, HWND_TOP, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
			ShowWindow(WindowsHandle, SW_SHOWMAXIMIZED);
		}
		else
		{
			int frameW = GetSystemMetrics(SM_CXPADDEDBORDER);
			int frameX = GetSystemMetrics(SM_CXSIZEFRAME);
			int frameY = GetSystemMetrics(SM_CYSIZEFRAME);
			int frameC = GetSystemMetrics(SM_CYCAPTION);

			int borderWidth = (frameX + frameW) * 2;
			int borderHeight = (frameY + frameW) * 2 + frameC;

			SetWindowLongPtr(WindowsHandle, GWL_STYLE, WS_OVERLAPPEDWINDOW);
			SetWindowLongPtr(WindowsHandle, GWL_EXSTYLE, 0);
			ShowWindow(WindowsHandle, SW_SHOWNORMAL);
			SetWindowPos(WindowsHandle, HWND_TOP, 0, 0, screenWidth + borderWidth, screenHeight + borderHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
		}

		UpdateWindow(WindowsHandle);
	}
}