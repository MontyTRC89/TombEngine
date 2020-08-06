#pragma once
#include "RenderEnums.h"
#include "ConstantBuffers/StaticBuffer.h"
#include "ConstantBuffers/LightBuffer.h"
#include "ConstantBuffers/MiscBuffer.h"
#include "ConstantBuffers/HUDBarBuffer.h"
#include "ConstantBuffers/HUDBuffer.h"
#include "ConstantBuffers/ShadowLightBuffer.h"
#include "ConstantBuffers/RoomBuffer.h"
#include "ConstantBuffers/ItemBuffer.h"
#include "Frustum.h"

#include "items.h"
#include "effect.h"
#include "IndexBuffer/IndexBuffer.h"
#include "VertexBuffer/VertexBuffer.h"
#include "RenderTarget2D/RenderTarget2D.h"
#include "ConstantBuffers/CameraMatrixBuffer.h"
#include "Texture2D/Texture2D.h"
#include "ConstantBuffers\SpriteBuffer.h"
#include "RenderTargetCube\RenderTargetCube.h"
#include "RenderView/RenderView.h"

struct CAMERA_INFO;

#include <level.h>

namespace T5M::Renderer
{
	using TexturePair = std::tuple<Texture2D, Texture2D>;
	#define MESH_BITS(x) (1 << x)
	#define DX11_RELEASE(x) if (x != NULL) x->Release()
	#define DX11_DELETE(x) if (x != NULL) { delete x; x = NULL; }
	constexpr auto NUM_ANIMATED_SETS = 1024;
	constexpr auto MAX_LIGHTS_DRAW = 16384;
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
	
	struct RendererVertex
	{
		DirectX::SimpleMath::Vector3 Position;
		DirectX::SimpleMath::Vector3 Normal;
		DirectX::SimpleMath::Vector2 UV;
		DirectX::SimpleMath::Vector4 Color;
		DirectX::SimpleMath::Vector3 Tangent;
		DirectX::SimpleMath::Vector3 BiTangent;
		float Bone;
		int IndexInPoly;
		int OriginalIndex;
	};
	
	
	
	
	struct RendererHUDBar
	{
		VertexBuffer vertexBufferBorder;
		IndexBuffer indexBufferBorder;
		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;
		/*
			Initialises a new Bar for rendering. the Coordinates are set in the Reference Resolution (default 800x600).
			The colors are setup like this
			0-----------1-----------2
			|           |           |
			3-----------4-----------5
			|           |           |
			6-----------7-----------8
		*/
		RendererHUDBar(ID3D11Device* m_device, int x, int y, int w, int h, int borderSize, std::array<DirectX::SimpleMath::Vector4, 9> colors);
	};
	
	struct RendererStringToDraw
	{
		float X;
		float Y;
		int Flags;
		std::wstring String;
		DirectX::SimpleMath::Vector3 Color;
	};
	
	struct RendererPolygon 
	{
		byte Shape;
		int AnimatedSet;
		int TextureId;
		int Distance;
		int Indices[4];
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
	
		RendererLight()
		{
		}
	};
	
	struct RendererAnimatedTexture 
	{
		int Id;
		DirectX::SimpleMath::Vector2 UV[4];
	};
	
	struct RendererAnimatedTextureSet 
	{
		int NumTextures;
		std::vector<RendererAnimatedTexture> Textures;
	};
	
	struct RendererBucket
	{
		std::vector<RendererVertex> Vertices;
		std::vector<int> Indices;
		std::vector<RendererPolygon> Polygons;
		std::vector<RendererPolygon> AnimatedPolygons;
		int StartVertex;
		int StartIndex;
		int NumTriangles;
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
		RendererBucket Buckets[NUM_BUCKETS];
		RendererBucket AnimatedBuckets[NUM_BUCKETS];
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
		RendererBucket Buckets[NUM_BUCKETS];
		RendererBucket AnimatedBuckets[NUM_BUCKETS];
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
		T5M::Renderer::Texture2D* Texture;
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
	
	struct RendererSpriteToDraw
	{
		RENDERER_SPRITE_TYPE Type;
		RendererSprite* Sprite;
		float Distance;
		float Scale;
		DirectX::SimpleMath::Vector3 pos;
		DirectX::SimpleMath::Vector3 vtx1;
		DirectX::SimpleMath::Vector3 vtx2;
		DirectX::SimpleMath::Vector3 vtx3;
		DirectX::SimpleMath::Vector3 vtx4;
		DirectX::SimpleMath::Vector4 color;
		float Rotation;
		float Width;
		float Height;
		BLEND_MODES BlendMode;
		DirectX::SimpleMath::Vector3 ConstrainAxis;
		DirectX::SimpleMath::Vector3 LookAtAxis;
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
	
	class Renderer11
	{
	private:
		// Core DX11 objects
		ID3D11Device* m_device = nullptr;
		ID3D11DeviceContext* m_context = nullptr;
		IDXGISwapChain* m_swapChain = nullptr;
		IDXGIDevice* m_dxgiDevice = nullptr;
		CommonStates* m_states = nullptr;
		ID3D11BlendState* m_subtractiveBlendState = nullptr;
		ID3D11InputLayout* m_inputLayout = nullptr;
		D3D11_VIEWPORT m_viewport;
		D3D11_VIEWPORT m_shadowMapViewport;
		Viewport* m_viewportToolkit;
		std::vector<RendererVideoAdapter> m_adapters;
	
		// Main back buffer
		ID3D11RenderTargetView* m_backBufferRTV;
		ID3D11Texture2D* m_backBufferTexture;
	
		ID3D11DepthStencilState* m_depthStencilState;
		ID3D11DepthStencilView* m_depthStencilView;
		ID3D11Texture2D* m_depthStencilTexture;
	
		T5M::Renderer::RenderTarget2D m_dumpScreenRenderTarget;
		T5M::Renderer::RenderTarget2D m_renderTarget;
		T5M::Renderer::RenderTarget2D m_currentRenderTarget;
		T5M::Renderer::RenderTarget2D m_shadowMap;
		T5M::Renderer::RenderTargetCube m_reflectionCubemap;
		// Shaders
		ID3D11VertexShader* m_vsRooms;
		ID3D11PixelShader* m_psRooms;
		ID3D11VertexShader* m_vsItems;
		ID3D11PixelShader* m_psItems;
		ID3D11VertexShader* m_vsHairs;
		ID3D11PixelShader* m_psHairs;
		ID3D11VertexShader* m_vsStatics;
		ID3D11PixelShader* m_psStatics;
		ID3D11VertexShader* m_vsSky;
		ID3D11PixelShader* m_psSky;
		ID3D11VertexShader* m_vsSprites;
		ID3D11PixelShader* m_psSprites;
		ID3D11VertexShader* m_vsSolid;
		ID3D11PixelShader* m_psSolid;
		ID3D11VertexShader* m_vsInventory;
		ID3D11PixelShader* m_psInventory;
		ID3D11VertexShader* m_vsFullScreenQuad;
		ID3D11PixelShader* m_psFullScreenQuad;
		ID3D11VertexShader* m_vsShadowMap;
		ID3D11PixelShader* m_psShadowMap;
		ID3D11VertexShader* m_vsHUD;
		ID3D11PixelShader* m_psHUDColor;
		ID3D11PixelShader* m_psHUDTexture;
		ID3D11PixelShader* m_psHUDBarColor;
	
		ID3D11ShaderResourceView* m_shadowMapRV;
		ID3D11Texture2D* m_shadowMapTexture;
		ID3D11DepthStencilView* m_shadowMapDSV;
	
	
		// Constant buffers
		RenderView gameCamera;
		ID3D11Buffer* m_cbCameraMatrices;
		CItemBuffer m_stItem;
		ID3D11Buffer* m_cbItem;
		CStaticBuffer m_stStatic;
		ID3D11Buffer* m_cbStatic;
		CLightBuffer m_stLights;
		ID3D11Buffer* m_cbLights;
		CMiscBuffer m_stMisc;
		ID3D11Buffer* m_cbMisc;
		CRoomBuffer m_stRoom;
		ID3D11Buffer* m_cbRoom;
		CShadowLightBuffer m_stShadowMap;
		ID3D11Buffer* m_cbShadowMap;
		CHUDBuffer m_stHUD;
		ID3D11Buffer* m_cbHUD;
		CHUDBarBuffer m_stHUDBar;
		ID3D11Buffer* m_cbHUDBar;
		CSpriteBuffer m_stSprite;
		ID3D11Buffer* m_cbSprite;
		// Text and sprites
		SpriteFont* m_gameFont;
		SpriteBatch* m_spriteBatch;
		std::vector<RendererStringToDraw> m_strings;
		int m_blinkColorValue;
		int m_blinkColorDirection;
		PrimitiveBatch<RendererVertex>* m_primitiveBatch;
	
		// System resources
		T5M::Renderer::Texture2D m_HUDBarBorderTexture;
		T5M::Renderer::Texture2D m_caustics[NUM_CAUSTICS_TEXTURES];
		T5M::Renderer::Texture2D m_binocularsTexture;
		T5M::Renderer::Texture2D m_whiteTexture;
	
		// Level data
		T5M::Renderer::Texture2D m_titleScreen;
		T5M::Renderer::Texture2D m_loadScreen;
		T5M::Renderer::Texture2D m_textureAtlas;
		T5M::Renderer::Texture2D m_skyTexture;
		T5M::Renderer::Texture2D m_logo;
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
		std::vector<RendererRoom*> m_roomsToDraw;
		std::vector<RendererItem*> m_itemsToDraw;
		std::vector<RendererEffect*> m_effectsToDraw;
		std::vector<RendererStatic*> m_staticsToDraw;
		std::vector<RendererLight*> m_lightsToDraw;
		std::vector<RendererLight*> m_dynamicLights;
		std::vector<RendererSpriteToDraw*> m_spritesToDraw;
		std::vector<RendererLine3D*> m_lines3DToDraw;
		std::vector<RendererLine2D*> m_lines2DToDraw;
		std::vector<RendererLight*> m_tempItemLights;
		RendererSpriteToDraw* m_spritesBuffer;
		int m_nextSprite;
		RendererLine3D* m_lines3DBuffer;
		int m_nextLine3D;
		RendererLine2D* m_lines2DBuffer;
		int m_nextLine2D;
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
		std::vector<TexturePair> m_moveablesTextures;
		std::vector<TexturePair> m_staticsTextures;
		std::vector<T5M::Renderer::Texture2D> m_spritesTextures;
	
		// Debug variables
		int m_numDrawCalls = 0;
	
		// Preallocated pools of objects for avoiding new/delete
		// Items and effects are safe (can't be more than 1024 items in TR), 
		// lights should be oversized (eventually ignore lights more than MAX_LIGHTS)
		RendererItem m_items[NUM_ITEMS];
		RendererEffect m_effects[NUM_ITEMS];
		RendererLight m_lights[MAX_LIGHTS];
		int m_nextLight;
		int m_currentY;
	
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
		int m_pickupRotation;
	
		// Private functions
		bool											drawScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget, CCameraMatrixBuffer& camera);
		bool											drawAllStrings();
		ID3D11VertexShader*								compileVertexShader(const wchar_t * fileName, const char* function, const char* model, ID3D10Blob** bytecode);
		ID3D11GeometryShader*							compileGeometryShader(const wchar_t * fileName);
		ID3D11PixelShader*								compilePixelShader(const wchar_t * fileName, const char* function, const char* model, ID3D10Blob** bytecode);
		ID3D11ComputeShader*							compileComputeShader(const wchar_t * fileName);
		ID3D11Buffer*									createConstantBuffer(size_t size);
		int												getAnimatedTextureInfo(short textureId);
		void											initialiseHairRemaps();
		RendererMesh*									getRendererMeshFromTrMesh(RendererObject* obj, MESH* meshPtr, short boneIndex, int isJoints, int isHairs);
		void											fromTrAngle(DirectX::SimpleMath::Matrix* matrix, short* frameptr, int index);
		void											buildHierarchy(RendererObject* obj);
		void											buildHierarchyRecursive(RendererObject* obj, RendererBone* node, RendererBone* parentNode);
		void											updateAnimation(RendererItem* item, RendererObject& obj, ANIM_FRAME** frmptr, short frac, short rate, int mask,bool useObjectWorldRotation = false);
		bool											printDebugMessage(int x, int y, int alpha, byte r, byte g, byte b, LPCSTR Message);
		void getVisibleObjects(int from, int to, Vector4* viewPort, bool water, int count, RenderView& renderView);
		bool checkPortal(short roomIndex, ROOM_DOOR* portal, Vector4* viewPort, Vector4* clipPort,const Matrix& viewProjection);
		void collectRooms(RenderView& renderView);
		void collectItems(short roomNumber, RenderView& renderView);
		void collectStatics(short roomNumber, RenderView& renderView);
		void collectLightsForEffect(short roomNumber, RendererEffect* effect, RenderView& renderView);
		void collectLightsForItem(short roomNumber, RendererItem* item, RenderView& renderView);
		void collectLightsForRoom(short roomNumber, RenderView& renderView);
		void											prepareLights();
		void collectEffects(short roomNumber, RenderView& renderView);
		void											clearSceneItems();
		void											updateItemsAnimations(RenderView& view);
		void											updateEffects(RenderView& view);
		int												getFrame(short animation, short frame, ANIM_FRAME** framePtr, int* rate);
		bool											drawAmbientCubeMap(short roomNumber);
		bool											sphereBoxIntersection(DirectX::SimpleMath::Vector3 boxMin, DirectX::SimpleMath::Vector3 boxMax, DirectX::SimpleMath::Vector3 sphereCentre, float sphereRadius);
		bool											drawHorizonAndSky(ID3D11DepthStencilView* depthTarget);
		bool drawRooms(bool transparent, bool animated, RenderView& view);
		bool											drawItems(bool transparent, bool animated,RenderView& view);
		bool											drawAnimatingItem(RendererItem* item, bool transparent, bool animated);
		bool											drawBaddieGunflashes();
		bool											drawScaledSpikes(RendererItem* item, bool transparent, bool animated);
		bool drawStatics(bool transparent, RenderView& view);
		bool											drawWaterfalls();
		bool											drawWraithExtra(RendererItem* item, bool transparent, bool animated);
		bool											drawShadowMap();
		bool											drawObjectOn2DPosition(short x, short y, short objectNum, short rotX, short rotY, short rotZ);
		bool											drawLara(bool transparent, bool shadowMap);
		void											printDebugMessage(LPCSTR message, ...);
		void											drawFires();
		void											drawSparks();
		void											drawSmokes();
		void											drawEnergyArcs();
		void											drawBlood();
		void											drawDrips();
		void											drawBubbles();
		bool											drawEffects(bool transparent);
		bool											drawEffect(RendererEffect* effect, bool transparent);
		void											drawSplahes();
		bool											drawSprites();
		bool											drawLines3D();
		bool											drawLines2D();
		bool											drawOverlays();
		bool											drawRopes();
		bool											drawBats();
		bool											drawRats();
		bool											drawLittleBeetles();
		bool											drawSpiders();
		bool											drawGunFlashes();
		bool											drawGunShells();
		int drawInventoryScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget, ID3D11ShaderResourceView* background);
		bool											drawDebris(bool transparent);
		int												drawFinalPass();
		bool drawFullScreenImage(ID3D11ShaderResourceView* texture, float fade, ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget);
		void											updateAnimatedTextures();
		void											createBillboardMatrix(DirectX::SimpleMath::Matrix* out, DirectX::SimpleMath::Vector3* particlePos, DirectX::SimpleMath::Vector3* cameraPos, float rotation);
		void											drawShockwaves();
		void											drawRipples();
		void											drawUnderwaterDust();	
		bool											doRain();
		bool											doSnow();
		bool											drawFullScreenQuad(ID3D11ShaderResourceView* texture, DirectX::SimpleMath::Vector3 color, bool cinematicBars);
		bool											isRoomUnderwater(short roomNumber);
		bool											isInRoom(int x, int y, int z, short roomNumber);
		bool											drawColoredQuad(int x, int y, int w, int h, DirectX::SimpleMath::Vector4 color);
		bool											initialiseScreen(int w, int h, int refreshRate, bool windowed, HWND handle, bool reset);
		bool											initialiseBars();
		bool											drawSmokeParticles();
		bool											drawSparkParticles();
		bool                                            drawDripParticles();
		bool											drawExplosionParticles();
		void renderToCubemap(const RenderTargetCube& dest,const Vector3& pos,int roomNumber);
		void drawLaraHolsters(bool transparent);
	public:
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
	
		bool Create();
		bool EnumerateVideoModes();
		bool Initialise(int w, int h, int refreshRate, bool windowed, HWND handle);
		int Draw();
		bool PrepareDataForTheRenderer();
		void UpdateCameraMatrices(CAMERA_INFO* cam, float roll, float fov);
		bool drawSimpleScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget, RenderView& view);
		int DumpGameScene();
		int DrawInventory();
		int DrawTitle();
		bool drawScene(ID3D11RenderTargetView* target, ID3D11DepthStencilView* depthTarget, RenderView& view);
		int DrawPickup(short objectNum);
		int SyncRenderer();
		bool PrintString(int x, int y, char* string, D3DCOLOR color, int flags);
		void ClearDynamicLights();
		void AddDynamicLight(int x, int y, int z, short falloff, byte r, byte g, byte b);
		void FreeRendererData();
		void EnableCinematicBars(bool value);
		void FadeIn();
		void FadeOut();
		void DrawLoadingScreen(std::wstring& fileName);
		void UpdateProgress(float value);
		bool IsFading();
		void GetLaraBonePosition(DirectX::SimpleMath::Vector3* pos, int bone);
		bool ToggleFullScreen();
		bool IsFullsScreen();
		std::vector<RendererVideoAdapter>*					GetAdapters();
		bool DoTitleImage();
		void AddLine2D(int x1, int y1, int x2, int y2, byte r, byte g, byte b, byte a);
		void AddSpriteBillboard(RendererSprite* sprite, DirectX::SimpleMath::Vector3 pos,DirectX::SimpleMath::Vector4 color, float rotation, float scale, float width, float height, BLEND_MODES blendMode);
		void AddSpriteBillboardConstrained(RendererSprite* sprite, DirectX::SimpleMath::Vector3 pos, DirectX::SimpleMath::Vector4 color, float rotation, float scale, float width, float height, BLEND_MODES blendMode, DirectX::SimpleMath::Vector3 constrainAxis);
		void AddSpriteBillboardConstrainedLookAt(RendererSprite* sprite, DirectX::SimpleMath::Vector3 pos, DirectX::SimpleMath::Vector4 color, float rotation, float scale, float width, float height, BLEND_MODES blendMode, DirectX::SimpleMath::Vector3 lookAtAxis);
		void AddSprite3D(RendererSprite* sprite, DirectX::SimpleMath::Vector3 vtx1, DirectX::SimpleMath::Vector3 vtx2, DirectX::SimpleMath::Vector3 vtx3, DirectX::SimpleMath::Vector3 vtx4, DirectX::SimpleMath::Vector4 color, float rotation, float scale, float width, float height, BLEND_MODES blendMode);
		void AddLine3D(DirectX::SimpleMath::Vector3 start, DirectX::SimpleMath::Vector3 end, DirectX::SimpleMath::Vector4 color);
		bool ChangeScreenResolution(int width, int height, int frequency, bool windowed);
		bool DrawBar(float percent, const RendererHUDBar* const bar);
		void FlipRooms(short roomNumber1, short roomNumber2);
		void ResetAnimations();
		void UpdateLaraAnimations(bool force);
		void UpdateItemAnimations(int itemNumber, bool force);
		void GetLaraAbsBonePosition(DirectX::SimpleMath::Vector3* pos, int joint);
		void GetItemAbsBonePosition(int itemNumber, DirectX::SimpleMath::Vector3* pos, int joint);
		int GetSpheres(short itemNumber, BoundingSphere* ptr, char worldSpace, DirectX::SimpleMath::Matrix local);
		void GetBoneMatrix(short itemNumber, int joint, DirectX::SimpleMath::Matrix* outMatrix);
	
		RendererMesh* getMesh(int meshIndex);
	private:
		Texture2D CreateDefaultNormalTexture();
		void drawFootprints();

		template<typename CBuff>
		bool updateConstantBuffer(ID3D11Buffer* buffer, CBuff& data) {
			HRESULT res;
			D3D11_MAPPED_SUBRESOURCE mappedResource;

			// Lock the constant buffer so it can be written to.
			res = m_context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			if (FAILED(res))
				return false;

			// Get a pointer to the data in the constant buffer.
			void* dataPtr = (mappedResource.pData);
			memcpy(dataPtr, &data, sizeof(CBuff));
			// Unlock the constant buffer.
			m_context->Unmap(buffer, 0);

			return true;
		}

};
	extern Renderer11 g_Renderer;
}