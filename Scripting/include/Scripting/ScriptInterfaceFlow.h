#pragma once

class ScriptInterfaceLevel;

class ScriptInterfaceFlow {
public:
	virtual ~ScriptInterfaceFlow() = default;
	virtual void LoadGameFlowScript() = 0;
	virtual int	GetNumLevels() const = 0;
	virtual char const* GetString(const char* id) const = 0;
	virtual bool IsFlyCheatEnabled() const = 0;
	virtual bool HasCrawlExtended() const = 0;
	virtual bool HasCrouchRoll() const = 0;
	virtual bool HasCrawlspaceSwandive() const = 0;
	virtual bool HasMonkeyTurn180() const = 0;
	virtual bool HasMonkeyAutoJump() const = 0;
	virtual bool HasOscillateHang() const = 0;
	virtual bool HasAFKPose() const = 0;
	virtual ScriptInterfaceLevel * GetLevel(int level) = 0;
};

