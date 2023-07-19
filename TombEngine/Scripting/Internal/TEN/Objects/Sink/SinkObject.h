#pragma once
#include "Scripting/Internal/TEN/Objects/NamedBase.h"
#include "Objects/Sink.h"
#include "Math/Math.h"

namespace sol { class state; }
class Vec3;

class Sink : public NamedBase<Sink, SinkInfo &>
{
public:
	using IdentifierType = std::reference_wrapper<SinkInfo>;
	Sink(SinkInfo& ref);
	~Sink() = default;

	Sink& operator =(const Sink& other) = delete;
	Sink(Sink const& other) = delete;

	static void Register(sol::table& parent);
	Vec3 GetPos() const;
	void SetPos(const Vec3& pos);

	int GetStrength() const;
	void SetStrength(int strength);

	std::string GetName() const;
	void SetName(const std::string&);

private:
	SinkInfo& m_sink;
};
