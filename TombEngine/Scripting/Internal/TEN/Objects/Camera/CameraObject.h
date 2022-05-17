#pragma once

#include "Objects/NamedBase.h"
#include "Specific/phd_global.h"

namespace sol {
	class state;
}
class Vec3;

class Camera : public NamedBase<Camera, LEVEL_CAMERA_INFO &>
{
public:
	using IdentifierType = std::reference_wrapper<LEVEL_CAMERA_INFO>;
	Camera(LEVEL_CAMERA_INFO& ref);
	~Camera() = default;

	Camera& operator=(Camera const& other) = delete;
	Camera(Camera const& other) = delete;

	static void Register(sol::table &);
	Vec3 GetPos() const;
	void SetPos(Vec3 const& pos);

	short GetRoom() const;
	void SetRoom(short room);

	std::string GetName() const;
	void SetName(std::string const &);

private:
	LEVEL_CAMERA_INFO & m_camera;
};
