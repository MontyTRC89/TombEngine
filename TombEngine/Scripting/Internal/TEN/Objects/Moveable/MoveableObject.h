#pragma once

#include "Objects/NamedBase.h"

namespace sol {
	class state;
	template <typename T> struct as_table_t;
}
class Vec3;
class Rotation;
class ScriptColor;

struct ItemInfo;
enum GAME_OBJECT_ID : short;

using aiBitsArray = std::array<int, 6>;
using aiBitsType = sol::as_table_t<aiBitsArray>;

class Moveable : public NamedBase<Moveable, short>
{
public:
	using IdentifierType = short;

	Moveable(short num, bool alreadyInitialised = true);
	~Moveable();
	Moveable& operator=(Moveable const& other) = delete;
	Moveable(Moveable const& other) = delete;
	Moveable(Moveable && other) noexcept;

	static void Register(sol::table & parent);

	[[nodiscard]] GAME_OBJECT_ID GetObjectID() const;
	void SetObjectID(GAME_OBJECT_ID id);

	[[nodiscard]] std::string GetName() const;
	bool SetName(std::string const &);

	[[nodiscard]] bool GetValid() const;
	void Invalidate();

	void Destroy();

	[[nodiscard]] Vec3 GetPos() const;
	[[nodiscard]] Vec3 GetJointPos(int index) const;
	void SetPos(Vec3 const& pos, sol::optional<bool> updateRoom);

	[[nodiscard]] Rotation GetRot() const;
	void SetRot(Rotation const& rot);

	[[nodiscard]] int GetStateNumber() const;
	void SetStateNumber(int stateNumber);

	[[nodiscard]] int GetAnimNumber() const;
	void SetAnimNumber(int animNumber);

	[[nodiscard]] int GetFrameNumber() const;
	void SetFrameNumber(int frameNumber);

	[[nodiscard]] ScriptColor GetColor() const;
	void SetColor(ScriptColor const& col);

	[[nodiscard]] short GetHP() const;
	void SetHP(short hp);

	[[nodiscard]] short GetSlotHP() const;

	[[nodiscard]] short GetOCB() const;
	void SetOCB(short ocb);

	[[nodiscard]] aiBitsType GetAIBits() const;
	void SetAIBits(aiBitsType const & bits);

	[[nodiscard]] short GetItemFlags(int index = 0) const;
	void SetItemFlags(short value, int index = 0);

	[[nodiscard]] bool MeshIsVisible(int meshId) const;
	void ShowMesh(int meshId);
	void HideMesh(int meshId);
	void ShatterMesh(int meshId);

	[[nodiscard]] bool MeshIsSwapped(int meshId) const;
	void SwapMesh(int meshId, int swapSlotId, sol::optional<int> swapMeshIndex);
	void UnswapMesh(int meshId);

	[[nodiscard]] bool GetHitStatus() const;

	[[nodiscard]] bool GetActive() const;
	void SetActive(bool active);

	[[nodiscard]] short GetRoom() const;
	void SetRoom(short room);

	void EnableItem();
	void DisableItem();
	void MakeInvisible();
	void Explode();
	void Shatter();

	[[nodiscard]] std::string GetOnHit() const;
	void SetOnHit(std::string const &);
	[[nodiscard]] std::string GetOnKilled() const;
	void SetOnKilled(std::string const &);
	[[nodiscard]] std::string GetOnCollidedWithObject() const;
	void SetOnCollidedWithObject(std::string const &);
	[[nodiscard]] std::string GetOnCollidedWithRoom() const;
	void SetOnCollidedWithRoom(std::string const &);

	[[nodiscard]] short GetStatus() const;

	void Init();

	friend bool operator==(Moveable const&, Moveable const&);
private:
	ItemInfo* m_item;
	short m_num;
	bool m_initialised;

	bool MeshExists(int number) const;
};
