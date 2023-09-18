#pragma once

struct ItemInfo;

namespace TEN::Animation
{
	// NOTE: Only used when loading animation commands from level file.
	enum class AnimCommandType
	{
		None,
		MoveOrigin, // "Post-animation adjustment"
		JumpVelocity,
		AttackReady,
		Deactivate,
		SoundEffect,
		Flipeffect
	};

	// Abstract base class used by high-level animation command classes.
	class AnimCommand abstract
	{
	public:
		virtual void Execute(ItemInfo& item, bool isFrameBased) const = 0;
	};

	class MoveOriginCommand : public AnimCommand
	{
	private:
		const Vector3 RelOffset = Vector3::Zero;
		
	public:
		MoveOriginCommand(const Vector3& relOffset) : RelOffset(relOffset) {};
		void Execute(ItemInfo& item, bool isFrameBased) const override;
	};
	
	class JumpVelocityCommand : public AnimCommand
	{
	private:
		const Vector3 JumpVelocity = Vector3::Zero;

	public:
		JumpVelocityCommand(const Vector3& jumpVel) : JumpVelocity(jumpVel) {};
		void Execute(ItemInfo& item, bool isFrameBased) const override;
	};

	class AttackReadyCommand : public AnimCommand
	{
	public:
		AttackReadyCommand() {};
		void Execute(ItemInfo& item, bool isFrameBased) const override;
	};
	
	class DeactivateCommand : public AnimCommand
	{
	public:
		DeactivateCommand() {};
		void Execute(ItemInfo& item, bool isFrameBased) const override;
	};

	class SoundEffectCommand : public AnimCommand
	{
	private:
		const int SoundID	  = 0;
		const int FrameNumber = 0;

	public:
		SoundEffectCommand(int soundID, int frameNumber) : SoundID(soundID), FrameNumber(frameNumber) {};
		void Execute(ItemInfo& item, bool isFrameBased) const override;
	};

	class FlipEffectCommand : public AnimCommand
	{
	private:
		const int FlipEffectID = 0;
		const int FrameNumber  = 0;

	public:
		FlipEffectCommand(int flipEffectID, int frameNumber) : FlipEffectID(flipEffectID), FrameNumber(frameNumber) {};
		void Execute(ItemInfo& item, bool isFrameBased) const override;
	};
}
