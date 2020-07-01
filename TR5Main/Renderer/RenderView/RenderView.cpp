#include "framework.h"
#include "RenderView.h"

void T5M::Renderer::RenderView::createConstantBuffer(CCameraMatrixBuffer& bufferToFill) {
	bufferToFill.Projection = camera.Projection;
	bufferToFill.View = camera.View;
	bufferToFill.ViewProjection = camera.ViewProjection;
	bufferToFill.WorldDirection = camera.WorldDirection;
	bufferToFill.ViewSize = camera.ViewSize;
	bufferToFill.InvViewSize = camera.InvViewSize;
	bufferToFill.RoomNumber = camera.RoomNumber;
}

void T5M::Renderer::RenderView::clear() {
	roomsToDraw.clear();
	effectsToDraw.clear();
	staticsToDraw.clear();
	lightsToDraw.clear();
	itemsToDraw.clear();
	
}

T5M::Renderer::RenderViewCamera::RenderViewCamera(CAMERA_INFO* cam, float roll, float fov, float n, float f, int w, int h) {
	RoomNumber = cam->pos.roomNumber;
	WorldPosition = Vector3(cam->pos.x, cam->pos.y, cam->pos.z);
	WorldDirection = Vector3(cam->target.x, cam->target.y, cam->target.z) - WorldPosition;
	WorldDirection.Normalize();
	Vector3 up = -Vector3::UnitY;
	Matrix upRotation = Matrix::CreateFromYawPitchRoll(0.0f, 0.0f, roll);
	up = Vector3::Transform(up, upRotation);
	View = Matrix::CreateLookAt(WorldPosition, WorldPosition + WorldDirection, up);
	Projection = Matrix::CreatePerspectiveFieldOfView(fov, (float)w/h, n, f);
	ViewProjection = View * Projection;
	ViewSize = { (float)w,(float)h };
	InvViewSize = { 1.0f / w,1.0f / h };
	frustum.Update(View, Projection);
}

T5M::Renderer::RenderViewCamera::RenderViewCamera(const Vector3& pos, const Vector3& dir, const Vector3& up, int room, int width, int height, float fov, float n, float f) {
	RoomNumber = room;
	WorldPosition = pos;
	WorldDirection = dir;
	View = Matrix::CreateLookAt(pos, pos+dir, up);
	Projection = Matrix::CreatePerspectiveFieldOfView(fov, width / (float)height, n, f);
	ViewProjection = View * Projection;
	ViewSize = { (float)width,(float)height };
	InvViewSize = { 1.0f / width,1.0f / height };
	frustum.Update(View, Projection);
}
