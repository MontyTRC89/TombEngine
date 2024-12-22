#include "framework.h"
#include <string>
#include <memory>
#include <filesystem>
#include <random>
#include <d3dcompiler.h>

#include "Renderer/Renderer.h"
#include "Renderer/RendererUtils.h"
#include "Renderer/SMAA/AreaTex.h"
#include "Renderer/SMAA/SearchTex.h"
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/configuration.h"
#include "Specific/memory/Vector.h"
#include "Specific/trutils.h"
#include "Specific/winmain.h"

extern GameConfiguration g_Configuration;

using namespace TEN::Renderer::Utils;

static std::wstring GetAssetPath(const wchar_t* fileName)
{
	return TEN::Utils::ToWString(g_GameFlow->GetGameDir()) + fileName;
}

namespace TEN::Renderer
{
	void Renderer::Initialize(int w, int h, bool windowed, HWND handle)
	{
		TENLog("Initializing DX11...", LogLevel::Info);

		_screenWidth = w;
		_screenHeight = h;
		_isWindowed = windowed;
		InitializeScreen(w, h, handle, false);
		InitializeCommonTextures();

		// Initialize render states
		_renderStates = std::make_unique<CommonStates>(_device.Get());

		// Load shaders
		const D3D_SHADER_MACRO roomDefinesAnimated[] = { "ANIMATED", "", nullptr, nullptr };
		const D3D_SHADER_MACRO roomDefinesShadowMap[] = { "SHADOW_MAP", "", nullptr, nullptr };

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

		_sRooms = CompileOrLoadShader("Rooms", "", ShaderType::PixelAndVertex);
		Utils::throwIfFailed(_device->CreateInputLayout(inputLayoutItems, 12, _sRooms.Vertex.Blob->GetBufferPointer(), _sRooms.Vertex.Blob->GetBufferSize(), &_inputLayout));

		_sRoomsAnimated = CompileOrLoadShader("Rooms", "", ShaderType::Vertex, &roomDefinesAnimated[0]);
		_sItems = CompileOrLoadShader("Items", "", ShaderType::PixelAndVertex);
		_sStatics = CompileOrLoadShader("Statics", "", ShaderType::PixelAndVertex);
		_sSky = CompileOrLoadShader("Sky", "", ShaderType::PixelAndVertex);
		_sSprites = CompileOrLoadShader("Sprites", "", ShaderType::PixelAndVertex);
		_sSolid = CompileOrLoadShader("Solid", "", ShaderType::PixelAndVertex);
		_sInventory = CompileOrLoadShader("Inventory", "", ShaderType::PixelAndVertex);
		_sFullScreenQuad = CompileOrLoadShader("FullScreenQuad", "", ShaderType::PixelAndVertex);
		_sShadowMap = CompileOrLoadShader("ShadowMap", "", ShaderType::PixelAndVertex, &roomDefinesShadowMap[0]);
		_sHUD = CompileOrLoadShader("HUD", "", ShaderType::Vertex);
		_sHUDColor = CompileOrLoadShader("HUD", "ColoredHUD", ShaderType::Pixel);
		_sHUDTexture = CompileOrLoadShader("HUD", "TexturedHUD", ShaderType::Pixel);
		_sHUDBarColor = CompileOrLoadShader("HUD", "TexturedHUDBar", ShaderType::Pixel);
		_sInstancedStatics = CompileOrLoadShader("InstancedStatics", "", ShaderType::PixelAndVertex);
		_sInstancedSprites = CompileOrLoadShader("InstancedSprites", "", ShaderType::PixelAndVertex);

		_sGBuffer = CompileOrLoadShader("GBuffer", "", ShaderType::Pixel);
		_sGBufferRooms = CompileOrLoadShader("GBuffer", "Rooms", ShaderType::Vertex);
		_sGBufferRoomsAnimated = CompileOrLoadShader("GBuffer", "Rooms", ShaderType::Vertex, &roomDefinesAnimated[0]);
		_sGBufferItems = CompileOrLoadShader("GBuffer", "Items", ShaderType::Vertex);
		_sGBufferStatics = CompileOrLoadShader("GBuffer", "Statics", ShaderType::Vertex);
		_sGBufferInstancedStatics = CompileOrLoadShader("GBuffer", "InstancedStatics", ShaderType::Vertex);

		_sRoomAmbient = CompileOrLoadShader("RoomAmbient", "", ShaderType::PixelAndVertex);
		_sRoomAmbientSky = CompileOrLoadShader("RoomAmbient", "Sky", ShaderType::Vertex);
		_sFXAA = CompileOrLoadShader("FXAA", "", ShaderType::Pixel);
		_sSSAO = CompileOrLoadShader("SSAO", "", ShaderType::Pixel);
		_sSSAOBlur = CompileOrLoadShader("SSAO", "Blur", ShaderType::Pixel);

		const D3D_SHADER_MACRO transparentDefines[] = { "TRANSPARENT", "", nullptr, nullptr };
		_sRoomsTransparent = CompileOrLoadShader("Rooms", "", ShaderType::Pixel, &transparentDefines[0]);

		// Initialize constant buffers
		_cbCameraMatrices = CreateConstantBuffer<CCameraMatrixBuffer>();
		_cbItem = CreateConstantBuffer<CItemBuffer>();
		_cbStatic = CreateConstantBuffer<CStaticBuffer>();
		_cbLights = CreateConstantBuffer<CLightBuffer>();
		_cbShadowMap = CreateConstantBuffer<CShadowLightBuffer>();
		_cbRoom = CreateConstantBuffer<CRoomBuffer>();
		_cbAnimated = CreateConstantBuffer<CAnimatedBuffer>();
		_cbPostProcessBuffer = CreateConstantBuffer<CPostProcessBuffer>();
		_cbBlending = CreateConstantBuffer<CBlendingBuffer>();
		_cbInstancedSpriteBuffer = CreateConstantBuffer<CInstancedSpriteBuffer>();
		_cbInstancedStaticMeshBuffer = CreateConstantBuffer<CInstancedStaticMeshBuffer>();
		_cbSMAABuffer = CreateConstantBuffer<CSMAABuffer>();

		// Prepare HUD Constant buffer  
		_cbHUDBar = CreateConstantBuffer<CHUDBarBuffer>();
		_cbHUD = CreateConstantBuffer<CHUDBuffer>();
		_cbSprite = CreateConstantBuffer<CSpriteBuffer>();
		_stHUD.View = Matrix::CreateLookAt(Vector3::Zero, Vector3(0, 0, 1), Vector3(0, -1, 0));
		_stHUD.Projection = Matrix::CreateOrthographicOffCenter(0, DISPLAY_SPACE_RES.x, 0, DISPLAY_SPACE_RES.y, 0, 1.0f);
		_cbHUD.UpdateData(_stHUD, _context.Get());
		_currentCausticsFrame = 0;

		// Preallocate lists
		_lines2DToDraw = createVector<RendererLine2D>(MAX_LINES_2D);
		_lines3DToDraw = createVector<RendererLine3D>(MAX_LINES_3D);
		_triangles3DToDraw = createVector<RendererTriangle3D>(MAX_TRIANGLES_3D);

		for (auto& dynamicLightList : _dynamicLights)
			dynamicLightList = createVector<RendererLight>(MAX_DYNAMIC_LIGHTS);

		for (auto& item : _items)
			item.LightsToDraw = createVector<RendererLight*>(MAX_LIGHTS_PER_ITEM);

		for (auto& effect : _effects)
			effect.LightsToDraw = createVector<RendererLight*>(MAX_LIGHTS_PER_ITEM);

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
		Utils::throwIfFailed(_device->CreateBlendState(&blendStateDesc, _subtractiveBlendState.GetAddressOf()));

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
		Utils::throwIfFailed(_device->CreateBlendState(&blendStateDesc, _screenBlendState.GetAddressOf()));

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
		Utils::throwIfFailed(_device->CreateBlendState(&blendStateDesc, _lightenBlendState.GetAddressOf()));
		 
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
		Utils::throwIfFailed(_device->CreateBlendState(&blendStateDesc, _excludeBlendState.GetAddressOf()));

		blendStateDesc.AlphaToCoverageEnable = false;
		blendStateDesc.IndependentBlendEnable = true;
		blendStateDesc.RenderTarget[0].BlendEnable = true;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		blendStateDesc.RenderTarget[1].BlendEnable = true;
		blendStateDesc.RenderTarget[1].SrcBlend = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[1].DestBlend = D3D11_BLEND_INV_SRC_COLOR;
		blendStateDesc.RenderTarget[1].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[1].SrcBlendAlpha = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[1].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[1].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[1].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED;
		Utils::throwIfFailed(_device->CreateBlendState(&blendStateDesc, _transparencyBlendState.GetAddressOf()));

		blendStateDesc.AlphaToCoverageEnable = false;
		blendStateDesc.IndependentBlendEnable = false;
		blendStateDesc.RenderTarget[0].BlendEnable = true;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		Utils::throwIfFailed(_device->CreateBlendState(&blendStateDesc, _finalTransparencyBlendState.GetAddressOf()));

		D3D11_SAMPLER_DESC shadowSamplerDesc = {};
		shadowSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		shadowSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		shadowSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		shadowSamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		shadowSamplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		Utils::throwIfFailed(_device->CreateSamplerState(&shadowSamplerDesc, _shadowSampler.GetAddressOf()));
		_shadowSampler->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof("ShadowSampler") + 1, "ShadowSampler");

		D3D11_RASTERIZER_DESC rasterizerStateDesc = {};

		rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
		rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerStateDesc.DepthClipEnable = true;
		rasterizerStateDesc.MultisampleEnable = true;
		rasterizerStateDesc.AntialiasedLineEnable = true;
		rasterizerStateDesc.ScissorEnable = true;
		Utils::throwIfFailed(_device->CreateRasterizerState(&rasterizerStateDesc, _cullCounterClockwiseRasterizerState.GetAddressOf()));

		rasterizerStateDesc.CullMode = D3D11_CULL_FRONT;
		rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerStateDesc.DepthClipEnable = true;
		rasterizerStateDesc.MultisampleEnable = true;
		rasterizerStateDesc.AntialiasedLineEnable = true;
		rasterizerStateDesc.ScissorEnable = true;
		Utils::throwIfFailed(_device->CreateRasterizerState(&rasterizerStateDesc, _cullClockwiseRasterizerState.GetAddressOf()));

		rasterizerStateDesc.CullMode = D3D11_CULL_NONE;
		rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerStateDesc.DepthClipEnable = true;
		rasterizerStateDesc.MultisampleEnable = true;
		rasterizerStateDesc.AntialiasedLineEnable = true;
		rasterizerStateDesc.ScissorEnable = true;
		Utils::throwIfFailed(_device->CreateRasterizerState(&rasterizerStateDesc, _cullNoneRasterizerState.GetAddressOf()));

		//_tempRoomAmbientRenderTarget1 = RenderTarget2D(_device.Get(), ROOM_AMBIENT_MAP_SIZE, ROOM_AMBIENT_MAP_SIZE, DXGI_FORMAT_R8G8B8A8_UNORM, false, DXGI_FORMAT_D24_UNORM_S8_UINT);
		//_tempRoomAmbientRenderTarget2 = RenderTarget2D(_device.Get(), ROOM_AMBIENT_MAP_SIZE, ROOM_AMBIENT_MAP_SIZE, DXGI_FORMAT_R8G8B8A8_UNORM, false, DXGI_FORMAT_D24_UNORM_S8_UINT);
		//_tempRoomAmbientRenderTarget3 = RenderTarget2D(_device.Get(), ROOM_AMBIENT_MAP_SIZE, ROOM_AMBIENT_MAP_SIZE, DXGI_FORMAT_R8G8B8A8_UNORM, false, DXGI_FORMAT_D24_UNORM_S8_UINT);
		//_tempRoomAmbientRenderTarget4 = RenderTarget2D(_device.Get(), ROOM_AMBIENT_MAP_SIZE, ROOM_AMBIENT_MAP_SIZE, DXGI_FORMAT_R8G8B8A8_UNORM, false, DXGI_FORMAT_D24_UNORM_S8_UINT);
		 
		_SMAAAreaTexture = Texture2D(_device.Get(), AREATEX_WIDTH, AREATEX_HEIGHT, DXGI_FORMAT_R8G8_UNORM, AREATEX_PITCH, areaTexBytes);
		_SMAASearchTexture = Texture2D(_device.Get(), SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, DXGI_FORMAT_R8_UNORM, SEARCHTEX_PITCH, searchTexBytes);

		CreateSSAONoiseTexture();
		InitializePostProcess();
		InitializeGameBars();
		InitializeSpriteQuad();
		InitializeSky();

		_roomAmbientMapFront = RenderTarget2D(_device.Get(), ROOM_AMBIENT_MAP_SIZE, ROOM_AMBIENT_MAP_SIZE, DXGI_FORMAT_R8G8B8A8_UNORM, false, DXGI_FORMAT_D32_FLOAT);
		_roomAmbientMapBack = RenderTarget2D(_device.Get(), ROOM_AMBIENT_MAP_SIZE, ROOM_AMBIENT_MAP_SIZE, DXGI_FORMAT_R8G8B8A8_UNORM, false, DXGI_FORMAT_D32_FLOAT);

		_sortedPolygonsVertices.reserve(MAX_TRANSPARENT_VERTICES);
		_sortedPolygonsIndices.reserve(MAX_TRANSPARENT_VERTICES);
		_sortedPolygonsVertexBuffer = VertexBuffer<Vertex>(_device.Get(), MAX_TRANSPARENT_VERTICES, _sortedPolygonsVertices);
		_sortedPolygonsIndexBuffer = IndexBuffer(_device.Get(), MAX_TRANSPARENT_VERTICES, _sortedPolygonsIndices);
	}

	void Renderer::InitializePostProcess()
	{
		PostProcessVertex vertices[3];

		vertices[0].Position = Vector3(-1.0f, -1.0f, 1.0f);
		vertices[1].Position = Vector3(-1.0f, 3.0f, 1.0f);
		vertices[2].Position = Vector3(3.0f, -1.0f, 1.0f);

		vertices[0].UV = Vector2(0.0f, 1.0f);
		vertices[1].UV = Vector2(0.0f, -1.0f);
		vertices[2].UV = Vector2(2.0f, 1.0f);

		_fullscreenTriangleVertexBuffer = VertexBuffer<PostProcessVertex>(_device.Get(), 3, &vertices[0]);

		_sPostProcess = CompileOrLoadShader("PostProcess", "", ShaderType::PixelAndVertex);

		D3D11_INPUT_ELEMENT_DESC postProcessInputLayoutItems[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		Utils::throwIfFailed(_device->CreateInputLayout(postProcessInputLayoutItems, 3, 
							 _sPostProcess.Vertex.Blob->GetBufferPointer(), _sPostProcess.Vertex.Blob->GetBufferSize(), &_fullscreenTriangleInputLayout));

		_sPostProcessMonochrome = CompileOrLoadShader("PostProcess", "Monochrome", ShaderType::Pixel);
		_sPostProcessNegative   = CompileOrLoadShader("PostProcess", "Negative",   ShaderType::Pixel);
		_sPostProcessExclusion  = CompileOrLoadShader("PostProcess", "Exclusion",  ShaderType::Pixel);
		_sPostProcessFinalPass  = CompileOrLoadShader("PostProcess", "FinalPass",  ShaderType::Pixel);
		_sPostProcessLensFlare  = CompileOrLoadShader("PostProcess", "LensFlare",  ShaderType::Pixel);
	}

	void Renderer::CreateSSAONoiseTexture()
	{
		std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
		std::default_random_engine generator;
		for (unsigned int i = 0; i < 64; ++i)
		{
			Vector4 sample(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator),
				1.0f
			);
			sample.Normalize();
			sample *= randomFloats(generator);

			float scale = (float)i / 64.0;
			scale = Lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			sample.w = 1.0f;

			_SSAOKernel.push_back(sample);
		}

		std::vector<Vector4> SSAONoise;
		for (unsigned int i = 0; i < 16; i++)
		{
			Vector4 noise(
				randomFloats(generator) * 2.0 - 1.0,
				randomFloats(generator) * 2.0 - 1.0,
				0.0f,
				1.0f);
			SSAONoise.push_back(noise);
		}

		_SSAONoiseTexture = Texture2D(_device.Get(), 4, 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 4 * sizeof(Vector4), SSAONoise.data());
	}

	void Renderer::InitializeSpriteQuad()
	{
		std::array<Vertex, 4> quadVertices;

		//Bottom Left
		quadVertices[0].Position = Vector3(-0.5, -0.5, 0);
		quadVertices[0].Normal = Vector3(-1, -1, 1);
		quadVertices[0].Normal.Normalize();
		quadVertices[0].UV = Vector2(0, 1);
		quadVertices[0].Color = Vector4(1, 1, 1, 1);
		quadVertices[0].IndexInPoly = 3;

		//Top Left 
		quadVertices[1].Position = Vector3(-0.5, 0.5, 0);
		quadVertices[1].Normal = Vector3(-1, 1, 1);
		quadVertices[1].Normal.Normalize();
		quadVertices[1].UV = Vector2(0, 0);
		quadVertices[1].Color = Vector4(1, 1, 1, 1);
		quadVertices[1].IndexInPoly = 0;

		//Top Right
		quadVertices[3].Position = Vector3(0.5, 0.5, 0);
		quadVertices[3].Normal = Vector3(1, 1, 1);
		quadVertices[3].Normal.Normalize();
		quadVertices[3].UV = Vector2(1, 0);
		quadVertices[3].Color = Vector4(1, 1, 1, 1);
		quadVertices[3].IndexInPoly = 1;

		//Bottom Right
		quadVertices[2].Position = Vector3(0.5, -0.5, 0);
		quadVertices[2].Normal = Vector3(1, -1, 1);
		quadVertices[2].Normal.Normalize();
		quadVertices[2].UV = Vector2(1, 1);
		quadVertices[2].Color = Vector4(1, 1, 1, 1);
		quadVertices[2].IndexInPoly = 2;

		_quadVertexBuffer = VertexBuffer<Vertex>(_device.Get(), 4, quadVertices.data());
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

		_skyVertexBuffer = VertexBuffer<Vertex>(_device.Get(), SKY_VERTICES_COUNT, vertices);
		_skyIndexBuffer = IndexBuffer(_device.Get(), SKY_INDICES_COUNT, indices);
	}

	void Renderer::InitializeScreen(int w, int h, HWND handle, bool reset)
	{
		DXGI_SWAP_CHAIN_DESC sd;
		sd.BufferDesc.Width = w;
		sd.BufferDesc.Height = h;
		if (!g_Configuration.EnableHighFramerate)
		{
			_refreshRate = 30;

			sd.BufferDesc.RefreshRate.Numerator = 0;
			sd.BufferDesc.RefreshRate.Denominator = 0;
		}
		else
		{
			_refreshRate = GetCurrentScreenRefreshRate();
			if (_refreshRate == 0)
			{
				_refreshRate = 60;
			}
			
			sd.BufferDesc.RefreshRate.Numerator = _refreshRate;
			sd.BufferDesc.RefreshRate.Denominator = 1;
		}
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
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
		Utils::throwIfFailed(_device.As(&dxgiDevice));

		ComPtr<IDXGIAdapter> dxgiAdapter;
		Utils::throwIfFailed(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), &dxgiAdapter));

		ComPtr<IDXGIFactory> dxgiFactory;
		Utils::throwIfFailed(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), &dxgiFactory));

		Utils::throwIfFailed(dxgiFactory->CreateSwapChain(_device.Get(), &sd, &_swapChain));

		dxgiFactory->MakeWindowAssociation(handle, DXGI_MWA_NO_ALT_ENTER);
 
		// Initialize render targets
		ID3D11Texture2D* backBufferTexture = NULL;
		Utils::throwIfFailed(_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast <void**>(&backBufferTexture)));
		_backBuffer = RenderTarget2D(_device.Get(), backBufferTexture, DXGI_FORMAT_D24_UNORM_S8_UINT);
		                
		_renderTarget = RenderTarget2D(_device.Get(), w, h, DXGI_FORMAT_R8G8B8A8_UNORM, false, DXGI_FORMAT_D24_UNORM_S8_UINT);
		_postProcessRenderTarget[0] = RenderTarget2D(_device.Get(), w, h, DXGI_FORMAT_R8G8B8A8_UNORM, false, DXGI_FORMAT_UNKNOWN);
		_postProcessRenderTarget[1] = RenderTarget2D(_device.Get(), w, h, DXGI_FORMAT_R8G8B8A8_UNORM, false, DXGI_FORMAT_UNKNOWN);
		_tempRenderTarget = RenderTarget2D(_device.Get(), w, h, DXGI_FORMAT_R8G8B8A8_UNORM, false, DXGI_FORMAT_UNKNOWN);
		_dumpScreenRenderTarget = RenderTarget2D(_device.Get(), w, h, DXGI_FORMAT_R8G8B8A8_UNORM, false, DXGI_FORMAT_D24_UNORM_S8_UINT);
		_shadowMap = Texture2DArray(_device.Get(), g_Configuration.ShadowMapSize, 6, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT);
		_depthRenderTarget = RenderTarget2D(_device.Get(), w, h, DXGI_FORMAT_R32_FLOAT, false, DXGI_FORMAT_UNKNOWN);
		_normalsRenderTarget = RenderTarget2D(_device.Get(), w, h, DXGI_FORMAT_R8G8B8A8_UNORM, false, DXGI_FORMAT_UNKNOWN);
		_SSAORenderTarget = RenderTarget2D(_device.Get(), w, h, DXGI_FORMAT_R8G8B8A8_UNORM, false, DXGI_FORMAT_UNKNOWN);
		_SSAOBlurredRenderTarget = RenderTarget2D(_device.Get(), w, h, DXGI_FORMAT_R8G8B8A8_UNORM, false, DXGI_FORMAT_UNKNOWN);

		// Initialize sprite and primitive batches
		_spriteBatch = std::make_unique<SpriteBatch>(_context.Get());
		_primitiveBatch = std::make_unique<PrimitiveBatch<Vertex>>(_context.Get());

		// Initialize viewport
		_viewport.TopLeftX = 0;
		_viewport.TopLeftY = 0;
		_viewport.Width = w;
		_viewport.Height = h;
		_viewport.MinDepth = 0.0f;
		_viewport.MaxDepth = 1.0f;

		_shadowMapViewport.TopLeftX = 0;
		_shadowMapViewport.TopLeftY = 0;
		_shadowMapViewport.Width = g_Configuration.ShadowMapSize;
		_shadowMapViewport.Height = g_Configuration.ShadowMapSize;
		_shadowMapViewport.MinDepth = 0.0f;
		_shadowMapViewport.MaxDepth = 1.0f;

		_viewportToolkit = Viewport(_viewport.TopLeftX, _viewport.TopLeftY, _viewport.Width, _viewport.Height,
			_viewport.MinDepth, _viewport.MaxDepth);

		// Low AA is done with FXAA, Medium - High AA are done with SMAA.
		if (g_Configuration.AntialiasingMode > AntialiasingMode::Low)
		{
			InitializeSMAA();
		}

		SetFullScreen();
	}

	void Renderer::InitializeSMAA()
	{
		int w = _screenWidth;
		int h = _screenHeight;

		_SMAASceneRenderTarget = RenderTarget2D(_device.Get(), w, h, DXGI_FORMAT_R8G8B8A8_UNORM, true, DXGI_FORMAT_UNKNOWN);
		_SMAASceneSRGBRenderTarget = RenderTarget2D(_device.Get(), &_SMAASceneRenderTarget, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
		_SMAAEdgesRenderTarget = RenderTarget2D(_device.Get(), w, h, DXGI_FORMAT_R8G8_UNORM, false, DXGI_FORMAT_UNKNOWN);
		_SMAABlendRenderTarget = RenderTarget2D(_device.Get(), w, h, DXGI_FORMAT_R8G8B8A8_UNORM, false, DXGI_FORMAT_UNKNOWN);

		auto string = std::stringstream{};
		auto defines = std::vector<D3D10_SHADER_MACRO>{};

		// Set up pixel size macro.
		string << "float4(1.0 / " << w << ", 1.0 / " << h << ", " << w << ", " << h << ")";
		auto pixelSizeText = string.str();
		auto renderTargetMetricsMacro = D3D10_SHADER_MACRO{ "SMAA_RT_METRICS", pixelSizeText.c_str() };
		defines.push_back(renderTargetMetricsMacro);

		if (g_Configuration.AntialiasingMode == AntialiasingMode::Medium)
		{
			defines.push_back({ "SMAA_PRESET_HIGH", nullptr });
		}
		else
		{
			defines.push_back({ "SMAA_PRESET_ULTRA", nullptr });
		}

		// defines.push_back({ "SMAA_PREDICATION", "1" });

		// Set up target macro.
		auto dx101Macro = D3D10_SHADER_MACRO{ "SMAA_HLSL_4_1", "1" };
		defines.push_back(dx101Macro);

		auto null = D3D10_SHADER_MACRO{ nullptr, nullptr };
		defines.push_back(null);

		_sSMAALumaEdgeDetection = CompileOrLoadShader("SMAA", "LumaEdgeDetection", ShaderType::Pixel, defines.data());
		_sSMAAColorEdgeDetection = CompileOrLoadShader("SMAA", "ColorEdgeDetection", ShaderType::Pixel, defines.data());
		_sSMAADepthEdgeDetection = CompileOrLoadShader("SMAA", "DepthEdgeDetection", ShaderType::Pixel, defines.data());
		_sSMAABlendingWeightCalculation = CompileOrLoadShader("SMAA", "BlendingWeightCalculation", ShaderType::PixelAndVertex, defines.data());
		_sSMAANeighborhoodBlending = CompileOrLoadShader("SMAA", "NeighborhoodBlending", ShaderType::PixelAndVertex, defines.data());
		_sSMAAEdgeDetection = CompileOrLoadShader("SMAA", "EdgeDetection", ShaderType::Vertex, defines.data());
	}

	void Renderer::InitializeCommonTextures()
	{
		// Initialize font.
		auto fontPath = GetAssetPath(L"Textures/Font.spritefont");
		if (!std::filesystem::is_regular_file(fontPath))
			throw std::runtime_error("Font not found; path " + TEN::Utils::ToString(fontPath) + " is missing.");
		     
		_gameFont = std::make_unique<SpriteFont>(_device.Get(), fontPath.c_str());

		// Initialize common textures.
		SetTextureOrDefault(_logo, GetAssetPath(L"Textures/Logo.png"));
		SetTextureOrDefault(_loadingBarBorder, GetAssetPath(L"Textures/LoadingBarBorder.png"));
		SetTextureOrDefault(_loadingBarInner, GetAssetPath(L"Textures/LoadingBarInner.png"));
		SetTextureOrDefault(_whiteTexture, GetAssetPath(L"Textures/WhiteSprite.png")); 

		_whiteSprite.Height = _whiteTexture.Height;
		_whiteSprite.Width = _whiteTexture.Width;
		_whiteSprite.UV[0] = Vector2(0.0f, 0.0f);
		_whiteSprite.UV[1] = Vector2(1.0f, 0.0f);
		_whiteSprite.UV[2] = Vector2(1.0f, 1.0f);
		_whiteSprite.UV[3] = Vector2(0.0f, 1.0f);
		_whiteSprite.Texture = &_whiteTexture;
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
				levels, 1, D3D11_SDK_VERSION, &_device, &featureLevel, &_context);
		}
		else
		{
			res = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL,
				levels, 1, D3D11_SDK_VERSION, &_device, &featureLevel, &_context);
		}

		Utils::throwIfFailed(res);
	}

	void Renderer::ToggleFullScreen(bool force)
	{
		_isWindowed = force ? false : !_isWindowed;
		SetFullScreen();
	}

	void Renderer::SetFullScreen()
	{
		if (!_isWindowed)
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
			SetWindowPos(WindowsHandle, HWND_TOP, 0, 0, _screenWidth + borderWidth, _screenHeight + borderHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
		}

		UpdateWindow(WindowsHandle);
	}

	void Renderer::BindShader(const RendererShader& shader)
	{
		if (shader.Vertex.Shader != nullptr)  _context->VSSetShader(shader.Vertex.Shader.Get(), nullptr, 0);
		if (shader.Pixel.Shader != nullptr)   _context->PSSetShader(shader.Pixel.Shader.Get(), nullptr, 0);
		if (shader.Compute.Shader != nullptr) _context->CSSetShader(shader.Compute.Shader.Get(), nullptr, 0);
	}

	RendererShader Renderer::CompileOrLoadShader(const std::string& fileName, const std::string& funcName, ShaderType type, const D3D_SHADER_MACRO* defines)
	{
		RendererShader result = {};

		// We need to increment the counter to avoid overwriting compiled shaders with the same source file name.
		static int compileCounter = 0;

		// Define paths for native (uncompiled) shaders and compiled shaders.
		std::wstring shaderPath = GetAssetPath(L"Shaders\\");
		std::wstring compiledShaderPath = shaderPath + L"Bin\\";
		std::wstring wideFileName = TEN::Utils::ToWString(fileName);

		// Ensure the /Bin subdirectory exists.
		std::filesystem::create_directories(compiledShaderPath);

		// Helper function to load or compile a shader.
		auto loadOrCompileShader = [this, type, defines, shaderPath, compiledShaderPath]
		(const std::wstring& baseFileName, const std::string& shaderType, const std::string& functionName, const char* model, ComPtr<ID3D10Blob>& bytecode)
		{
			// Construct the full paths using GetAssetPath.
			auto prefix = ((compileCounter < 10) ? L"0" : L"") + std::to_wstring(compileCounter) + L"_";
			auto csoFileName = compiledShaderPath + prefix + baseFileName + L"." + std::wstring(shaderType.begin(), shaderType.end()) + L".cso";
			auto srcFileName = shaderPath + baseFileName;

			// Try both .hlsl and .fx extensions for the source shader.
			auto srcFileNameWithExtension = srcFileName + L".hlsl";
			if (!std::filesystem::exists(srcFileNameWithExtension))
			{
				srcFileNameWithExtension = srcFileName + L".fx";
				if (!std::filesystem::exists(srcFileNameWithExtension))
				{
					TENLog("Shader source file not found: " + TEN::Utils::ToString(srcFileNameWithExtension), LogLevel::Error);
					throw std::runtime_error("Shader source file not found");
				}
			}

			// Check modification dates of the source and compiled files.
			bool shouldRecompile = false;
			if (std::filesystem::exists(csoFileName))
			{
				auto csoTime = std::filesystem::last_write_time(csoFileName);
				auto srcTime = std::filesystem::last_write_time(srcFileNameWithExtension);
				shouldRecompile = srcTime > csoTime; // Recompile if the source is newer.
			}

			// Load compiled shader if it exists and is up to date.
			if (!shouldRecompile)
			{
				std::ifstream csoFile(csoFileName, std::ios::binary);

				if (csoFile.is_open())
				{
					// Load compiled shader.
					csoFile.seekg(0, std::ios::end);
					size_t fileSize = csoFile.tellg();
					csoFile.seekg(0, std::ios::beg);

					std::vector<char> buffer(fileSize);
					csoFile.read(buffer.data(), fileSize);
					csoFile.close();

					D3DCreateBlob(fileSize, &bytecode);
					memcpy(bytecode->GetBufferPointer(), buffer.data(), fileSize);

					return;
				}
			}

			// Set up compilation flags according to the build configuration.
			unsigned int flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR;

			if constexpr (DebugBuild)
				flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
			else
				flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_IEEE_STRICTNESS;

			auto trimmedFileName = std::filesystem::path(srcFileNameWithExtension).filename().string();
			TENLog("Compiling shader: " + trimmedFileName, LogLevel::Info);

			// Compile shader.
			ComPtr<ID3D10Blob> errors;
			HRESULT res = D3DCompileFromFile(srcFileNameWithExtension.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
				(shaderType + functionName).c_str(), model, flags, 0, bytecode.GetAddressOf(), errors.GetAddressOf());

			if (FAILED(res))
			{
				if (errors)
				{
					auto error = std::string(static_cast<const char*>(errors->GetBufferPointer()));
					TENLog(error, LogLevel::Error);
					throw std::runtime_error(error);
				}
				else
				{
					TENLog("Error while compiling shader: " + trimmedFileName, LogLevel::Error);
					throwIfFailed(res);
				}
			}

			// Save compiled shader to .cso file.
			std::ofstream outCsoFile(csoFileName, std::ios::binary);
			if (outCsoFile.is_open())
			{
				outCsoFile.write(reinterpret_cast<const char*>(bytecode->GetBufferPointer()), bytecode->GetBufferSize());
				outCsoFile.close();
			}
		};

		// Load or compile and create pixel shader.
		if (type == ShaderType::Pixel || type == ShaderType::PixelAndVertex)
		{
			loadOrCompileShader(wideFileName, "PS", funcName, "ps_5_0", result.Pixel.Blob);
			throwIfFailed(_device->CreatePixelShader(result.Pixel.Blob->GetBufferPointer(), result.Pixel.Blob->GetBufferSize(),
				nullptr, result.Pixel.Shader.GetAddressOf()
			));
		}

		// Load or compile and create vertex shader.
		if (type == ShaderType::Vertex || type == ShaderType::PixelAndVertex)
		{
			loadOrCompileShader(wideFileName, "VS", funcName, "vs_5_0", result.Vertex.Blob);
			throwIfFailed(_device->CreateVertexShader(result.Vertex.Blob->GetBufferPointer(), result.Vertex.Blob->GetBufferSize(),
				nullptr, result.Vertex.Shader.GetAddressOf()
			));
		}

		// Load or compile and create compute shader.
		if (type == ShaderType::Compute)
		{
			loadOrCompileShader(wideFileName, "CS", funcName, "cs_5_0", result.Compute.Blob);
			throwIfFailed(_device->CreateComputeShader(result.Compute.Blob->GetBufferPointer(), result.Compute.Blob->GetBufferSize(),
				nullptr, result.Compute.Shader.GetAddressOf()
			));
		}

		// Increment the compile counter.
		compileCounter++;

		return result;
	}
}