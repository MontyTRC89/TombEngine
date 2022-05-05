#pragma once
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
#include "Renderer/ConstantBuffers/AlphaTestBuffer.h"
#include "Frustum.h"
#include "RendererBucket.h"
#include "Game/items.h"
#include "Game/animation.h"
#include "Game/effects/effects.h"
#include "IndexBuffer/IndexBuffer.h"
#include "VertexBuffer/VertexBuffer.h"
#include "RenderTarget2D/RenderTarget2D.h"
#include "ConstantBuffers/CameraMatrixBuffer.h"
#include "Texture2D/Texture2D.h"
#include "ConstantBuffers/SpriteBuffer.h"
#include "RenderTargetCube/RenderTargetCube.h"
#include "RenderView/RenderView.h"
#include "Specific/level.h"
#include "ConstantBuffer/ConstantBuffer.h"
#include "RenderTargetCubeArray/RenderTargetCubeArray.h"
#include "Specific/fast_vector.h"
#include "Renderer/TextureBase.h"
#include "Renderer/ConstantBuffers/PostProcessBuffer.h"
#include <wrl/client.h>
#include <CommonStates.h>
#include <SpriteFont.h>
#include <PrimitiveBatch.h>
#include <d3d9types.h>
#include "Renderer/Structures/RendererBone.h"
#include "Renderer/Structures/RendererVideoAdapter.h"
#include "Renderer/Structures/RendererLight.h"
#include "Renderer/Structures/RendererStringToDraw.h"
#include "Renderer/Structures/RendererRoom.h"

struct CAMERA_INFO;

namespace TEN::Renderer
{
	using TexturePair = std::tuple<Texture2D, Texture2D>;

	struct RendererHUDBar
	{
		VertexBuffer VertexBufferBorder;
		IndexBuffer IndexBufferBorder;
		VertexBuffer InnerVertexBuffer;
		IndexBuffer InnerIndexBuffer;
		/*
			Initialises a new Bar for rendering. the Coordinates are set in the Reference Resolution (default 800x600).
			The colors are setup like this (4 triangles)
			0-------1
			| \   / |
			|  >2<  |
			| /   \ |
			3-------4
		*/
		RendererHUDBar(ID3D11Device* m_device, int x, int y, int w, int h, int borderSize,
		               std::array<Vector4, 5> colors);
	};

	struct RendererAnimatedTexture
	{
		Vector2 UV[4];
	};

	struct RendererAnimatedTextureSet
	{
		int NumTextures;
		std::vector<RendererAnimatedTexture> Textures;
	};

	struct RendererStatic
	{
		int Id;
		short RoomIndex;
		MESH_INFO* Mesh;
		Matrix World;
	};

	struct RendererRoomNode
	{
		short From;
		short To;
		Vector4 ClipPort;
	};

	struct RendererItem
	{
		short ItemNumber;
		bool DoneAnimations;
		Matrix World;
		Matrix Translation;
		Matrix Rotation;
		Matrix Scale;
		Matrix AnimationTransforms[32];
		std::vector<RendererLight*> LightsToDraw;
		int PreviousRoomNumber = NO_ROOM;
		int CurrentRoomNumber = NO_ROOM;
		Vector4 AmbientLight;
		int AmbientLightSteps;
	};

	struct RendererMesh
	{
		BoundingSphere Sphere;
		std::vector<RendererBucket> buckets;
		std::vector<Vector3> Positions;
	};

	struct RendererEffect
	{
		int Id;
		FX_INFO* Effect;
		Matrix World;
		RendererMesh* Mesh;
		std::vector<RendererLight*> Lights;
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
		Vector3 start;
		Vector3 end;
		Vector4 color;
	};

	struct RendererWeatherParticle
	{
		float X, Y, Z;
		float AngleH;
		float AngleV;
		float Size;
		short Room;
		bool Reset;
		bool Draw;
	};

	struct RendererUnderwaterDustParticle
	{
		float X, Y, Z;
		short Life;
		short Room;
		bool Reset;
	};

	struct RendererLine2D
	{
		Vector2 Vertices[2];
		Vector4 Color;
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
		std::vector<RendererVideoAdapter> m_adapters;

		// Main back buffer
		ID3D11RenderTargetView* m_backBufferRTV;
		ID3D11Texture2D* m_backBufferTexture;

		ID3D11DepthStencilState* m_depthStencilState;
		ID3D11DepthStencilView* m_depthStencilView;
		ID3D11Texture2D* m_depthStencilTexture;

		RenderTarget2D m_dumpScreenRenderTarget;
		RenderTarget2D m_renderTarget;
		RenderTarget2D m_currentRenderTarget;
		RenderTarget2D m_shadowMap;
		RenderTarget2D m_depthMap;
		RenderTargetCube m_reflectionCubemap;

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
		CAlphaTestBuffer m_stAlphaTest;
		ConstantBuffer<CAlphaTestBuffer> m_cbAlphaTest;

		// Text and sprites
		std::unique_ptr<SpriteFont> m_gameFont;
		std::unique_ptr<SpriteBatch> m_spriteBatch;
		std::vector<RendererStringToDraw> m_strings;
		int m_blinkColorValue;
		int m_blinkColorDirection;
		std::unique_ptr<PrimitiveBatch<RendererVertex>> m_primitiveBatch;
		int m_currentY;

		// System resources
		Texture2D m_HUDBarBorderTexture;
		Texture2D m_HUDBarTextures[4];
		std::vector<Texture2D> m_caustics;
		Texture2D m_binocularsTexture;
		Texture2D m_LasersightTexture;
		Texture2D m_whiteTexture;
		RenderTargetCubeArray m_shadowMaps;
		Texture2D loadingBarBorder;
		Texture2D loadingBarInner;
		Texture2D* loadingScreenTexture = nullptr;

		// Level data
		Texture2D m_titleScreen;
		Texture2D m_loadScreen;
		Texture2D m_textureAtlas;
		Texture2D m_skyTexture;
		Texture2D m_logo;

		VertexBuffer m_roomsVertexBuffer;
		IndexBuffer m_roomsIndexBuffer;
		VertexBuffer m_moveablesVertexBuffer;
		IndexBuffer m_moveablesIndexBuffer;
		VertexBuffer m_staticsVertexBuffer;
		IndexBuffer m_staticsIndexBuffer;

		std::vector<RendererVertex> roomsVertices;
		std::vector<int> roomsIndices;
		std::vector<RendererVertex> moveablesVertices;
		std::vector<int> moveablesIndices;
		std::vector<RendererVertex> staticsVertices;
		std::vector<int> staticsIndices;

		VertexBuffer m_transparentFacesVertexBuffer;
		IndexBuffer m_transparentFacesIndexBuffer;
		std::vector<RendererVertex> m_transparentFacesVertices;
		fast_vector<int> m_transparentFacesIndices;
		std::vector<RendererTransparentFace> m_transparentFaces;

		std::vector<RendererRoom> m_rooms;

		Matrix m_hairsMatrices[12];
		short m_numHairVertices;
		short m_numHairIndices;
		std::vector<RendererVertex> m_hairVertices;
		std::vector<short> m_hairIndices;

		std::vector<RendererLight> dynamicLights;
		RendererLight* shadowLight;

		std::vector<RendererLine3D> m_lines3DToDraw;
		std::vector<RendererLine2D> m_lines2DToDraw;

		int m_nextSprite;
		std::vector<std::optional<RendererObject>> m_moveableObjects;
		std::vector<std::optional<RendererObject>> m_staticObjects;
		std::vector<RendererSprite> m_sprites;
		int m_numMoveables;
		int m_numStatics;
		int m_numSprites;
		int m_numSpritesSequences;
		std::vector<RendererSpriteSequence> m_spriteSequences;
		std::unordered_map<int, RendererMesh*> m_meshPointersToMesh;
		Matrix m_LaraWorldMatrix;
		std::vector<RendererAnimatedTextureSet> m_animatedTextureSets;
		int m_numAnimatedTextureSets;
		int m_currentCausticsFrame;
		RendererUnderwaterDustParticle m_underwaterDustParticles[NUM_UNDERWATER_DUST_PARTICLES];
		bool m_firstUnderwaterDustParticles = true;
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
		int m_numTransparentDrawCalls = 0;
		int m_numRoomsTransparentDrawCalls = 0;
		int m_numMoveablesTransparentDrawCalls = 0;
		int m_numStaticsTransparentDrawCalls = 0;
		int m_numSpritesTransparentDrawCalls = 0;
		int m_biggestRoomIndexBuffer = 0;
		RENDERER_DEBUG_PAGE m_numDebugPage = NO_PAGE;
		int numRoomsTransparentPolygons;
		int m_numPolygons = 0;

		// Times for debug
		int m_timeUpdate;
		int m_timeDraw;
		int m_timeFrame;
		float m_fps;

		// Weather
		bool m_firstWeather;
		RendererWeatherParticle m_rain[NUM_RAIN_DROPS];
		RendererWeatherParticle m_snow[NUM_SNOW_PARTICLES];

		// Old fade-in/out
		RENDERER_FADE_STATUS m_fadeStatus = NO_FADE;
		float m_fadeFactor;
		int m_progress;

		// Misc
		int m_pickupRotation = 0;

		// Caching state changes
		TextureBase* lastTexture;
		BLEND_MODES lastBlendMode;
		DEPTH_STATES lastDepthState;
		CULL_MODES lastCullMode;

		// Rooms culling 
		bool m_outside = false;
		bool m_cameraUnderwater = false;
		short m_boundList[MAX_ROOM_BOUNDS];
		short m_boundStart = 0;
		short m_boundEnd = 1;
		RendererRectangle m_outsideClip;

		// Private functions
		void BindTexture(TEXTURE_REGISTERS registerType, TextureBase* texture, SAMPLER_STATES samplerType);
		void BindRenderTargetAsTexture(TEXTURE_REGISTERS registerType, RenderTarget2D* target, SAMPLER_STATES samplerType);
		void BindConstantBufferVS(CONSTANT_BUFFERS constantBufferType, ID3D11Buffer** buffer);
		void BindConstantBufferPS(CONSTANT_BUFFERS constantBufferType, ID3D11Buffer** buffer);
		
		void DrawAllStrings();
		void FromTrAngle(Matrix* matrix, short* frameptr, int index);
		void BuildHierarchy(RendererObject* obj);
		void BuildHierarchyRecursive(RendererObject* obj, RendererBone* node, RendererBone* parentNode);
		void UpdateAnimation(RendererItem* item, RendererObject& obj, ANIM_FRAME** frmptr, short frac, short rate,
		                     int mask, bool useObjectWorldRotation = false);
		bool PrintDebugMessage(int x, int y, int alpha, byte r, byte g, byte b, LPCSTR Message);
		void GetVisibleObjects(RenderView& renderView, bool onlyRooms);
		void GetRoomBounds(RenderView& renderView, bool onlyRooms);
		void SetRoomBounds(ROOM_DOOR* door, short parentRoomNumber, RenderView& renderView);
		void CollectRooms(RenderView& renderView, bool onlyRooms);
		Vector4 GetPortalRect(Vector4 v, Vector4 vp);
		void CollectItems(short roomNumber, RenderView& renderView);
		void CollectStatics(short roomNumber, RenderView& renderView);
		void CollectLightsForEffect(short roomNumber, RendererEffect* effect, RenderView& renderView);
		void CollectLightsForItem(short roomNumber, RendererItem* item, RenderView& renderView);
		void CollectLightsForRoom(short roomNumber, RenderView& renderView);
		void CollectEffects(short roomNumber, RenderView& renderView);
		void ClearSceneItems();
		void UpdateItemsAnimations(RenderView& view);
		void UpdateEffects(RenderView& view);
		bool SphereBoxIntersection(Vector3 boxMin, Vector3 boxMax,
		                           Vector3 sphereCentre, float sphereRadius);
		void DrawHorizonAndSky(RenderView& renderView, ID3D11DepthStencilView* depthTarget);
		void DrawRooms(RenderView& view, bool transparent);
		void DrawRoomsTransparent(RendererTransparentFaceInfo* info, RenderView& view);
		void DrawSpritesTransparent(RendererTransparentFaceInfo* info, RenderView& view);
		void DrawStaticsTransparent(RendererTransparentFaceInfo* info, RenderView& view);
		void DrawItems(RenderView& view, bool transparent);
		void DrawItemsTransparent(RendererTransparentFaceInfo* info, RenderView& view);
		void DrawAnimatingItem(RendererItem* item, RenderView& view, bool transparent);
		void DrawBaddieGunflashes(RenderView& view);
		void DrawStatics(RenderView& view, bool transparent);
		void RenderShadowMap(RenderView& view);
		void DrawWraithExtra(RendererItem* item, RenderView& view);
		void DrawDarts(RendererItem* item, RenderView& view);
		void DrawLara(bool shadowMap, RenderView& view, bool transparent);
		void DrawFires(RenderView& view);
		void DrawSparks(RenderView& view);
		void DrawSmokes(RenderView& view);
		void DrawLightning(RenderView& view);
		void DrawBlood(RenderView& view);
		void DrawWeatherParticles(RenderView& view);
		void DrawDrips(RenderView& view);
		void DrawBubbles(RenderView& view);
		void DrawEffects(RenderView& view, bool transparent);
		void DrawEffect(RenderView& view, RendererEffect* effect, bool transparent);
		void DrawSplashes(RenderView& view);
		void DrawSprites(RenderView& view);
		void DrawTransparentFaces(RenderView& view);
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
		void RenderInventoryScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget,
		                          ID3D11ShaderResourceView* background);
		void RenderTitleMenu();
		void RenderPauseMenu();
		void RenderLoadSaveMenu();
		void RenderNewInventory();
		void DrawStatistics();
		void DrawExamines();
		void DrawDiary();
		void DrawDebris(RenderView& view, bool transparent);
		void DrawFullScreenImage(ID3D11ShaderResourceView* texture, float fade, ID3D11RenderTargetView* target,
		                         ID3D11DepthStencilView* depthTarget);
		void DrawShockwaves(RenderView& view);
		void DrawRipples(RenderView& view);
		void DrawUnderwaterDust(RenderView& view);
		void DrawFullScreenQuad(ID3D11ShaderResourceView* texture, Vector3 color);
		bool IsRoomUnderwater(short roomNumber);
		bool IsInRoom(int x, int y, int z, short roomNumber);
		void InitialiseScreen(int w, int h, bool windowed, HWND handle, bool reset);
		void InitialiseBars();
		void DrawSmokeParticles(RenderView& view);
		void DrawSparkParticles(RenderView& view);
		void DrawDripParticles(RenderView& view);
		void DrawExplosionParticles(RenderView& view);
		void RenderToCubemap(const RenderTargetCube& dest, const Vector3& pos, int roomNumber);
		void DrawLaraHolsters(bool transparent);
		void DrawMoveableMesh(RendererItem* itemToDraw, RendererMesh* mesh, RendererRoom* room, int boneIndex, bool transparent);
		void DrawSimpleParticles(RenderView& view);
		void SetBlendMode(BLEND_MODES blendMode, bool force = false);
		void SetDepthState(DEPTH_STATES depthState, bool force = false);
		void SetCullMode(CULL_MODES cullMode, bool force = false);
		void SetAlphaTest(ALPHA_TEST_MODES mode, float threshold, bool force = false);
		void SetScissor(RendererRectangle rectangle);
		void ResetScissor();
		float CalculateFrameRate();
		void AddSpriteBillboard(RendererSprite* sprite, Vector3 pos, Vector4 color, float rotation, float scale,
		                        Vector2 size, BLEND_MODES blendMode, RenderView& view);
		void AddSpriteBillboardConstrained(RendererSprite* sprite, Vector3 pos, Vector4 color, float rotation,
		                                   float scale, Vector2 size, BLEND_MODES blendMode, Vector3 constrainAxis,
		                                   RenderView& view);
		void AddSpriteBillboardConstrainedLookAt(RendererSprite* sprite, Vector3 pos, Vector4 color, float rotation,
		                                         float scale, Vector2 size, BLEND_MODES blendMode, Vector3 lookAtAxis,
		                                         RenderView& view);
		void addSprite3D(RendererSprite* sprite, Vector3 vtx1, Vector3 vtx2, Vector3 vtx3, Vector3 vtx4, Vector4 color,
		                 float rotation, float scale, Vector2 size, BLEND_MODES blendMode, RenderView& view);
		short GetRoomNumberForSpriteTest(Vector3 position);
		void DoFadingAndCinematicBars(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget,
		                              RenderView& view);
		RendererMesh* GetMesh(int meshIndex);
		Texture2D CreateDefaultNormalTexture();
		void DrawFootprints(RenderView& view);
		void DrawLoadingBar(float percent);
		Vector2 GetScreenResolution();

		inline void DrawIndexedTriangles(int count, int baseIndex, int baseVertex)
		{
			m_context->DrawIndexed(count, baseIndex, baseVertex);
			m_numPolygons += count / 3;
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
			return (blendMode == BLENDMODE_ALPHABLEND
				|| blendMode == BLENDMODE_EXCLUDE
				|| blendMode == BLENDMODE_LIGHTEN
				|| blendMode == BLENDMODE_SCREEN
				|| blendMode == BLENDMODE_SUBTRACTIVE
				|| blendMode == BLENDMODE_NOZTEST);
		}

	public:
		Matrix View;
		Matrix Projection;
		Matrix ViewProjection;
		float FieldOfView;
		int ScreenWidth;
		int ScreenHeight;
		bool Windowed;
		int NumTexturePages;
		Renderer11();
		~Renderer11();

		RendererMesh* GetRendererMeshFromTrMesh(RendererObject* obj, MESH* meshPtr, short boneIndex, int isJoints, int isHairs, int* lastVertex, int* lastIndex);
		void DrawBar(float percent, const RendererHUDBar* bar, GAME_OBJECT_ID textureSlot, int frame, bool poison);
		void Create();
		std::vector<Vector2Int> EnumerateScreenResolutions();
		void Initialise(int w, int h, bool windowed, HWND handle);
		void Draw();
		bool PrepareDataForTheRenderer();
		void UpdateCameraMatrices(CAMERA_INFO* cam, float roll, float fov);
		void RenderSimpleScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget, RenderView& view);
		void DumpGameScene();
		void RenderInventory();
		void RenderTitle();
		void RenderScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget, RenderView& view);
		void ClearScene();
		void PrintDebugMessage(LPCSTR message, ...);
		void DrawDebugInfo(RenderView& view);
		void SwitchDebugPage(bool back);
		void drawPickup(short objectNum);
		int SyncRenderer();
		void DrawString(int x, int y, const char* string, D3DCOLOR color, int flags);
		void ClearDynamicLights();
		void AddDynamicLight(int x, int y, int z, short falloff, byte r, byte g, byte b);
		void FreeRendererData();
		void RenderLoadingScreen(float percentage);
		void UpdateProgress(float value);
		void GetLaraBonePosition(Vector3* pos, int bone);
		void ToggleFullScreen();
		bool IsFullsScreen();
		std::vector<RendererVideoAdapter>* GetAdapters();
		void RenderTitleImage();
		void AddLine2D(int x1, int y1, int x2, int y2, byte r, byte g, byte b, byte a);
		void AddLine3D(Vector3 start, Vector3 end,
		               Vector4 color);
		void AddBox(Vector3 min, Vector3 max, Vector4 color);
		void AddBox(Vector3* corners, Vector4 color);
		void AddDebugBox(BoundingOrientedBox box, Vector4 color, RENDERER_DEBUG_PAGE page);
		void AddDebugBox(Vector3 min, Vector3 max, Vector4 color, RENDERER_DEBUG_PAGE page);
		void AddSphere(Vector3 center, float radius, Vector4 color);
		void AddDebugSphere(Vector3 center, float radius, Vector4 color, RENDERER_DEBUG_PAGE page);
		void ChangeScreenResolution(int width, int height, bool windowed);
		void FlipRooms(short roomNumber1, short roomNumber2);
		void ResetAnimations();
		void UpdateLaraAnimations(bool force);
		void UpdateItemAnimations(int itemNumber, bool force);
		void GetLaraAbsBonePosition(Vector3* pos, int joint);
		void GetItemAbsBonePosition(int itemNumber, Vector3* pos, int joint);
		int getSpheres(short itemNumber, BoundingSphere* ptr, char worldSpace, Matrix local);
		void GetBoneMatrix(short itemNumber, int joint, Matrix* outMatrix);
		void DrawObjectOn2DPosition(short x, short y, short objectNum, float rotX, float rotY, float rotZ,
		                            float scale1);
		void SetLoadingScreen(std::wstring& fileName);
		std::string GetDefaultAdapterName();
	};

	extern Renderer11 g_Renderer;
}
