#pragma once
#include <wrl/client.h>
#include <CommonStates.h>
#include <SpriteFont.h>
#include <PrimitiveBatch.h>
#include <d3d9types.h>

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
#include "Frustum.h"
#include "RendererBucket.h"
#include "Renderer/RenderTargetCube/RenderTargetCube.h"
#include "RenderView/RenderView.h"
#include "Specific/level.h"
#include "ConstantBuffer/ConstantBuffer.h"
#include "RenderTargetCubeArray/RenderTargetCubeArray.h"
#include "Specific/fast_vector.h"
#include "Renderer/TextureBase.h"
#include "Renderer/Texture2DArray/Texture2DArray.h"
#include "Renderer/ConstantBuffers/InstancedSpriteBuffer.h"
#include "Renderer/ConstantBuffers/PostProcessBuffer.h"
#include "Renderer/Structures/RendererBone.h"
#include "Renderer/Structures/RendererLight.h"
#include "Renderer/Structures/RendererStringToDraw.h"
#include "Renderer/Structures/RendererRoom.h"
#include "Renderer/VertexBuffer/VertexBuffer.h"
#include "Renderer/IndexBuffer/IndexBuffer.h"
#include "Renderer/Texture2D/Texture2D.h"
#include "Renderer/RenderTarget2D/RenderTarget2D.h"
#include "Renderer/Structures/RendererDoor.h"

class EulerAngles;
struct AnimFrameInterpData;
struct CAMERA_INFO;
struct RendererRectangle;

using namespace TEN::Effects::Electricity;
using namespace TEN::Gui;
using namespace TEN::Hud;

namespace TEN::Renderer
{
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

	struct RendererAnimatedTexture
	{
		Vector2 UV[4];
	};

	struct RendererAnimatedTextureSet
	{
		int NumTextures;
		int Fps;
		std::vector<RendererAnimatedTexture> Textures;
	};

	struct RendererStatic
	{
		int ObjectNumber;
		int RoomNumber;
		int IndexInRoom;
		Pose Pose;
		Matrix World;
		Vector4 Color;
		Vector4 AmbientLight;
		std::vector<RendererLight*> LightsToDraw;
		std::vector<RendererLight*> CachedRoomLights;
		bool CacheLights;
		GameBoundingBox OriginalVisibilityBox;
		BoundingOrientedBox VisibilityBox;
		float Scale;

		void Update()
		{
			World = (Pose.Orientation.ToRotationMatrix() *
				Matrix::CreateScale(Scale) *
				Matrix::CreateTranslation(Pose.Position.x, Pose.Position.y, Pose.Position.z));
			CacheLights = true;
			VisibilityBox = OriginalVisibilityBox.ToBoundingOrientedBox(Pose);
		}
	};

	struct RendererRoomNode
	{
		short From;
		short To;
		Vector4 ClipPort;
	};

	struct RendererItem
	{
		int ItemNumber;
		int ObjectNumber;

		Vector3 Position;
		Matrix World;
		Matrix Translation;
		Matrix Rotation;
		Matrix Scale;
		Matrix AnimationTransforms[MAX_BONES];

		int RoomNumber = NO_ROOM;
		int PrevRoomNumber = NO_ROOM;
		Vector4 Color;
		Vector4 AmbientLight;
		std::vector<RendererLight*> LightsToDraw;
		float LightFade;

		std::vector<int> MeshIndex;

		bool DoneAnimations;
	};

	struct RendererMesh
	{
		LIGHT_MODES LightMode;
		BoundingSphere Sphere;
		std::vector<RendererBucket> Buckets;
		std::vector<Vector3> Positions;
	};

	struct RendererEffect
	{
		int ObjectNumber;
		int RoomNumber;
		Vector3 Position;
		Matrix World;
		Vector4 Color;
		Vector4 AmbientLight;
		RendererMesh* Mesh;
		std::vector<RendererLight*> LightsToDraw;
	};

	struct RendererObject
	{
		int Id;
		int Type;
		std::vector<RendererMesh*> ObjectMeshes;
		RendererBone* Skeleton;
		std::vector<Matrix> AnimationTransforms;
		std::vector<Matrix> BindPoseTransforms;
		std::vector<RendererBone*> LinearizedBones;
		bool DoNotDraw;
		ShadowMode ShadowType;
		bool HasDataInBucket[NUM_BUCKETS];
		bool HasDataInAnimatedBucket[NUM_BUCKETS];

		~RendererObject()
		{
			for (int i = 0; i < LinearizedBones.size(); i++)
				delete LinearizedBones[i];
		}
	};

	struct RendererSprite
	{
		int Index;
		int Width;
		int Height;
		Vector2 UV[4];
		Texture2D* Texture;
	};

	struct RendererSpriteSequence
	{
		int Id;
		std::vector<RendererSprite*> SpritesList;
		int NumSprites;

		RendererSpriteSequence()
		{
			Id = 0;
			NumSprites = 0;
		}

		RendererSpriteSequence(int id, int num)
		{
			Id = id;
			NumSprites = num;
			SpritesList = std::vector<RendererSprite*>(NumSprites);
		}

		RendererSpriteSequence(const RendererSpriteSequence& rhs)
		{
			Id = rhs.Id;
			NumSprites = rhs.NumSprites;
			SpritesList = rhs.SpritesList;
		}

		RendererSpriteSequence& operator=(const RendererSpriteSequence& other)
		{
			if (this != &other)
			{
				Id = other.Id;
				NumSprites = other.NumSprites;
				SpritesList = std::vector<RendererSprite*>(NumSprites);
				std::copy(other.SpritesList.begin(), other.SpritesList.end(), back_inserter(SpritesList));
			}
			return *this;
		}
	};

	struct RendererLine3D
	{
		Vector3 Start;
		Vector3 End;
		Vector4 Color;
	};

	struct RendererLine2D
	{
		Vector2 Origin = Vector2::Zero;
		Vector2 Target = Vector2::Zero;
		Vector4 Color  = Vector4::Zero;
	};

	struct RendererRect2D
	{
		RECT Rectangle;
		Vector4 Color;
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

		// Sprites
		std::unique_ptr<SpriteBatch> m_spriteBatch;
		std::unique_ptr<PrimitiveBatch<RendererVertex>> m_primitiveBatch;

		// Text
		std::unique_ptr<SpriteFont> m_gameFont;
		std::vector<RendererStringToDraw> m_strings;
		int m_blinkColorValue;
		int m_blinkColorDirection;
		bool m_blinkUpdated = false;

		// System resources
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

		std::vector<RendererVertex> m_roomsVertices;
		std::vector<int> m_roomsIndices;
		std::vector<RendererVertex> m_moveablesVertices;
		std::vector<int> m_moveablesIndices;
		std::vector<RendererVertex> m_staticsVertices;
		std::vector<int> m_staticsIndices;

		VertexBuffer m_transparentFacesVertexBuffer;
		IndexBuffer m_transparentFacesIndexBuffer;
		std::vector<RendererVertex> m_transparentFacesVertices;
		fast_vector<int> m_transparentFacesIndices;
		std::vector<RendererTransparentFace> m_transparentFaces;

		std::vector<RendererRoom> m_rooms;
		bool m_invalidateCache;

		std::vector<RendererLight> m_dynamicLights;
		RendererLight* m_shadowLight;

		std::vector<RendererLine3D> m_lines3DToDraw;
		std::vector<RendererLine2D> m_lines2DToDraw;

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

		RENDERER_DEBUG_PAGE m_numDebugPage = NO_PAGE;

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

		// A flag to prevent extra renderer object addition
		bool m_Locked = false;

		// Caching state changes
		TextureBase* lastTexture;
		BLEND_MODES lastBlendMode;
		DEPTH_STATES lastDepthState;
		CULL_MODES lastCullMode;

		float m_farView = DEFAULT_FAR_VIEW;

		int m_numCheckPortalCalls;
		int m_numGetVisibleRoomsCalls;

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

		void DrawAllStrings();
		void DrawLaserBarriers(RenderView& view);
		void DrawHorizonAndSky(RenderView& renderView, ID3D11DepthStencilView* depthTarget);
		void DrawRooms(RenderView& view, bool transparent);
		void DrawRoomsSorted(RendererTransparentFaceInfo* info, bool resetPipeline, RenderView& view);
		void DrawSpritesSorted(RendererTransparentFaceInfo* info, bool resetPipeline, RenderView& view);
		void DrawStaticsSorted(RendererTransparentFaceInfo* info, bool resetPipeline, RenderView& view);
		void DrawItems(RenderView& view, bool transparent);
		void DrawItemsSorted(RendererTransparentFaceInfo* info, bool resetPipeline, RenderView& view);
		void DrawAnimatingItem(RendererItem* item, RenderView& view, bool transparent);
		void DrawWaterfalls(RendererItem* item, RenderView& view, int fps, bool transparent);
		void DrawBaddyGunflashes(RenderView& view);
		void DrawStatics(RenderView& view, bool transparent);
		void DrawLara(RenderView& view, bool transparent);
		void DrawFires(RenderView& view);
		void DrawParticles(RenderView& view);
		void DrawSmokes(RenderView& view);
		void DrawElectricity(RenderView& view);
		void DrawHelicalLasers(RenderView& view);
		void DrawBlood(RenderView& view);
		void DrawWeatherParticles(RenderView& view);
		void DrawDrips(RenderView& view);
		void DrawBubbles(RenderView& view);
		void DrawEffects(RenderView& view, bool transparent);
		void DrawEffect(RenderView& view, RendererEffect* effect, bool transparent);
		void DrawSplashes(RenderView& view);
		void DrawSprites(RenderView& view);
		void DrawSortedFaces(RenderView& view);
		void DrawLines3D(RenderView& view);
		void DrawLines2D();
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
		void DrawDebris(RenderView& view, bool transparent);
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
		void DrawLaraHolsters(RendererItem* itemToDraw, RendererRoom* room, bool transparent);
		void DrawLaraJoints(RendererItem* itemToDraw, RendererRoom* room, bool transparent);
		void DrawLaraHair(RendererItem* itemToDraw, RendererRoom* room, bool transparent);
		void DrawMoveableMesh(RendererItem* itemToDraw, RendererMesh* mesh, RendererRoom* room, int boneIndex, bool transparent);
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
		void PrintDebugMessage(LPCSTR message, ...);
		void DrawDebugInfo(RenderView& view);
		void SwitchDebugPage(bool back);
		void DrawDisplayPickup(const DisplayPickup& pickup);
		int  Synchronize();
		void AddString(int x, int y, const char* string, D3DCOLOR color, int flags);
		void AddString(const std::string& string, const Vector2& pos, const Color& color, float scale, int flags);
		void FreeRendererData();
		void AddDynamicLight(int x, int y, int z, short falloff, byte r, byte g, byte b);
		void RenderLoadingScreen(float percentage);
		void UpdateProgress(float value);
		void ToggleFullScreen(bool force = false);
		void SetFullScreen();
		bool IsFullsScreen();
		void RenderTitleImage();
		void AddLine2D(const Vector2& origin, const Vector2& target, const Color& color);
		void AddLine3D(Vector3 start, Vector3 end, Vector4 color);
		void AddBox(Vector3 min, Vector3 max, Vector4 color);
		void AddBox(Vector3* corners, Vector4 color);
		void AddDebugBox(BoundingOrientedBox box, Vector4 color, RENDERER_DEBUG_PAGE page);
		void AddDebugBox(Vector3 min, Vector3 max, Vector4 color, RENDERER_DEBUG_PAGE page);
		void AddSphere(Vector3 center, float radius, Vector4 color);
		void AddDebugSphere(Vector3 center, float radius, Vector4 color, RENDERER_DEBUG_PAGE page);
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

		Vector2i GetScreenResolution() const;
		Vector2	 GetScreenSpacePosition(const Vector3& pos) const;
		Vector3	 GetAbsEntityBonePosition(int itemNumber, int jointIndex, const Vector3& relOffset = Vector3::Zero);
	};

	extern Renderer11 g_Renderer;
}
