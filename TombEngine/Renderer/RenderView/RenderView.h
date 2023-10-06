#pragma once
#include "Game/camera.h"
#include "Renderer/ConstantBuffers/CameraMatrixBuffer.h"
#include "Renderer/Frustum.h"
#include "Specific/memory/LinearArrayBuffer.h"
#include "Renderer/RendererSprite2D.h"
#include "Renderer/RendererSprites.h"
#include "Renderer/RendererTransparentFace.h"
#include "Renderer/Structures/RendererFogBulb.h"

namespace TEN::Renderer 
{
	struct RendererStatic;
	struct RendererItem;
	struct RendererLight;
	struct RendererEffect;
	struct RendererRoom;
	struct RendererTransparentFace;

	constexpr auto MAX_ROOMS_DRAW = 256;
	constexpr auto MAX_STATICS_DRAW = 128;
	constexpr auto MAX_EFFECTS_DRAW = 16;
	constexpr auto MAX_ITEMS_DRAW = 128;
	constexpr auto MAX_LIGHTS_DRAW = 48;
	constexpr auto MAX_FOG_BULBS_DRAW = 32;
	constexpr auto MAX_SPRITES_DRAW = 512;

	struct RenderViewCamera
	{
		Matrix ViewProjection;
		Matrix View;
		Matrix Projection;
		Vector3 WorldPosition;
		Vector3 WorldDirection;
		Vector2 ViewSize;
		Vector2 InvViewSize;
		int RoomNumber;
		Frustum Frustum;
		float NearPlane;
		float FarPlane;

		RenderViewCamera(CAMERA_INFO* cam, float roll, float fov, float n, float f, int w, int h);
		RenderViewCamera(const Vector3& pos, const Vector3& dir, const Vector3& up, int room, int width, int height, float fov, float n, float f);
	};

	struct RenderView
	{
		RenderViewCamera Camera;
		D3D11_VIEWPORT	 Viewport;

		std::vector<RendererRoom*>					RoomsToDraw			 = {};
		std::vector<RendererLight*>					LightsToDraw		 = {};
		std::vector<RendererFogBulb>				FogBulbsToDraw		 = {};
		std::vector<RendererSpriteToDraw>			SpritesToDraw		 = {};
		std::vector<RendererDisplaySpriteToDraw>	DisplaySpritesToDraw = {};
		std::map<int, std::vector<RendererStatic*>> SortedStaticsToDraw	 = {};

		RenderView(CAMERA_INFO* cam, float roll, float fov, float nearPlane, float farPlane, int w, int h);
		RenderView(const Vector3& pos, const Vector3& dir, const Vector3& up, int w, int h, int room, float nearPlane, float farPlane, float fov);
		
		void fillConstantBuffer(CCameraMatrixBuffer& bufferToFill);
		void clear();
	};
}
