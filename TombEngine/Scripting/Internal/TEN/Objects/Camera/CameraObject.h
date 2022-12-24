#pragma once

#include "Objects/NamedBase.h"

struct LevelCameraInfo;

namespace sol
{
	class state;
}
class Vec3;

class CameraObject : public NamedBase<CameraObject, LevelCameraInfo &>
{
public:
	using IdentifierType = std::reference_wrapper<LevelCameraInfo>;
	CameraObject(LevelCameraInfo& ref);
	~CameraObject() = default;

	CameraObject& operator=(CameraObject const& other) = delete;
	CameraObject(CameraObject const& other) = delete;

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
