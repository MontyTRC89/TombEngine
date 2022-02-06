#pragma once

#include "GameScriptNamedBase.h"
#include "Specific/phd_global.h"

namespace sol {
	class state;
}
class GameScriptPosition;

class Camera : public GameScriptNamedBase<Camera, LEVEL_CAMERA_INFO &>
{
public:
	using IdentifierType = std::reference_wrapper<LEVEL_CAMERA_INFO>;
	Camera(LEVEL_CAMERA_INFO& ref, bool temp);
	~Camera();

	Camera& operator=(Camera const& other) = delete;
	Camera(Camera const& other) = delete;

	static void Register(sol::table &);
	GameScriptPosition GetPos() const;
	void SetPos(GameScriptPosition const& pos);

	short GetRoom() const;
	void SetRoom(short Room);

	std::string GetName() const;
	void SetName(std::string const &);

private:
	LEVEL_CAMERA_INFO & m_camera;
	bool m_temporary;
};
