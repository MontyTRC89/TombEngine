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
#include "Renderer/RendererEnums.h"
#include "Renderer/Structures/RendererLight.h"
#include "Renderer/RenderView.h"
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
#include "Renderer/Structures/RendererHudBar.h"

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

	class Renderer
	{
	private:
		// Core DX11 objects
		ComPtr<ID3D11Device> _device = nullptr;
		ComPtr<ID3D11DeviceContext> _context = nullptr;
		ComPtr<IDXGISwapChain> _swapChain = nullptr;
		std::unique_ptr<CommonStates> _renderStates = nullptr;
		ComPtr<ID3D11BlendState> _subtractiveBlendState = nullptr;
		ComPtr<ID3D11BlendState> _screenBlendState = nullptr;
		ComPtr<ID3D11BlendState> _lightenBlendState = nullptr;
		ComPtr<ID3D11BlendState> _excludeBlendState = nullptr;
		ComPtr<ID3D11BlendState> _transparencyBlendState = nullptr;
		ComPtr<ID3D11BlendState> _finalTransparencyBlendState = nullptr;
		ComPtr<ID3D11RasterizerState> _cullCounterClockwiseRasterizerState = nullptr;
		ComPtr<ID3D11RasterizerState> _cullClockwiseRasterizerState = nullptr;
		ComPtr<ID3D11RasterizerState> _cullNoneRasterizerState = nullptr;
		ComPtr<ID3D11InputLayout> _inputLayout = nullptr;
		D3D11_VIEWPORT _viewport;
		D3D11_VIEWPORT _shadowMapViewport;
		Viewport _viewportToolkit;

		// Render targets
		RenderTarget2D _backBuffer;
		RenderTarget2D _dumpScreenRenderTarget;
		RenderTarget2D _renderTarget;
		RenderTarget2D _depthMap;
		RenderTarget2D _transparencyRenderTarget;
		RenderTarget2D _weightRenderTarget;
		RenderTargetCube _reflectionCubemap;
		Texture2DArray _shadowMap;

		// Shaders
		ComPtr<ID3D11VertexShader> _vsRooms;
		ComPtr<ID3D11VertexShader> _vsRoomsAnimatedTextures;
		ComPtr<ID3D11PixelShader> _psRooms;
		ComPtr<ID3D11PixelShader> _psRoomsTransparent;
		ComPtr<ID3D11VertexShader> _vsItems;
		ComPtr<ID3D11PixelShader> _psItems;
		ComPtr<ID3D11VertexShader> _vsHairs;
		ComPtr<ID3D11PixelShader> _psHairs;
		ComPtr<ID3D11VertexShader> _vsStatics;
		ComPtr<ID3D11PixelShader> _psStatics;
		ComPtr<ID3D11VertexShader> _vsSky;
		ComPtr<ID3D11PixelShader> _psSky;
		ComPtr<ID3D11VertexShader> _vsSprites;
		ComPtr<ID3D11PixelShader> _psSprites;
		ComPtr<ID3D11VertexShader> _vsInstancedSprites;
		ComPtr<ID3D11PixelShader> _psInstancedSprites;
		ComPtr<ID3D11VertexShader> _vsInstancedStaticMeshes;
		ComPtr<ID3D11PixelShader> _psInstancedStaticMeshes;
		ComPtr<ID3D11VertexShader> _vsSolid;
		ComPtr<ID3D11PixelShader> _psSolid;
		ComPtr<ID3D11VertexShader> _vsInventory;
		ComPtr<ID3D11PixelShader> _psInventory;
		ComPtr<ID3D11VertexShader> _vsFullScreenQuad;
		ComPtr<ID3D11PixelShader> _psFullScreenQuad;
		ComPtr<ID3D11VertexShader> _vsShadowMap;
		ComPtr<ID3D11PixelShader> _psShadowMap;
		ComPtr<ID3D11VertexShader> _vsHUD;
		ComPtr<ID3D11PixelShader> _psHUDColor;
		ComPtr<ID3D11PixelShader> _psHUDTexture;
		ComPtr<ID3D11PixelShader> _psHUDBarColor;
		ComPtr<ID3D11SamplerState> _shadowSampler;
		ComPtr<ID3D11VertexShader> _vsFinalPass;
		ComPtr<ID3D11PixelShader> _psFinalPass;
		ComPtr<ID3D11VertexShader> _vsTransparentFinalPass;
		ComPtr<ID3D11PixelShader> _psTransparentFinalPass;

		// Constant buffers
		RenderView _gameCamera;
		ConstantBuffer<CCameraMatrixBuffer> _cbCameraMatrices;
		CItemBuffer _stItem;
		ConstantBuffer<CItemBuffer> _cbItem;
		CStaticBuffer _stStatic;
		ConstantBuffer<CStaticBuffer> _cbStatic;
		CLightBuffer _stLights;
		ConstantBuffer<CLightBuffer> _cbLights;
		CMiscBuffer _stMisc;
		ConstantBuffer<CMiscBuffer> _cbMisc;
		CRoomBuffer _stRoom;
		ConstantBuffer<CRoomBuffer> _cbRoom;
		CAnimatedBuffer _stAnimated;
		ConstantBuffer<CAnimatedBuffer> _cbAnimated;
		CShadowLightBuffer _stShadowMap;
		ConstantBuffer<CShadowLightBuffer> _cbShadowMap;
		CHUDBuffer _stHUD;
		ConstantBuffer<CHUDBuffer> _cbHUD;
		CHUDBarBuffer _stHUDBar;
		ConstantBuffer<CHUDBarBuffer> _cbHUDBar;
		CSpriteBuffer _stSprite;
		ConstantBuffer<CSpriteBuffer> _cbSprite;
		CPostProcessBuffer _stPostProcessBuffer;
		ConstantBuffer<CPostProcessBuffer> _cbPostProcessBuffer;
		CInstancedSpriteBuffer _stInstancedSpriteBuffer;
		ConstantBuffer<CInstancedSpriteBuffer> _cbInstancedSpriteBuffer;
		CBlendingBuffer _stBlending;
		ConstantBuffer<CBlendingBuffer> _cbBlending;
		CInstancedStaticMeshBuffer _stInstancedStaticMeshBuffer;
		ConstantBuffer<CInstancedStaticMeshBuffer> _cbInstancedStaticMeshBuffer;
		CSkyBuffer _stSky;
		ConstantBuffer<CSkyBuffer> _cbSky;

		// Primitive batchs
		std::unique_ptr<SpriteBatch> _spriteBatch;
		std::unique_ptr<PrimitiveBatch<Vertex>> _primitiveBatch;

		// Text
		std::unique_ptr<SpriteFont> _gameFont;
		std::vector<RendererStringToDraw> _stringsToDraw;
		float _blinkColorValue = 0.0f;
		float _blinkTime		  = 0.0f;
		bool  _isBlinkUpdated  = false;

		// Graphics resources
		Texture2D _logoTexture;
		Texture2D _skyTexture;
		Texture2D _whiteTexture;
		RendererSprite _whiteSprite;
		Texture2D _loadingBarBorder;
		Texture2D _loadingBarInner;
		Texture2D _loadingScreenTexture;

		VertexBuffer _roomsVertexBuffer;
		IndexBuffer _roomsIndexBuffer;
		VertexBuffer _moveablesVertexBuffer;
		IndexBuffer _moveablesIndexBuffer;
		VertexBuffer _staticsVertexBuffer;
		IndexBuffer _staticsIndexBuffer;
		VertexBuffer _skyVertexBuffer;
		IndexBuffer _skyIndexBuffer;
		VertexBuffer _quadVertexBuffer;

		// TODO: used by legacy transaprency, to remove in the new system
		std::vector<Vertex> _roomsVertices;
		std::vector<int> _roomsIndices;
		std::vector<Vertex> _moveablesVertices;
		std::vector<int> _moveablesIndices;
		std::vector<Vertex> _staticsVertices;
		std::vector<int> _staticsIndices;
		VertexBuffer _transparentFacesVertexBuffer;
		IndexBuffer _transparentFacesIndexBuffer;
		std::vector<Vertex> _transparentFacesVertices;
		fast_vector<int> _transparentFacesIndices;
		std::vector<RendererTransparentFace> _transparentFaces;

		// Rooms and collector
		std::vector<RendererRoom> _rooms;
		bool _invalidateCache;
		std::vector<short> _visitedRoomsStack;

		// Lights
		std::vector<RendererLight> _dynamicLights;
		RendererLight* _shadowLight;

		// Lines
		std::vector<RendererLine3D> _lines3DToDraw;
		std::vector<RendererLine2D> _lines2DToDraw;

		// Textures, objects and sprites
		std::vector<std::optional<RendererObject>> _moveableObjects;
		std::vector<std::optional<RendererObject>> _staticObjects;
		std::vector<RendererSprite> _sprites;
		std::vector<RendererSpriteSequence> _spriteSequences;
		Matrix _laraWorldMatrix;
		std::vector<RendererAnimatedTextureSet> _animatedTextureSets;
		std::vector<RendererMesh*> _meshes;
		std::vector<TexturePair> _roomTextures;
		std::vector<TexturePair> _animatedTextures;
		std::vector<TexturePair> _moveablesTextures;
		std::vector<TexturePair> _staticTextures;
		std::vector<Texture2D> _spritesTextures;

		// Preallocated pools of objects for avoiding new/delete
		// Items and effects are safe (can't be more than 1024 items in TR), 
		// lights should be oversized (eventually ignore lights more than MAX_LIGHTS)
		RendererItem _items[NUM_ITEMS];
		RendererEffect _effects[NUM_ITEMS];

		// Debug variables
		int _numDrawCalls = 0;
		int _numRoomsDrawCalls = 0;
		int _numMoveablesDrawCalls = 0;
		int _numStaticsDrawCalls = 0;
		int _numSpritesDrawCalls = 0;
		int _numTransparentDrawCalls = 0;
		int _numRoomsTransparentDrawCalls = 0;
		int _numMoveablesTransparentDrawCalls = 0;
		int _numStaticsTransparentDrawCalls = 0;
		int _numSpritesTransparentDrawCalls = 0;
		int _biggestRoomIndexBuffer = 0;
		int _numRoomsTransparentPolygons;
		int _numPolygons = 0;
		int _currentY;
		int _numDotProducts = 0;
		int _numCheckPortalCalls = 0;
		int _numGetVisibleRoomsCalls = 0;

		RendererDebugPage _debugPage = RendererDebugPage::None;

		// Times for debug
		int _timeUpdate;
		int _timeRoomsCollector;
		int _timeDraw;
		int _timeFrame;
		float _fps;
		int _currentCausticsFrame;

		// Screen settings
		int _screenWidth;
		int _screenHeight;
		bool _isWindowed;
		float _farView = DEFAULT_FAR_VIEW;

		// A flag to prevent extra renderer object addition
		bool _isLocked = false;

		// Caching state changes
		TextureBase* _lastTexture;
		BlendMode _lastBlendMode;
		DepthState _lastDepthState;
		CullMode _lastCullMode;

		// Private functions
		void BindTexture(TextureRegister registerType, TextureBase* texture, SamplerStateRegister samplerType);
		void BindRoomLights(std::vector<RendererLight*>& lights);
		void BindStaticLights(std::vector<RendererLight*>& lights);
		void BindInstancedStaticLights(std::vector<RendererLight*>& lights, int instanceID);
		void BindMoveableLights(std::vector<RendererLight*>& lights, int roomNumber, int prevRoomNumber, float fade);
		void BindRenderTargetAsTexture(TextureRegister registerType, RenderTarget2D* target, SamplerStateRegister samplerType);
		void BindConstantBufferVS(ConstantBufferRegister constantBufferType, ID3D11Buffer** buffer);
		void BindConstantBufferPS(ConstantBufferRegister constantBufferType, ID3D11Buffer** buffer);
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
		void DrawTransparentFaces(RenderView& renderView);
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
		void DrawPostprocess(RenderTarget2D* renderTarget, RenderView& view);
		void RenderInventoryScene(RenderTarget2D* renderTarget, TextureBase* background);
		void RenderTitleMenu(Menu menu);
		void RenderPauseMenu(Menu menu);
		void RenderLoadSaveMenu();
		void RenderOptionsMenu(Menu menu, int initialY);
		void RenderNewInventory();
		void RenderToCubemap(const RenderTargetCube& dest, const Vector3& pos, int roomNumber); 
		void RenderBlobShadows(RenderView& renderView);
		void RenderShadowMap(RendererItem* item, RenderView& view);
		void RenderItemShadows(RenderView& renderView);
		void SetBlendMode(BlendMode blendMode, bool force = false);
		void SetDepthState(DepthState depthState, bool force = false);
		void SetCullMode(CullMode cullMode, bool force = false);
		void SetAlphaTest(AlphaTestMode mode, float threshold, bool force = false);
		void SetScissor(RendererRectangle rectangle);
		void ResetAnimations();
		void ResetScissor();
		void ResetDebugVariables();
		float CalculateFrameRate();
		void AddSpriteBillboard(RendererSprite* sprite, const Vector3& pos, const Vector4& color, float orient2D, float scale,
					 Vector2 size, BlendMode blendMode, bool isSoftParticle, RenderView& view, SpriteRenderType renderType = SpriteRenderType::Default);
		void AddSpriteBillboardConstrained(RendererSprite* sprite, const Vector3& pos, const Vector4& color, float orient2D,
					 float scale, Vector2 size, BlendMode blendMode, const Vector3& constrainAxis,
					 bool isSoftParticle, RenderView& view, SpriteRenderType renderType = SpriteRenderType::Default);
		void AddSpriteBillboardConstrainedLookAt(RendererSprite* sprite, const Vector3& pos, const Vector4& color, float orient2D,
					 float scale, Vector2 size, BlendMode blendMode, const Vector3& lookAtAxis,
					 bool isSoftParticle, RenderView& view, SpriteRenderType renderType = SpriteRenderType::Default);
		void AddQuad(RendererSprite* sprite, const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& vertex3,
					 const Vector4 color, float orient2D, float scale, Vector2 size, BlendMode blendMode, bool softParticles,
					 RenderView& view);
		void AddQuad(RendererSprite* sprite, const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& vertex3,
					 const Vector4& color0, const Vector4& color1, const Vector4& color2, const Vector4& color3, float orient2D,
					 float scale, Vector2 size, BlendMode blendMode, bool isSoftParticle, RenderView& view, SpriteRenderType renderType = SpriteRenderType::Default);
		void AddColoredQuad(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& vertex3,
							const Vector4& color, BlendMode blendMode, RenderView& view);
		void AddColoredQuad(const Vector3& vertex0, const Vector3& vertex1, const Vector3& vertex2, const Vector3& vertex3,
							const Vector4& color0, const Vector4& color1, const Vector4& color2, const Vector4& color3,
							BlendMode blendMode, RenderView& view, SpriteRenderType renderType = SpriteRenderType::Default);
		Matrix GetWorldMatrixForSprite(RendererSpriteToDraw* spr, RenderView& view);
		RendererObject& GetRendererObject(GAME_OBJECT_ID id);
		RendererMesh* GetMesh(int meshIndex);
		Texture2D CreateDefaultNormalTexture();
		Vector4 GetPortalRect(Vector4 v, Vector4 vp);
		bool SphereBoxIntersection(BoundingBox box, Vector3 sphereCentre, float sphereRadius);
		void InitQuad();

		inline void DrawIndexedTriangles(int count, int baseIndex, int baseVertex)
		{
			_context->DrawIndexed(count, baseIndex, baseVertex);
			_numPolygons += count / 3;
			_numDrawCalls++;
		}

		inline void DrawIndexedInstancedTriangles(int count, int instances, int baseIndex, int baseVertex)
		{
			_context->DrawIndexedInstanced(count, instances, baseIndex, baseVertex, 0);
			_numPolygons += count / 3 * instances;
			_numDrawCalls++;
		}

		inline void DrawInstancedTriangles(int count, int instances, int baseVertex)
		{
			_context->DrawInstanced(count, instances, baseVertex, 0);
			_numPolygons += count / 3 * instances;
			_numDrawCalls++;
		}

		inline void DrawTriangles(int count, int baseVertex)
		{
			_context->Draw(count, baseVertex);
			_numPolygons += count / 3;
			_numDrawCalls++;
		}

		template <typename C>
		ConstantBuffer<C> CreateConstantBuffer()
		{
			return ConstantBuffer<C>(_device.Get());
		}

		static bool DoesBlendModeRequireSorting(BlendMode blendMode)
		{
			return (blendMode == BlendMode::AlphaBlend ||
				    blendMode == BlendMode::Exclude ||
				    blendMode == BlendMode::Lighten ||
				    blendMode == BlendMode::Screen ||
				    blendMode == BlendMode::Subtractive ||
				    blendMode == BlendMode::NoDepthTest);
		}

	public:
		Renderer();
		~Renderer();

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
		void RenderScene(RenderTarget2D* renderTarget, RenderView& view);
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
							  int priority, BlendMode blendMode, const Vector2& aspectCorrection, RenderView& renderView);
		void CollectDisplaySprites(RenderView& renderView);
	};

	extern Renderer g_Renderer;
}
