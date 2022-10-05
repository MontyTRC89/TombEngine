#pragma once

#include "Objects/NamedBase.h"
#include "Specific/phd_global.h"

namespace sol {
	class state;
}
class Vec3;

class Camera : public NamedBase<Camera, LevelCameraInfo &>
{
public:
	using IdentifierType = std::reference_wrapper<LevelCameraInfo>;
	Camera(LevelCameraInfo& ref);
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
	LevelCameraInfo & m_camera;
};
