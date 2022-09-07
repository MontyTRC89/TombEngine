#pragma once

#include "Objects/NamedBase.h"
#include "Math/Math.h"

namespace sol {
	class state;
}
class Vec3;

class Sink : public NamedBase<Sink, SINK_INFO &>
{
public:
	using IdentifierType = std::reference_wrapper<SINK_INFO>;
	Sink(SINK_INFO& ref);
	~Sink() = default;

	Sink& operator=(Sink const& other) = delete;
	Sink(Sink const& other) = delete;

	static void Register(sol::table &);
	Vec3 GetPos() const;
	void SetPos(Vec3 const& pos);

	int GetStrength() const;
	void SetStrength(int strength);

	std::string GetName() const;
	void SetName(std::string const &);

private:
	SINK_INFO & m_sink;
};
