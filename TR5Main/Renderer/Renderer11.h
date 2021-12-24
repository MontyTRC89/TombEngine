#pragma once
#include "Renderer/RenderEnums.h"
#include "ConstantBuffers/StaticBuffer.h"
#include "ConstantBuffers/LightBuffer.h"
#include "ConstantBuffers/MiscBuffer.h"
#include "ConstantBuffers/HUDBarBuffer.h"
#include "ConstantBuffers/HUDBuffer.h"
#include "ConstantBuffers/ShadowLightBuffer.h"
#include "ConstantBuffers/RoomBuffer.h"
#include "ConstantBuffers/ItemBuffer.h"
#include "ConstantBuffers/AnimatedBuffer.h"
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
#include "ConstantBuffers\SpriteBuffer.h"
#include "RenderTargetCube\RenderTargetCube.h"
#include "RenderView/RenderView.h"
#include <level.h>
#include "ConstantBuffer/ConstantBuffer.h"
#include "RenderTargetCubeArray/RenderTargetCubeArray.h"
#include <wrl/client.h>
#include <CommonStates.h>
#include <SpriteFont.h>
#include <PrimitiveBatch.h>
#include <d3d9types.h>

struct CAMERA_INFO;
namespace TEN::Renderer
{
	constexpr size_t MAX_DYNAMIC_SHADOWS = 1;
	using TexturePair = std::tuple<Texture2D, Texture2D>;

	constexpr auto NUM_ANIMATED_SETS = 1024;
	//constexpr auto MAX_LIGHTS_DRAW = 16384;
	constexpr auto MAX_DYNAMIC_LIGHTS = 16384;
	constexpr auto MAX_DRAW_STATICS = 16384;
	constexpr auto MAX_Bones = 32;
	constexpr auto MAX_SPRITES = 16384;
	constexpr auto REFERENCE_RES_WIDTH = 800;
	constexpr auto REFERENCE_RES_HEIGHT = 450;
	constexpr auto HUD_UNIT_X = 1.0f / REFERENCE_RES_WIDTH;
	constexpr auto HUD_UNIT_Y = 1.0f / REFERENCE_RES_HEIGHT;
	constexpr auto HUD_ZERO_Y = -REFERENCE_RES_HEIGHT;
	
	struct RendererDisplayMode
	{
		int Width;
		int Height;
		int RefreshRate;
	};
	struct RendererVideoAdapter
	{
		std::string Name;
		int Index;
		std::vector<RendererDisplayMode> DisplayModes;
	};
	
	struct RendererHUDBar
	{
		VertexBuffer vertexBufferBorder;
		IndexBuffer indexBufferBorder;
		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;
		/*
			Initialises a new Bar for rendering. the Coordinates are set in the Reference Resolution (default 800x600).
			The colors are setup like this (4 triangles)
			0-------1
			| \   / |
			|  >2<  |
			| /   \ |
			3-------4
		*/
		RendererHUDBar(ID3D11Device* m_device, int x, int y, int w, int h, int borderSize, std::array<DirectX::SimpleMath::Vector4, 5> colors);
	};
	
	struct RendererStringToDraw
	{
		float X;
		float Y;
		int Flags;
		std::wstring String;
		DirectX::SimpleMath::Vector3 Color;
	};
	
	struct RendererBone 
	{
		DirectX::SimpleMath::Vector3 Translation;
		DirectX::SimpleMath::Matrix GlobalTransform;
		DirectX::SimpleMath::Matrix Transform;
		DirectX::SimpleMath::Vector3 GlobalTranslation;
		std::vector<RendererBone*> Children;
		RendererBone* Parent;
		int Index;
		DirectX::SimpleMath::Vector3 ExtraRotation;
		byte ExtraRotationFlags;
	
		RendererBone(int index)
		{
			Index = index;
			ExtraRotationFlags = 0;
			Translation = DirectX::SimpleMath::Vector3(0, 0, 0);
			ExtraRotation = DirectX::SimpleMath::Vector3(0, 0, 0);
		}
	};
	
	struct RendererLight
	{
		DirectX::SimpleMath::Vector3 Position;
		float Type;
		DirectX::SimpleMath::Vector3 Color;
		bool Dynamic;
		DirectX::SimpleMath::Vector4 Direction;
		float Intensity;
		float In;
		float Out;
		float Range;
	};
	
	struct RendererAnimatedTexture 
	{
		DirectX::SimpleMath::Vector2 UV[4];
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
		DirectX::SimpleMath::Matrix World;
	};
	
	struct RendererRoom 
	{
		ROOM_INFO* Room;
		DirectX::SimpleMath::Vector4 AmbientLight;
		std::vector<RendererBucket> buckets;
		std::vector<RendererLight> Lights;
		std::vector<RendererStatic> Statics;
		bool Visited;
		float Distance;
		int RoomNumber;
	};
	
	struct RendererRoomNode
	{
		short From;
		short To;
		DirectX::SimpleMath::Vector4 ClipPort;
	};
	
	struct RendererItem
	{
		int Id;
		ITEM_INFO* Item;
		DirectX::SimpleMath::Matrix World;
		DirectX::SimpleMath::Matrix Translation;
		DirectX::SimpleMath::Matrix Rotation;
		DirectX::SimpleMath::Matrix Scale;
		DirectX::SimpleMath::Matrix AnimationTransforms[32];
		int NumMeshes;
		std::vector<RendererLight*> Lights;
		bool DoneAnimations;
	};
	
	struct RendererMesh
	{
		BoundingSphere Sphere;
		std::vector<RendererBucket> buckets;
		std::vector<DirectX::SimpleMath::Vector3> Positions;
	};
	
	struct RendererEffect
	{
		int Id;
		FX_INFO* Effect;
		DirectX::SimpleMath::Matrix World;
		RendererMesh* Mesh;
		std::vector<RendererLight*> Lights;
	};
	
	struct RendererObject
	{
		int Id;
		std::vector<RendererMesh*> ObjectMeshes;
		RendererBone* Skeleton;
		std::vector<DirectX::SimpleMath::Matrix> AnimationTransforms;
		std::vector<DirectX::SimpleMath::Matrix> BindPoseTransforms;
		std::vector<RendererBone*> LinearizedBones;
		bool DoNotDraw;
		bool HasDataInBucket[NUM_BUCKETS];
		bool HasDataInAnimatedBucket[NUM_BUCKETS];
	
		~RendererObject()
		{
			/*for (int i = 0; i < ObjectMeshes.size(); i++)
				delete ObjectMeshes[i];*/
			for (int i = 0; i < LinearizedBones.size(); i++)
				delete LinearizedBones[i];
		}
	};
	
	struct RendererSprite
	{
		int Width;
		int Height;
		Vector2 UV[4];
		TEN::Renderer::Texture2D* Texture;
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
				std::copy(other.SpritesList.begin(), other.SpritesList.end(),back_inserter(SpritesList));
			}
			return *this;
		}
	};
	
	struct RendererLine3D
	{
		DirectX::SimpleMath::Vector3 start;
		DirectX::SimpleMath::Vector3 end;
		DirectX::SimpleMath::Vector4 color;
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
		DirectX::SimpleMath::Vector2 Vertices[2];
		DirectX::SimpleMath::Vector4 Color;
	};

	struct RendererRect2D
	{
		RECT Rectangle;
		DirectX::SimpleMath::Vector4 Color;
	};
	
	class Renderer11
	{
	private:
		// Core DX11 objects
		Microsoft::WRL::ComPtr<ID3D11Device> m_device = nullptr;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context = nullptr;
		Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain = nullptr;
		std::unique_ptr<CommonStates> m_states = nullptr;
		Microsoft::WRL::ComPtr<ID3D11BlendState> m_subtractiveBlendState = nullptr;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout = nullptr;
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
	
		TEN::Renderer::RenderTarget2D m_dumpScreenRenderTarget;
		TEN::Renderer::RenderTarget2D m_renderTarget;
		TEN::Renderer::RenderTarget2D m_currentRenderTarget;
		TEN::Renderer::RenderTarget2D m_shadowMap;
		TEN::Renderer::RenderTargetCube m_reflectionCubemap;
		// Shaders
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vsRooms;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vsRooms_Anim;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_psRooms;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vsItems;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_psItems;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vsHairs;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_psHairs;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vsStatics;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_psStatics;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vsSky;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_psSky;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vsSprites;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_psSprites;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vsSolid;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_psSolid;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vsInventory;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_psInventory;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vsFullScreenQuad;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_psFullScreenQuad;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vsShadowMap;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_psShadowMap;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vsHUD;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_psHUDColor;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_psHUDTexture;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_psHUDBarColor;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_shadowSampler;
	
	
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
		// Text and sprites
		std::unique_ptr<SpriteFont> m_gameFont;
		std::unique_ptr<SpriteBatch> m_spriteBatch;
		std::vector<RendererStringToDraw> m_strings;
		int m_blinkColorValue;
		int m_blinkColorDirection;
		std::unique_ptr<PrimitiveBatch<RendererVertex>> m_primitiveBatch;
	
		// System resources
		TEN::Renderer::Texture2D m_HUDBarBorderTexture;
		TEN::Renderer::Texture2D m_HUDBarTextures[4];
		std::vector<TEN::Renderer::Texture2D> m_caustics;
		TEN::Renderer::Texture2D m_binocularsTexture;
		TEN::Renderer::Texture2D m_LasersightTexture;
		TEN::Renderer::Texture2D m_whiteTexture;
		TEN::Renderer::RenderTargetCubeArray m_shadowMaps;
	
		// Level data
		TEN::Renderer::Texture2D m_titleScreen;
		TEN::Renderer::Texture2D m_loadScreen;
		TEN::Renderer::Texture2D m_textureAtlas;
		TEN::Renderer::Texture2D m_skyTexture;
		TEN::Renderer::Texture2D m_logo;
		VertexBuffer m_roomsVertexBuffer;
		IndexBuffer m_roomsIndexBuffer;
		VertexBuffer m_moveablesVertexBuffer;
		IndexBuffer m_moveablesIndexBuffer;
		VertexBuffer m_staticsVertexBuffer;
		IndexBuffer m_staticsIndexBuffer;
		std::vector<RendererRoom> m_rooms;
		DirectX::SimpleMath::Matrix m_hairsMatrices[12];
		short m_numHairVertices;
		short m_numHairIndices;
		std::vector<RendererVertex> m_hairVertices;
		std::vector<short> m_hairIndices;
		std::vector<RendererLight*> m_dynamicLights;
		std::vector<RendererLine3D*> m_lines3DToDraw;
		std::vector<RendererLine2D*> m_lines2DToDraw;
		std::vector<RendererRect2D*> m_rects2DToDraw;
		std::vector<RendererBucket*> m_bucketsToDraw;
		int m_nextSprite;
		RendererLine3D* m_lines3DBuffer;
		int m_nextLine3D;
		RendererLine2D* m_lines2DBuffer;
		int m_nextLine2D;
		RendererRect2D* m_rects2DBuffer;
		int m_nextRect2D;
		RendererLight* m_shadowLight;
		std::vector<std::optional<RendererObject>> m_moveableObjects;
		std::vector<std::optional<RendererObject>> m_staticObjects;
		std::vector<RendererSprite> m_sprites;
		int m_numMoveables;
		int m_numStatics;
		int m_numSprites;
		int m_numSpritesSequences;
		std::vector<RendererSpriteSequence> m_spriteSequences;
		std::unordered_map<int, RendererMesh*> m_meshPointersToMesh;
		DirectX::SimpleMath::Matrix m_LaraWorldMatrix;
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
		std::vector<TEN::Renderer::Texture2D> m_spritesTextures;
	
		// Preallocated pools of objects for avoiding new/delete
		// Items and effects are safe (can't be more than 1024 items in TR), 
		// lights should be oversized (eventually ignore lights more than MAX_LIGHTS)
		RendererItem m_items[NUM_ITEMS];
		RendererEffect m_effects[NUM_ITEMS];
		RendererLight m_lights[MAX_LIGHTS];
		int m_nextLight;
		int m_currentY;

		// Debug variables
		int m_numDrawCalls = 0;
		RENDERER_DEBUG_PAGE m_numDebugPage = RENDERER_DEBUG_PAGE::NO_PAGE;
	
		// Times for debug
		int m_timeUpdate;
		int m_timeDraw;
		int m_timeFrame;
	
		// Others
		bool m_firstWeather;
		RendererWeatherParticle	m_rain[NUM_RAIN_DROPS];
		RendererWeatherParticle	m_snow[NUM_SNOW_PARTICLES];
		RENDERER_FADE_STATUS m_fadeStatus = RENDERER_FADE_STATUS::NO_FADE;
		float m_fadeFactor;
		int m_progress;
		bool m_enableCinematicBars = false;
		int m_pickupRotation = 0;

		// Private functions
		int getAnimatedTextureInfo(short textureId);
		void drawAllStrings();
		void fromTrAngle(DirectX::SimpleMath::Matrix* matrix, short* frameptr, int index);
		void buildHierarchy(RendererObject* obj);
		void buildHierarchyRecursive(RendererObject* obj, RendererBone* node, RendererBone* parentNode);
		void updateAnimation(RendererItem* item, RendererObject& obj, ANIM_FRAME** frmptr, short frac, short rate, int mask,bool useObjectWorldRotation = false);
		bool printDebugMessage(int x, int y, int alpha, byte r, byte g, byte b, LPCSTR Message);
		void getVisibleObjects(int from, int to, RenderView& renderView);
		bool checkPortal(short roomIndex, ROOM_DOOR* portal,const Matrix& viewProjection);
		void collectRooms(RenderView& renderView);
		void collectItems(short roomNumber, RenderView& renderView);
		void collectStatics(short roomNumber, RenderView& renderView);
		void collectLightsForEffect(short roomNumber, RendererEffect* effect, RenderView& renderView);
		void collectLightsForItem(short roomNumber, RendererItem* item, RenderView& renderView);
		void collectLightsForRoom(short roomNumber, RenderView& renderView);
		void prepareLights(RenderView& view);
		void collectEffects(short roomNumber, RenderView& renderView);
		void clearSceneItems();
		void updateItemsAnimations(RenderView& view);
		void updateEffects(RenderView& view);
		int	getFrame(short animation, short frame, ANIM_FRAME** framePtr, int* rate);
		bool drawAmbientCubeMap(short roomNumber);
		bool sphereBoxIntersection(DirectX::SimpleMath::Vector3 boxMin, DirectX::SimpleMath::Vector3 boxMax, DirectX::SimpleMath::Vector3 sphereCentre, float sphereRadius);
		void drawHorizonAndSky(RenderView& renderView, ID3D11DepthStencilView* depthTarget);
		void drawRooms(bool transparent, bool animated, RenderView& view);
		void drawItems(bool transparent, bool animated,RenderView& view);
		void drawAnimatingItem(RenderView& view,RendererItem* item, bool transparent, bool animated);
		void drawBaddieGunflashes(RenderView& view);
		void drawStatics(bool transparent, RenderView& view);
		void renderShadowMap(RenderView& view);
		void drawWraithExtra(RenderView& view,RendererItem* item, bool transparent, bool animated);
		void drawDarts(RenderView& view, RendererItem* item, bool transparent, bool animated);
		void drawLara(RenderView& view,bool transparent, bool shadowMap);
		void drawFires(RenderView& view);
		void drawSparks(RenderView& view);
		void drawSmokes(RenderView& view);
		void drawLightning(RenderView& view);
		void drawBlood(RenderView& view);
		void drawWeatherParticles(RenderView& view);
		void drawDrips(RenderView& view);
		void drawBubbles(RenderView& view);
		void drawEffects(RenderView& view,bool transparent);
		void drawEffect(RenderView& view,RendererEffect* effect, bool transparent);
		void drawSplahes(RenderView& view);
		void drawSprites(RenderView& view);
		void drawLines3D(RenderView& view);
		void drawLines2D();
		void drawRects2D();
		void drawOverlays(RenderView& view);
		void drawRopes(RenderView& view);
		void drawBats(RenderView& view);
		void drawRats(RenderView& view);
		void drawScarabs(RenderView& view);
		void drawSpiders(RenderView& view);
		bool drawGunFlashes(RenderView& view);
		void drawGunShells(RenderView& view);
		void drawLocusts(RenderView& view);
		void renderInventoryScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget, ID3D11ShaderResourceView* background);
		void renderTitleMenu();
		void renderPauseMenu();
		void renderLoadSaveMenu();
		void renderNewInventory();
		void drawStatistics();
		void drawExamines();
		void drawDiary();
		void drawDebris(RenderView& view,bool transparent);
		void drawFullScreenImage(ID3D11ShaderResourceView* texture, float fade, ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget);
		void createBillboardMatrix(DirectX::SimpleMath::Matrix* out, DirectX::SimpleMath::Vector3* particlePos, DirectX::SimpleMath::Vector3* cameraPos, float rotation);
		void drawShockwaves(RenderView& view);
		void drawRipples(RenderView& view);
		void drawUnderwaterDust(RenderView& view);
		void drawFullScreenQuad(ID3D11ShaderResourceView* texture, DirectX::SimpleMath::Vector3 color, bool cinematicBars);
		bool isRoomUnderwater(short roomNumber);
		bool isInRoom(int x, int y, int z, short roomNumber);
		void drawColoredQuad(int x, int y, int w, int h, DirectX::SimpleMath::Vector4 color);
		void initialiseScreen(int w, int h, int refreshRate, bool windowed, HWND handle, bool reset);
		void initialiseBars();
		void drawSmokeParticles(RenderView& view);
		void drawSparkParticles(RenderView& view);
		void drawDripParticles(RenderView& view);
		void drawExplosionParticles(RenderView& view);
		void renderToCubemap(const RenderTargetCube& dest,const Vector3& pos,int roomNumber);
		void drawLaraHolsters(bool transparent);
		void drawLaraMesh(RendererMesh* mesh, bool transparent);
		void drawSimpleParticles(RenderView& view); 
		void setBlendMode(BLEND_MODES blendMode);
	public:
		RendererMesh* getRendererMeshFromTrMesh(RendererObject* obj, MESH* meshPtr, short boneIndex, int isJoints, int isHairs);
		void drawBar(float percent, const RendererHUDBar * const bar, GAME_OBJECT_ID textureSlot, int frame, bool poison);

		DirectX::SimpleMath::Matrix View;
		DirectX::SimpleMath::Matrix Projection;
		DirectX::SimpleMath::Matrix ViewProjection;
		float FieldOfView;
		int ScreenWidth;
		int ScreenHeight;
		bool Windowed;
		int NumTexturePages;
		Renderer11();
		~Renderer11();
	
		void Create();
		void EnumerateVideoModes();
		void Initialise(int w, int h, int refreshRate, bool windowed, HWND handle);
		void Draw();
		bool PrepareDataForTheRenderer();
		void UpdateCameraMatrices(CAMERA_INFO* cam, float roll, float fov);
		void renderSimpleScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget, RenderView& view);
		void DumpGameScene();
		void renderInventory();
		void renderTitle();
		void renderScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget, RenderView& view);
		void clearScene();
		void printDebugMessage(LPCSTR message, ...);
		void drawDebugInfo();
		void switchDebugPage(bool back);
		void drawPickup(short objectNum);
		int SyncRenderer();
		void drawString(int x, int y, const char* string, D3DCOLOR color, int flags);
		void clearDynamicLights();
		void addDynamicLight(int x, int y, int z, short falloff, byte r, byte g, byte b);
		void freeRendererData();
		void enableCinematicBars(bool value);
		void fadeIn();
		void fadeOut();
		void renderLoadingScreen(std::wstring& fileName);
		void updateProgress(float value);
		bool isFading();
		void getLaraBonePosition(DirectX::SimpleMath::Vector3* pos, int bone);
		void toggleFullScreen();
		bool isFullsScreen();
		std::vector<RendererVideoAdapter>* getAdapters();
		void renderTitleImage();
		void addLine2D(int x1, int y1, int x2, int y2, byte r, byte g, byte b, byte a);
		void addQuad2D(RECT rect, byte r, byte g, byte b, byte a);
		void addLine3D(DirectX::SimpleMath::Vector3 start, DirectX::SimpleMath::Vector3 end, DirectX::SimpleMath::Vector4 color);
		void addBox(Vector3 min, Vector3 max, Vector4 color); 
		void addBox(Vector3* corners, Vector4 color);
		void addDebugBox(BoundingOrientedBox box, Vector4 color, RENDERER_DEBUG_PAGE page);
		void addDebugBox(Vector3 min, Vector3 max, Vector4 color, RENDERER_DEBUG_PAGE page);
		void addSphere(Vector3 center, float radius , Vector4 color);
		void addDebugSphere(Vector3 center, float radius, Vector4 color, RENDERER_DEBUG_PAGE page);
		void changeScreenResolution(int width, int height, int frequency, bool windowed);
		void flipRooms(short roomNumber1, short roomNumber2);
		void resetAnimations();
		void updateLaraAnimations(bool force);
		void updateItemAnimations(int itemNumber, bool force);
		void getLaraAbsBonePosition(DirectX::SimpleMath::Vector3* pos, int joint);
		void getItemAbsBonePosition(int itemNumber, DirectX::SimpleMath::Vector3* pos, int joint);
		int getSpheres(short itemNumber, BoundingSphere* ptr, char worldSpace, DirectX::SimpleMath::Matrix local);
		void getBoneMatrix(short itemNumber, int joint, DirectX::SimpleMath::Matrix* outMatrix);
		void drawObjectOn2DPosition(short x, short y, short objectNum, short rotX, short rotY, short rotZ, float scale1);

		RendererMesh* getMesh(int meshIndex);
	private:
		Texture2D createDefaultNormalTexture();
		void drawFootprints(RenderView& view);

		template <typename C>
		ConstantBuffer<C> createConstantBuffer() {
			return ConstantBuffer<C>(m_device.Get());
		}

};
	extern Renderer11 g_Renderer;
}