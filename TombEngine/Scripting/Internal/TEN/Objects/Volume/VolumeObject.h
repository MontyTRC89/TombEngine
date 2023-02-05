#pragma once
#include "Game/Control/volume.h"
#include "Scripting/Internal/TEN/Objects/Moveable/MoveableObject.h"
#include "Scripting/Internal/TEN/Objects/NamedBase.h"

namespace sol
{
	class state;
}

class Rotation;
class Vec3;

class Volume : public NamedBase<Volume, TriggerVolume&>
{
public:
	using IdentifierType = std::reference_wrapper<TriggerVolume>;
	Volume(TriggerVolume& volume);
	~Volume() = default;

	Volume& operator =(const Volume& other) = delete;
	Volume(const Volume& other) = delete;

	static void Register(sol::table& parent);

	void Enable();
	void Disable();
	[[nodiscard]] bool GetActive() const;
	[[nodiscard]] Rotation GetRot() const;
	void SetRot(const Rotation& rot);
	[[nodiscard]] Vec3 GetPos() const;
	void SetPos(const Vec3& pos);
	[[nodiscard]] Vec3 GetScale() const;
	void SetScale(const Vec3& scale);
	[[nodiscard]] std::string GetName() const;
	void SetName(const std::string& name);

	void ClearActivators();
	[[nodiscard]] bool IsMoveableInside(const Moveable& moveable);

private:
	TriggerVolume& m_volume;
};
