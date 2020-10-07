#pragma once
#include <vector>
#include <d3d11.h>
#include <SimpleMath.h>
#include "camera.h"
#include "Frustum.h"
#include "ConstantBuffers/CameraMatrixBuffer.h"
#include "RendererSprites.h"
#include "memory/ArrayBuffer.h"
namespace T5M::Renderer {
	struct RendererStatic;
	struct RendererItem;
	struct RendererLight;
	struct RendererEffect;
	struct RendererRoom;
	using DirectX::SimpleMath::Vector3;
	using DirectX::SimpleMath::Vector2;
	using DirectX::SimpleMath::Matrix;
	class RenderViewCamera {
		friend class RenderView;
		friend class Renderer11;
		Matrix ViewProjection;
		Matrix View;
		Matrix Projection;
		Vector3 WorldPosition;
		Vector3 WorldDirection;
		Vector2 ViewSize;
		Vector2 InvViewSize;
		int RoomNumber;
		Frustum frustum;
		RenderViewCamera(CAMERA_INFO* cam, float roll, float fov, float n, float f, int w, int h);
		RenderViewCamera(const Vector3& pos, const Vector3& dir,const Vector3& up, int room,int width, int height,float fov,float n, float f);
	};
	class RenderView {
		friend class Renderer11;
		RenderViewCamera camera;
		D3D11_VIEWPORT viewport;
	public:
		T5M::Memory::LinearArrayBuffer<RendererRoom*,512> roomsToDraw;
		T5M::Memory::LinearArrayBuffer<RendererStatic*,512> staticsToDraw;
		T5M::Memory::LinearArrayBuffer<RendererEffect*,1024> effectsToDraw;
		T5M::Memory::LinearArrayBuffer<RendererItem*,512> itemsToDraw;
		T5M::Memory::LinearArrayBuffer<RendererLight*,128> lightsToDraw;
		T5M::Memory::LinearArrayBuffer<RendererSpriteToDraw,512> spritesToDraw;
		size_t numSpritesToDraw;
		size_t numLightsToDraw;
		RenderView(CAMERA_INFO* cam, float roll, float fov, float nearPlane, float farPlane, int w, int h);
		RenderView(const Vector3& pos, const Vector3& dir, const Vector3& up, int w, int h, int room, float nearPlane, float farPlane, float fov);
		void fillConstantBuffer(CCameraMatrixBuffer& bufferToFill);
		void clear();
	};
}
