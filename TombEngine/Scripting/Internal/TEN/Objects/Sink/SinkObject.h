#pragma once
#include "Objects/NamedBase.h"
#include "Objects/Sink.h"
#include "Math/Math.h"

namespace sol
{
	class state;
}
class Vec3;

class Sink : public NamedBase<Sink, SinkInfo &>
{
public:
	using IdentifierType = std::reference_wrapper<SinkInfo>;
	Sink(SinkInfo& ref);
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
	SinkInfo& m_sink;
};
