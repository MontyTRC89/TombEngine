#include "framework.h"
#include "RenderView.h"

namespace TEN::Renderer
{
	constexpr float EPSILON = 0.00001f;

	RenderView::RenderView(CAMERA_INFO* cam, float roll, float fov, float nearPlane, float farPlane, int w, int h) : camera(cam, roll, fov, nearPlane, farPlane, w, h) 
	{
		viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = w;
		viewport.Height = h;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;
	}

	RenderView::RenderView(const Vector3& pos, const Vector3& dir, const Vector3& up, int w, int h, int room, float nearPlane, float farPlane, float fov) : camera(pos, dir, up, room, w, h, fov, nearPlane, farPlane) 
	{

		viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = w;
		viewport.Height = h;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;
	}

	void RenderView::fillConstantBuffer(CCameraMatrixBuffer& bufferToFill)
	{
		bufferToFill.Projection = camera.Projection;
		bufferToFill.View = camera.View;
		bufferToFill.ViewProjection = camera.ViewProjection;
		bufferToFill.CamDirectionWS = Vector4(camera.WorldDirection);
		bufferToFill.CamPositionWS = Vector4(camera.WorldPosition);
		bufferToFill.ViewSize = camera.ViewSize;
		bufferToFill.InvViewSize = camera.InvViewSize;
		bufferToFill.RoomNumber = camera.RoomNumber;
	}

	void RenderView::clear() 
	{
		roomsToDraw.clear();
		lightsToDraw.clear();
		spritesToDraw.clear();
	}

	RenderViewCamera::RenderViewCamera(CAMERA_INFO* cam, float roll, float fov, float n, float f, int w, int h)
	{
		RoomNumber = cam->pos.roomNumber;
		WorldPosition = Vector3(cam->pos.x, cam->pos.y, cam->pos.z);

		Vector3 target = Vector3(cam->target.x, cam->target.y, cam->target.z);
		if ((target - WorldPosition) == Vector3::Zero)
			target.y -= 10;

		WorldDirection = target - WorldPosition;
		WorldDirection.Normalize();
		
		Vector3 up = -Vector3::UnitY;
		Matrix upRotation = Matrix::CreateFromYawPitchRoll(0.0f, 0.0f, roll);
		up = Vector3::Transform(up, upRotation);
		up.Normalize();

		View = Matrix::CreateLookAt(WorldPosition, target, up);
		Projection = Matrix::CreatePerspectiveFieldOfView(fov, w / (float)h, n, f);
		ViewProjection = View * Projection;
		ViewSize = { (float)w,(float)h };
		InvViewSize = { 1.0f / w,1.0f / h };
		frustum.Update(View, Projection);
	}

	RenderViewCamera::RenderViewCamera(const Vector3& pos, const Vector3& dir, const Vector3& up, int room, int width, int height, float fov, float n, float f) 
	{
		RoomNumber = room;
		WorldPosition = pos;
		WorldDirection = dir;
		View = Matrix::CreateLookAt(pos, pos + dir*10240, up);
		float aspect = (float)width / (float)height;
		Projection = Matrix::CreatePerspectiveFieldOfView(fov, aspect, n, f);
		ViewProjection = View * Projection;
		ViewSize = { (float)width,(float)height };
		InvViewSize = { 1.0f / width,1.0f / height };
		frustum.Update(View, Projection);
	}
}
