#pragma once

#include "Objects/NamedBase.h"
#include "Specific/phd_global.h"

namespace sol {
	class state;
}
class Position;

class Sink : public NamedBase<Sink, SINK_INFO &>
{
public:
	using IdentifierType = std::reference_wrapper<SINK_INFO>;
	Sink(SINK_INFO& ref, bool temp);
	~Sink();
	Sink& operator=(Sink const& other) = delete;
	Sink(Sink const& other) = delete;

	static void Register(sol::table &);
	Position GetPos() const;
	void SetPos(Position const& pos);

	int GetStrength() const;
	void SetStrength(int strength);

	int GetBoxIndex() const;
	void SetBoxIndex(int Room);

	std::string GetName() const;
	void SetName(std::string const &);

private:
	SINK_INFO & m_sink;
	bool m_temporary;
};
