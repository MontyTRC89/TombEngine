#pragma once

#include "Objects/NamedBase.h"
#include "Objects/Moveable/MoveableObject.h"
#include "Game/Control/volume.h"

namespace sol {
	class state;
}

class Vec3;
class Rotation;

class Volume : public NamedBase<Volume, TriggerVolume&>
{
public:
	using IdentifierType = std::reference_wrapper<TriggerVolume>;
	Volume(TriggerVolume& volume);
	~Volume() = default;

	Volume& operator=(Volume const& other) = delete;
	Volume(Volume const& other) = delete;

	static void Register(sol::table& parent);

	void Enable();
	void Disable();
	[[nodiscard]] bool GetActive() const;
	[[nodiscard]] Rotation GetRot() const;
	void SetRot(Rotation const& rot);
	[[nodiscard]] Vec3 GetPos() const;
	void SetPos(Vec3 const& pos);
	[[nodiscard]] Vec3 GetScale() const;
	void SetScale(Vec3 const& scale);
	[[nodiscard]] std::string GetName() const;
	void SetName(std::string const& name);

	void ClearActivators();
	[[nodiscard]] bool IsMoveableInside(Moveable const& moveable);

private:
	TriggerVolume& m_volume;
};
