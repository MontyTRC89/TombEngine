#pragma once
#include <vector>
#include <d3d11.h>
#include <SimpleMath.h>
#include "camera.h"
#include "Frustum.h"
#include "ConstantBuffers/CameraMatrixBuffer.h"
namespace T5M::Renderer {
	struct RendererStatic;
	struct RendererItem;
	struct RendererLight;
	struct RendererEffect;
	struct RendererRoom;
	using DirectX::SimpleMath::Vector3;
	using DirectX::SimpleMath::Vector2;
	using DirectX::SimpleMath::Matrix;
	struct RenderViewCamera {
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
	struct RenderView {
		RenderViewCamera camera;
		std::vector<RendererRoom*> roomsToDraw;
		std::vector<RendererStatic*> staticsToDraw;
		std::vector<RendererEffect*> effectsToDraw;
		std::vector<RendererItem*> itemsToDraw;
		std::vector<RendererLight*> lightsToDraw;
		RenderView(CAMERA_INFO* cam, float roll, float fov, float nearPlane, float farPlane, int w, int h) : camera(cam, roll, fov, nearPlane, farPlane,w,h) {};
		RenderView(const Vector3& pos,const Vector3& dir,const Vector3& up,int w, int h,int room, float nearPlane, float farPlane,float fov) : camera(pos,dir,up,room,w,h,fov,nearPlane,farPlane) {};
		void createConstantBuffer(CCameraMatrixBuffer& bufferToFill);
		void clear();
	};
}
