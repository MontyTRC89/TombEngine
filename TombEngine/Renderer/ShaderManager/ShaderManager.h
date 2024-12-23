#pragma once
#include "framework.h"
#include "Renderer/Structures/RendererShader.h"

namespace TEN::Renderer::Utils
{
	using namespace TEN::Renderer::Structures;

	enum class Shader
	{
		None,
		Rooms,
		RoomsAnimated,
		RoomsTransparent,
		RoomAmbient,
		RoomAmbientSky,
		Items,
		Statics,
		InstancedStatics,
		Sprites,
		InstancedSprites,
		Sky,
		Solid,
		Inventory,
		FullScreenQuad,
		ShadowMap,

		HUD,
		HUDColor,
		HUDTexture,
		HUDBarColor,

		GBuffer,
		GBufferRooms,
		GBufferRoomsAnimated,
		GBufferItems,
		GBufferStatics,
		GBufferInstancedStatics,

		SMAAEdgeDetection,
		SMAALumaEdgeDetection,
		SMAAColorEdgeDetection,
		SMAADepthEdgeDetection,
		SMAABlendingWeightCalculation,
		SMAANeighborhoodBlending,
		FXAA,

		PostProcess,
		PostProcessMonochrome,
		PostProcessNegative,
		PostProcessExclusion,
		PostProcessFinalPass,
		PostProcessLensFlare,

		SSAO,
		SSAOBlur,

		Count
	};

	class ShaderManager
	{
		private:
			ComPtr<ID3D11Device> _device = nullptr;
			ComPtr<ID3D11DeviceContext> _context = nullptr;

			int _compileCounter = 0;
			std::array<RendererShader, (int)Shader::Count> _shaders = {};

			RendererShader LoadOrCompile(const std::string& fileName, const std::string& funcName, ShaderType type, const D3D_SHADER_MACRO* defines);
			void Load(Shader shader, const std::string& fileName, const std::string& funcName, ShaderType type, const D3D_SHADER_MACRO* defines = nullptr);
			void Destroy(Shader shader);

		public:
			ShaderManager() = default;
			~ShaderManager();

			void Initialize(ComPtr<ID3D11Device>& device, ComPtr<ID3D11DeviceContext>& context);
			void LoadAllShaders(int width, int height);

			void Bind(Shader shader);
			const RendererShader& Get(Shader shader);
	};
}