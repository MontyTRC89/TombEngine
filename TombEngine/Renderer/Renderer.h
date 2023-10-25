#pragma once
#include <wrl/client.h>
#include <CommonStates.h>
#include <SpriteFont.h>
#include <PrimitiveBatch.h>
#include <d3d9types.h>

#include "Math/Math.h"
#include "Game/control/box.h"
#include "Game/items.h"
#include "Game/animation.h"
#include "Game/Gui.h"
#include "Game/Hud/Hud.h"
#include "Game/Hud/PickupSummary.h"
#include "Game/effects/effects.h"
#include "Game/effects/Electricity.h"
#include "Specific/level.h"
#include "Specific/fast_vector.h"
#include "Renderer/Renderer11Enums.h"
#include "Renderer/Structures/RendererLight.h"
#include "RenderView/RenderView.h"
#include "Renderer/ConstantBuffers/StaticBuffer.h"
#include "Renderer/ConstantBuffers/LightBuffer.h"
#include "Renderer/ConstantBuffers/MiscBuffer.h"
#include "Renderer/ConstantBuffers/HUDBarBuffer.h"
#include "Renderer/ConstantBuffers/HUDBuffer.h"
#include "Renderer/ConstantBuffers/ShadowLightBuffer.h"
#include "Renderer/ConstantBuffers/RoomBuffer.h"
#include "Renderer/ConstantBuffers/ItemBuffer.h"
#include "Renderer/ConstantBuffers/AnimatedBuffer.h"
#include "Renderer/ConstantBuffers/BlendingBuffer.h"
#include "Renderer/ConstantBuffers/CameraMatrixBuffer.h"
#include "Renderer/ConstantBuffers/SpriteBuffer.h"
#include "Renderer/ConstantBuffers/InstancedStaticBuffer.h"
#include "Renderer/ConstantBuffers/InstancedSpriteBuffer.h"
#include "Frustum.h"
#include "Specific/level.h"
#include "ConstantBuffers/ConstantBuffer.h"
#include "Specific/fast_vector.h"
#include "Renderer/ConstantBuffers/InstancedSpriteBuffer.h"
#include "Renderer/ConstantBuffers/PostProcessBuffer.h"
#include "Renderer/Structures/RendererBone.h"
#include "Renderer/Structures/RendererStringToDraw.h"
#include "Renderer/Structures/RendererRoom.h"
#include "Renderer/Structures/RendererSprite.h"
#include "Renderer/Graphics/VertexBuffer.h"
#include "Renderer/Structures/RendererDoor.h"
#include "Renderer/ConstantBuffers/SkyBuffer.h"
#include "Structures/RendererAnimatedTexture.h"
#include "Structures/RendererAnimatedTextureSet.h"
#include "Structures/RendererRoom.h"
#include "Structures/RendererTransparentFace.h"
#include "Renderer/Graphics/Texture2D.h"
#include "Renderer/Graphics/IndexBuffer.h"
#include "Renderer/Graphics/RenderTarget2D.h"
#include "Renderer/Graphics/RenderTargetCube.h"
#include "Renderer/Graphics/Texture2DArray.h"
#include "Renderer/Structures/RendererItem.h"
#include "Renderer/Structures/RendererEffect.h"
#include "Renderer/Structures/RendererLine3D.h"
#include "Renderer/Structures/RendererMesh.h"
#include "Renderer/Structures/RendererSpriteSequence.h"
#include "Renderer/Structures/RendererLine2D.h"

enum GAME_OBJECT_ID : short;
class EulerAngles;
struct AnimFrameInterpData;
struct CAMERA_INFO;

namespace TEN::Renderer
{
	using namespace TEN::Effects::Electricity;
	using namespace TEN::Gui;
	using namespace TEN::Hud;
	using namespace TEN::Renderer::Graphics;
	using namespace TEN::Renderer::ConstantBuffers;
	using namespace TEN::Renderer::Structures;

	using TexturePair = std::tuple<Texture2D, Texture2D>;

	struct RendererHudBar
	{
		static constexpr auto COLOR_COUNT  = 5;
		static constexpr auto SIZE_DEFAULT = Vector2(150.0f, 10.0f);

		VertexBuffer VertexBufferBorder = VertexBuffer();
		IndexBuffer	 IndexBufferBorder	= IndexBuffer();
		VertexBuffer InnerVertexBuffer	= VertexBuffer();
		IndexBuffer	 InnerIndexBuffer	= IndexBuffer();

		/*
			Initializes status bar for rendering. Coordinates are set in screen space.
			Colors are set in 5 vertices as described in the diagram:
			0-------1
			| \   / |
			|  >2<  |
			| /   \ |
			3-------4
		*/
		RendererHudBar(ID3D11Device* devicePtr, const Vector2& pos, const Vector2& size, int borderSize, std::array<Vector4, COLOR_COUNT> colors);
	};

	class Renderer11
	{
	private:
		// Core DX11 objects
		ComPtr<ID3D11Device> m_device = nullptr;
		ComPtr<ID3D11DeviceContext> m_context = nullptr;
		ComPtr<IDXGISwapChain> m_swapChain = nullptr;
		std::unique_ptr<CommonStates> m_states = nullptr;
		ComPtr<ID3D11BlendState> m_subtractiveBlendState = nullptr;
		ComPtr<ID3D11BlendState> m_screenBlendState = nullptr;
		ComPtr<ID3D11BlendState> m_lightenBlendState = nullptr;
		ComPtr<ID3D11BlendState> m_excludeBlendState = nullptr;
		ComPtr<ID3D11RasterizerState> m_cullCounterClockwiseRasterizerState = nullptr;
		ComPtr<ID3D11RasterizerState> m_cullClockwiseRasterizerState = nullptr;
		ComPtr<ID3D11RasterizerState> m_cullNoneRasterizerState = nullptr;
		ComPtr<ID3D11InputLayout> m_inputLayout = nullptr;
		D3D11_VIEWPORT m_viewport;
		D3D11_VIEWPORT m_shadowMapViewport;
		Viewport m_viewportToolkit;

		// Main back buffer
		ID3D11RenderTargetView* m_backBufferRTV;
		ID3D11Texture2D* m_backBufferTexture;
		ID3D11DepthStencilState* m_depthStencilState;
		ID3D11DepthStencilView* m_depthStencilView;
		ID3D11Texture2D* m_depthStencilTexture;

		// Render targets
		RenderTarget2D m_dumpScreenRenderTarget;
		RenderTarget2D m_renderTarget;
		RenderTarget2D m_currentRenderTarget;
		RenderTarget2D m_depthMap;
		RenderTargetCube m_reflectionCubemap;
		Texture2DArray m_shadowMap;

		// Shaders
		ComPtr<ID3D11VertexShader> m_vsRooms;
		ComPtr<ID3D11VertexShader> m_vsRooms_Anim;
		ComPtr<ID3D11PixelShader> m_psRooms;
		ComPtr<ID3D11VertexShader> m_vsItems;
		ComPtr<ID3D11PixelShader> m_psItems;
		ComPtr<ID3D11VertexShader> m_vsHairs;
		ComPtr<ID3D11PixelShader> m_psHairs;
		ComPtr<ID3D11VertexShader> m_vsStatics;
		ComPtr<ID3D11PixelShader> m_psStatics;
		ComPtr<ID3D11VertexShader> m_vsSky;
		ComPtr<ID3D11PixelShader> m_psSky;
		ComPtr<ID3D11VertexShader> m_vsSprites;
		ComPtr<ID3D11PixelShader> m_psSprites;
		ComPtr<ID3D11VertexShader> m_vsInstancedSprites;
		ComPtr<ID3D11PixelShader> m_psInstancedSprites;
		ComPtr<ID3D11VertexShader> m_vsInstancedStaticMeshes;
		ComPtr<ID3D11PixelShader> m_psInstancedStaticMeshes;
		ComPtr<ID3D11VertexShader> m_vsSolid;
		ComPtr<ID3D11PixelShader> m_psSolid;
		ComPtr<ID3D11VertexShader> m_vsInventory;
		ComPtr<ID3D11PixelShader> m_psInventory;
		ComPtr<ID3D11VertexShader> m_vsFullScreenQuad;
		ComPtr<ID3D11PixelShader> m_psFullScreenQuad;
		ComPtr<ID3D11VertexShader> m_vsShadowMap;
		ComPtr<ID3D11PixelShader> m_psShadowMap;
		ComPtr<ID3D11VertexShader> m_vsHUD;
		ComPtr<ID3D11PixelShader> m_psHUDColor;
		ComPtr<ID3D11PixelShader> m_psHUDTexture;
		ComPtr<ID3D11PixelShader> m_psHUDBarColor;
		ComPtr<ID3D11SamplerState> m_shadowSampler;
		ComPtr<ID3D11VertexShader> m_vsFinalPass;
		ComPtr<ID3D11PixelShader> m_psFinalPass;

		// Constant buffers
		RenderView gameCamera;
		ConstantBuffer<CCameraMatrixBuffer> m_cbCameraMatrices;
		CItemBuffer m_stItem;
		ConstantBuffer<CItemBuffer> m_cbItem;
		CStaticBuffer m_stStatic;
		ConstantBuffer<CStaticBuffer> m_cbStatic;
		CLightBuffer m_stLights;
		ConstantBuffer<CLightBuffer> m_cbLights;
		CMiscBuffer m_stMisc;
		ConstantBuffer<CMiscBuffer> m_cbMisc;
		CRoomBuffer m_stRoom;
		ConstantBuffer<CRoomBuffer> m_cbRoom;
		CAnimatedBuffer m_stAnimated;
		ConstantBuffer<CAnimatedBuffer> m_cbAnimated;
		CShadowLightBuffer m_stShadowMap;
		ConstantBuffer<CShadowLightBuffer> m_cbShadowMap;
		CHUDBuffer m_stHUD;
		ConstantBuffer<CHUDBuffer> m_cbHUD;
		CHUDBarBuffer m_stHUDBar;
		ConstantBuffer<CHUDBarBuffer> m_cbHUDBar;
		CSpriteBuffer m_stSprite;
		ConstantBuffer<CSpriteBuffer> m_cbSprite;
		CPostProcessBuffer m_stPostProcessBuffer;
		ConstantBuffer<CPostProcessBuffer> m_cbPostProcessBuffer;
		CInstancedSpriteBuffer m_stInstancedSpriteBuffer;
		ConstantBuffer<CInstancedSpriteBuffer> m_cbInstancedSpriteBuffer;
		CBlendingBuffer m_stBlending;
		ConstantBuffer<CBlendingBuffer> m_cbBlending;
		CInstancedStaticMeshBuffer m_stInstancedStaticMeshBuffer;
		ConstantBuffer<CInstancedStaticMeshBuffer> m_cbInstancedStaticMeshBuffer;
		CSkyBuffer m_stSky;
		ConstantBuffer<CSkyBuffer> m_cbSky;

		// Primitive batchs
		std::unique_ptr<SpriteBatch> m_spriteBatch;
		std::unique_ptr<PrimitiveBatch<Vertex>> m_primitiveBatch;

		// Text
		std::unique_ptr<SpriteFont> m_gameFont;
		std::vector<RendererStringToDraw> m_strings;
		float m_blinkColorValue = 0.0f;
		float m_blinkTime		  = 0.0f;
		bool  m_isBlinkUpdated  = false;

		// Graphics resources
		Texture2D m_logo;
		Texture2D m_skyTexture;
		Texture2D m_whiteTexture;
		RendererSprite m_whiteSprite;
		Texture2D m_loadingBarBorder;
		Texture2D m_loadingBarInner;
		Texture2D m_loadingScreenTexture;

		VertexBuffer m_roomsVertexBuffer;
		IndexBuffer m_roomsIndexBuffer;
		VertexBuffer m_moveablesVertexBuffer;
		IndexBuffer m_moveablesIndexBuffer;
		VertexBuffer m_staticsVertexBuffer;
		IndexBuffer m_staticsIndexBuffer;
		VertexBuffer m_skyVertexBuffer;
		IndexBuffer m_skyIndexBuffer;

		// TODO: used by legacy transaprency, to remove in the new system
		std::vector<Vertex> m_roomsVertices;
		std::vector<int> m_roomsIndices;
		std::vector<Vertex> m_moveablesVertices;
		std::vector<int> m_moveablesIndices;
		std::vector<Vertex> m_staticsVertices;
		std::vector<int> m_staticsIndices;
		VertexBuffer m_transparentFacesVertexBuffer;
		IndexBuffer m_transparentFacesIndexBuffer;
		std::vector<Vertex> m_transparentFacesVertices;
		fast_vector<int> m_transparentFacesIndices;
		std::vector<RendererTransparentFace> m_transparentFaces;

		// Rooms and collector
		std::vector<RendererRoom> m_rooms;
		bool m_invalidateCache;
		std::vector<short> m_visitedRoomsStack;

		// Lights
		std::vector<RendererLight> m_dynamicLights;
		RendererLight* m_shadowLight;

		// Lines
		std::vector<RendererLine3D> m_lines3DToDraw;
		std::vector<RendererLine2D> m_lines2DToDraw;

		// Textures, objects and sprites
		std::vector<std::optional<RendererObject>> m_moveableObjects;
		std::vector<std::optional<RendererObject>> m_staticObjects;
		std::vector<RendererSprite> m_sprites;
		std::vector<RendererSpriteSequence> m_spriteSequences;
		std::unordered_map<int, RendererMesh*> m_meshPointersToMesh;
		Matrix m_LaraWorldMatrix;
		std::vector<RendererAnimatedTextureSet> m_animatedTextureSets;
		int m_currentCausticsFrame;
		std::vector<RendererMesh*> m_meshes;
		std::vector<TexturePair> m_roomTextures;
		std::vector<TexturePair> m_animatedTextures;
		std::vector<TexturePair> m_moveablesTextures;
		std::vector<TexturePair> m_staticsTextures;
		std::vector<Texture2D> m_spritesTextures;

		// Preallocated pools of objects for avoiding new/delete
		// Items and effects are safe (can't be more than 1024 items in TR), 
		// lights should be oversized (eventually ignore lights more than MAX_LIGHTS)
		RendererItem m_items[NUM_ITEMS];
		RendererEffect m_effects[NUM_ITEMS];

		// Debug variables
		int m_numDrawCalls = 0;
		int m_numRoomsDrawCalls = 0;
		int m_numMoveablesDrawCalls = 0;
		int m_numStaticsDrawCalls = 0;
		int m_numSpritesDrawCalls = 0;
		int m_numTransparentDrawCalls = 0;
		int m_numRoomsTransparentDrawCalls = 0;
		int m_numMoveablesTransparentDrawCalls = 0;
		int m_numStaticsTransparentDrawCalls = 0;
		int m_numSpritesTransparentDrawCalls = 0;
		int m_biggestRoomIndexBuffer = 0;
		int m_numRoomsTransparentPolygons;
		int m_numPolygons = 0;
		int m_currentY;
		int m_dotProducts = 0;
		int m_numCheckPortalCalls = 0;
		int m_numGetVisibleRoomsCalls = 0;

		RendererDebugPage DebugPage = RendererDebugPage::None;

		// Times for debug
		int m_timeUpdate;
		int m_timeRoomsCollector;
		int m_timeDraw;
		int m_timeFrame;
		float m_fps;

		// Screen settings
		int m_screenWidth;
		int m_screenHeight;
		bool m_windowed;
		float m_farView = DEFAULT_FAR_VIEW;

		// A flag to prevent extra renderer object addition
		bool m_locked = false;

		// Caching state changes
		TextureBase* lastTexture;
		BLEND_MODES m_lastBlendMode;
		DEPTH_STATES m_lastDepthState;
		CULL_MODES m_lastCullMode;

		



		// Private functions
		void BindTexture(TEXTURE_REGISTERS registerType, TextureBase* texture, SAMPLER_STATES samplerType);
		void BindRoomLights(std::vector<RendererLight*>& lights);
		void BindStaticLights(std::vector<RendererLight*>& lights);
		void BindInstancedStaticLights(std::vector<RendererLight*>& lights, int instanceID);
		void BindMoveableLights(std::vector<RendererLight*>& lights, int roomNumber, int prevRoomNumber, float fade);
		void BindRenderTargetAsTexture(TEXTURE_REGISTERS registerType, RenderTarget2D* target, SAMPLER_STATES samplerType);
		void BindConstantBufferVS(CONSTANT_BUFFERS constantBufferType, ID3D11Buffer** buffer);
		void BindConstantBufferPS(CONSTANT_BUFFERS constantBufferType, ID3D11Buffer** buffer);
		
		void BuildHierarchy(RendererObject* obj);
		void BuildHierarchyRecursive(RendererObject* obj, RendererBone* node, RendererBone* parentNode);
		void UpdateAnimation(RendererItem* item, RendererObject& obj, const AnimFrameInterpData& frameData, int mask, bool useObjectWorldRotation = false);
		bool CheckPortal(short parentRoomNumber, RendererDoor* door, Vector4 viewPort, Vector4* clipPort, RenderView& renderView);
		void GetVisibleRooms(short from, short to, Vector4 viewPort, bool water, int count, bool onlyRooms, RenderView& renderView);
		void CollectRooms(RenderView& renderView, bool onlyRooms);
		void CollectItems(short roomNumber, RenderView& renderView);
		void CollectStatics(short roomNumber, RenderView& renderView);
		void CollectLights(Vector3 position, float radius, int roomNumber, int prevRoomNumber, bool prioritizeShadowLight, bool useCachedRoomLights, std::vector<RendererLight*>* roomsLights, std::vector<RendererLight*>* outputLights);
		void CollectLightsForItem(RendererItem* item);
		void CollectLightsForEffect(short roomNumber, RendererEffect* effect);
		void CollectLightsForRoom(short roomNumber, RenderView& renderView);
		void CollectLightsForCamera();
		void CalculateLightFades(RendererItem* item);
		void CollectEffects(short roomNumber);
		void ClearSceneItems();
		void ClearDynamicLights();
		void ClearShadowMap();
		void UpdateItemAnimations(RenderView& view);
		bool PrintDebugMessage(int x, int y, int alpha, byte r, byte g, byte b, LPCSTR Message);

		void InitializeScreen(int w, int h, HWND handle, bool reset);
		void InitializeCommonTextures();
		void InitializeGameBars();
		void InitializeMenuBars(int y);
		void InitializeSky();

		void DrawAllStrings();
		void DrawLaserBarriers(RenderView& view);
		void DrawHorizonAndSky(RenderView& renderView, ID3D11DepthStencilView* depthTarget);
		void DrawRooms(RenderView& view, RendererPass rendererPass);
		void DrawRoomsSorted(RendererTransparentFaceInfo* info, bool resetPipeline, RenderView& view);
		void DrawSpritesSorted(RendererTransparentFaceInfo* info, bool resetPipeline, RenderView& view);
		void DrawStaticsSorted(RendererTransparentFaceInfo* info, bool resetPipeline, RenderView& view);
		void DrawItems(RenderView& view, RendererPass rendererPass);
		void DrawItemsSorted(RendererTransparentFaceInfo* info, bool resetPipeline, RenderView& view);
		void DrawAnimatingItem(RendererItem* item, RenderView& view, RendererPass rendererPass);
		void DrawWaterfalls(RendererItem* item, RenderView& view, int fps, RendererPass rendererPass);
		void DrawBaddyGunflashes(RenderView& view);
		void DrawStatics(RenderView& view, RendererPass rendererPass);
		void DrawLara(RenderView& view, RendererPass rendererPass);
		void DrawFires(RenderView& view);
		void DrawParticles(RenderView& view);
		void DrawSmokes(RenderView& view);
		void DrawElectricity(RenderView& view);
		void DrawHelicalLasers(RenderView& view);
		void DrawBlood(RenderView& view);
		void DrawWeatherParticles(RenderView& view);
		void DrawDrips(RenderView& view);
		void DrawBubbles(RenderView& view);
		void DrawEffects(RenderView& view, RendererPass rendererPass);
		void DrawEffect(RenderView& view, RendererEffect* effect, RendererPass rendererPass);
		void DrawSplashes(RenderView& view);
		void DrawSprites(RenderView& view);
		void DrawDisplaySprites(RenderView& view);
		void DrawSortedFaces(RenderView& view);
		void DrawLines3D(RenderView& view);
		void DrawLinesIn2DSpace();
		void DrawOverlays(RenderView& view);
		void DrawRopes(RenderView& view);
		void DrawBats(RenderView& view);
		void DrawRats(RenderView& view);
		void DrawScarabs(RenderView& view);
		void DrawSpiders(RenderView& view);
		bool DrawGunFlashes(RenderView& view);
		void DrawGunShells(RenderView& view);
		void DrawLocusts(RenderView& view);
		void DrawStatistics();
		void DrawExamines();
		void DrawDiary();
		void DrawDebris(RenderView& view, RendererPass rendererPass);
		void DrawFullScreenImage(ID3D11ShaderResourceView* texture, float fade, ID3D11RenderTargetView* target,
		                         ID3D11DepthStencilView* depthTarget);
		void DrawShockwaves(RenderView& view);
		void DrawRipples(RenderView& view);
		void DrawUnderwaterBloodParticles(RenderView& view);
		void DrawFullScreenQuad(ID3D11ShaderResourceView* texture, Vector3 color, bool fit = true);
		void DrawFullScreenSprite(RendererSprite* sprite, DirectX::SimpleMath::Vector3 color, bool fit = true);
		void DrawSmokeParticles(RenderView& view);
		void DrawSparkParticles(RenderView& view);
		void DrawExplosionParticles(RenderView& view);
		void DrawLaraHolsters(RendererItem* itemToDraw, RendererRoom* room, RendererPass rendererPass);
		void DrawLaraJoints(RendererItem* itemToDraw, RendererRoom* room, RendererPass rendererPass);
		void DrawLaraHair(RendererItem* itemToDraw, RendererRoom* room, RendererPass rendererPass);
		void DrawMoveableMesh(RendererItem* itemToDraw, RendererMesh* mesh, RendererRoom* room, int boneIndex, RendererPass rendererPass);
		void DrawSimpleParticles(RenderView& view);
		void DrawStreamers(RenderView& view);
		void DrawFootprints(RenderView& view);
		void DrawLoadingBar(float percent);
		void DrawPostprocess(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget, RenderView& view);
		
		void RenderInventoryScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget,
			ID3D11ShaderResourceView* background);
		void RenderTitleMenu(Menu menu);
		void RenderPauseMenu(Menu menu);
		void RenderLoadSaveMenu();
		void RenderOptionsMenu(Menu menu, int initialY);
		void RenderNewInventory();
		void RenderToCubemap(const RenderTargetCube& dest, const Vector3& pos, int roomNumber); 
		void RenderBlobShadows(RenderView& renderView);
		void RenderShadowMap(RendererItem* item, RenderView& view);
		void RenderItemShadows(RenderView& renderView);

		void SetBlendMode(BLEND_MODES blendMode, bool force = false);
		void SetDepthState(DEPTH_STATES depthState, bool force = false);
		void SetCullMode(CULL_MODES cullMode, bool force = false);
		void SetAlphaTest(ALPHA_TEST_MODES mode, float threshold, bool force = false);
		void SetScissor(RendererRectangle rectangle);
		void ResetAnimations();
		void ResetScissor();
		void ResetDebugVariables();
		float CalculateFrameRate();

		void AddSpriteBillboard(RendererSprite* sprite, const Vector3& pos, const Vector4& color, float orient2D, float scale,
					 Vector2 size, BLEND_MODES blendMode, bool isSoftParticle, RenderView& view, SpriteRenderType renderType = SpriteRenderType::Default);
		void AddSpriteBillboardConstrained(RendererSprite* sprite, const Vector3& pos, const Vector4& color, float orient2D,
					 float scale, Vector2 size, BLEND_MODES blendMode, const Vector3& constrainAxis,
					 bool isSoftParticle, RenderView& view, SpriteRenderType renderType = SpriteRenderType::Default);
		void AddSpriteBillboardConstrainedLookAt(RendererSprite* sprite, const Vector3& pos, const Vector4& color, float orient2D,
					 float scale, Vector2 size, BLEND_MODES blendMode, const Vector3& lookAtAxis,
					 bool isSoftParticle, RenderView& view, SpriteRenderType renderType = SpriteRenderType::Default);
		void AddQuad(RendererSprite* sprite, const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& vertex3,
					 const Vector4 color, float orient2D, float scale, Vector2 size, BLEND_MODES blendMode, bool softParticles,
					 RenderView& view);
		void AddQuad(RendererSprite* sprite, const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& vertex3,
					 const Vector4& color0, const Vector4& color1, const Vector4& color2, const Vector4& color3, float orient2D,
					 float scale, Vector2 size, BLEND_MODES blendMode, bool isSoftParticle, RenderView& view, SpriteRenderType renderType = SpriteRenderType::Default);
		void AddColoredQuad(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& vertex3,
							const Vector4& color, BLEND_MODES blendMode, RenderView& view);
		void AddColoredQuad(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& vertex3,
							const Vector4& color0, const Vector4& color1, const Vector4& color2, const Vector4& color3,
							BLEND_MODES blendMode, RenderView& view, SpriteRenderType renderType = SpriteRenderType::Default);
		Matrix GetWorldMatrixForSprite(RendererSpriteToDraw* spr, RenderView& view);

		RendererObject& GetRendererObject(GAME_OBJECT_ID id);
		RendererMesh* GetMesh(int meshIndex);
		Texture2D CreateDefaultNormalTexture();

		Vector4 GetPortalRect(Vector4 v, Vector4 vp);
		bool SphereBoxIntersection(BoundingBox box, Vector3 sphereCentre, float sphereRadius);

		inline void DrawIndexedTriangles(int count, int baseIndex, int baseVertex)
		{
			m_context->DrawIndexed(count, baseIndex, baseVertex);
			m_numPolygons += count / 3;
			m_numDrawCalls++;
		}

		inline void DrawIndexedInstancedTriangles(int count, int instances, int baseIndex, int baseVertex)
		{
			m_context->DrawIndexedInstanced(count, instances, baseIndex, baseVertex, 0);
			m_numPolygons += count / 3 * instances;
			m_numDrawCalls++;
		}

		inline void DrawInstancedTriangles(int count, int instances, int baseVertex)
		{
			m_context->DrawInstanced(count, instances, baseVertex, 0);
			m_numPolygons += count / 3 * instances;
			m_numDrawCalls++;
		}

		inline void DrawTriangles(int count, int baseVertex)
		{
			m_context->Draw(count, baseVertex);
			m_numPolygons += count / 3;
			m_numDrawCalls++;
		}

		template <typename C>
		ConstantBuffer<C> CreateConstantBuffer()
		{
			return ConstantBuffer<C>(m_device.Get());
		}

		static bool DoesBlendModeRequireSorting(BLEND_MODES blendMode)
		{
			return (blendMode == BLENDMODE_ALPHABLEND ||
				    blendMode == BLENDMODE_EXCLUDE ||
				    blendMode == BLENDMODE_LIGHTEN ||
				    blendMode == BLENDMODE_SCREEN ||
				    blendMode == BLENDMODE_SUBTRACTIVE ||
				    blendMode == BLENDMODE_NOZTEST);
		}

	public:
		Renderer11();
		~Renderer11();

		RendererMesh* GetRendererMeshFromTrMesh(RendererObject* obj, MESH* meshPtr, short boneIndex, int isJoints, int isHairs, int* lastVertex, int* lastIndex);
		void DrawBar(float percent, const RendererHudBar& bar, GAME_OBJECT_ID textureSlot, int frame, bool poison);
		void Create();
		void Initialize(int w, int h, bool windowed, HWND handle);
		void Render();
		void RenderTitle();
		void Lock();
		bool PrepareDataForTheRenderer();
		void UpdateCameraMatrices(CAMERA_INFO* cam, float roll, float fov, float farView);
		void RenderSimpleScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget, RenderView& view);
		void DumpGameScene();
		void RenderInventory();
		void RenderScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget, RenderView& view);
		void ClearScene();
		void SaveScreenshot();
		void PrintDebugMessage(LPCSTR message, ...);
		void DrawDebugInfo(RenderView& view);
		void SwitchDebugPage(bool goBack);
		void DrawDisplayPickup(const DisplayPickup& pickup);
		int  Synchronize();
		void AddString(int x, int y, const std::string& string, D3DCOLOR color, int flags);
		void AddString(const std::string& string, const Vector2& pos, const Color& color, float scale, int flags);
		void AddDebugString(const std::string& string, const Vector2& pos, const Color& color, float scale, int flags, RendererDebugPage page);
		void FreeRendererData();
		void AddDynamicLight(int x, int y, int z, short falloff, byte r, byte g, byte b);
		void RenderLoadingScreen(float percentage);
		void UpdateProgress(float value);
		void ToggleFullScreen(bool force = false);
		void SetFullScreen();
		bool IsFullsScreen();
		void RenderTitleImage();
		void AddLine2D(const Vector2& origin, const Vector2& target, const Color& color);
		void AddLine3D(const Vector3& origin, const Vector3& target, const Vector4& color);
		void AddReticle(const Vector3& center, float radius, const Vector4& color);
		void AddDebugReticle(const Vector3& center, float radius, const Vector4& color, RendererDebugPage page);
		void AddBox(const Vector3 min, const Vector3& max, const Vector4& color);
		void AddBox(Vector3* corners, const Vector4& color);
		void AddDebugBox(const BoundingOrientedBox& box, const Vector4& color, RendererDebugPage page);
		void AddDebugBox(const Vector3& min, const Vector3& max, const Vector4& color, RendererDebugPage page);
		void AddSphere(const Vector3& center, float radius, const Vector4& color);
		void AddDebugSphere(const Vector3& center, float radius, const Vector4& color, RendererDebugPage page);
		void ChangeScreenResolution(int width, int height, bool windowed);
		void FlipRooms(short roomNumber1, short roomNumber2);
		void UpdateLaraAnimations(bool force);
		void UpdateItemAnimations(int itemNumber, bool force);
		int  GetSpheres(short itemNumber, BoundingSphere* ptr, char worldSpace, Matrix local);
		void GetBoneMatrix(short itemNumber, int jointIndex, Matrix* outMatrix);
		void DrawObjectIn2DSpace(int objectNumber, Vector2 pos2D, EulerAngles orient, float scale1, float opacity = 1.0f, int meshBits = NO_JOINT_BITS);
		void SetLoadingScreen(std::wstring& fileName);
		void SetTextureOrDefault(Texture2D& texture, std::wstring path);
		std::string GetDefaultAdapterName();

		Vector2i			   GetScreenResolution() const;
		std::optional<Vector2> Get2DPosition(const Vector3& pos) const;
		Vector3				   GetAbsEntityBonePosition(int itemNumber, int jointIndex, const Vector3& relOffset = Vector3::Zero);

		void AddDisplaySprite(const RendererSprite& sprite, const Vector2& pos2D, short orient, const Vector2& size, const Vector4& color,
							  int priority, BLEND_MODES blendMode, const Vector2& aspectCorrection, RenderView& renderView);
		void CollectDisplaySprites(RenderView& renderView);
	};

	extern Renderer11 g_Renderer;
}
