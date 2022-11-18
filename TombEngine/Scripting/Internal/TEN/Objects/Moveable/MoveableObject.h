#pragma once

#include "ScriptUtil.h"
#include "Objects/NamedBase.h"

class LevelFunc;

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

	[[nodiscard]] Vec3 GetVelocity() const;
	void SetVelocity(Vec3 velocity);

	[[nodiscard]] ScriptColor GetColor() const;
	void SetColor(ScriptColor const& col);

	[[nodiscard]] short GetHP() const;
	void SetHP(short hp);

	[[nodiscard]] short GetSlotHP() const;

	[[nodiscard]] short GetOCB() const;
	void SetOCB(short ocb);

	[[nodiscard]] bool GetOnFire() const;
	void SetOnFire(bool onFire);

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

	void AttachObjCamera(short camMeshId, Moveable& mov, short targetMeshId);
	void AnimFromObject(GAME_OBJECT_ID object, int animNumber, int stateID);

	void EnableItem();
	void DisableItem();
	void MakeInvisible();
	void Explode();
	void Shatter();

	void SetOnHit(TypeOrNil<LevelFunc> const& cb);
	void SetOnKilled(TypeOrNil<LevelFunc> const& cb);
	void SetOnCollidedWithObject(TypeOrNil<LevelFunc> const& cb);
	void SetOnCollidedWithRoom(TypeOrNil<LevelFunc> const& cb);

	[[nodiscard]] short GetStatus() const;

	void Init();

	friend bool operator==(Moveable const&, Moveable const&);
	friend void SetLevelFuncCallback(TypeOrNil<LevelFunc> const& cb, std::string const & callerName, Moveable& mov, std::string& toModify);

protected:
	ItemInfo* m_item;

private:
	short m_num;
	bool m_initialised;

	bool MeshExists(int number) const;
};
