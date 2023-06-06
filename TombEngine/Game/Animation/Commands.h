#pragma once

struct ItemInfo;

namespace TEN::Animation
{
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
		const int SoundID     = 0;
		const int FrameNumber = 0;

	public:
		SoundEffectCommand(int soundID, int frameNumber) : SoundID(soundID), FrameNumber(frameNumber) {};
		void Execute(ItemInfo& item, bool isFrameBased) const override;
	};

	class FlipeffectCommand : public AnimCommand
	{
	private:
		const int FlipeffectID = 0;
		const int FrameNumber  = 0;

	public:
		FlipeffectCommand(int flipeffectID, int frameNumber) : FlipeffectID(flipeffectID), FrameNumber(frameNumber) {};
		void Execute(ItemInfo& item, bool isFrameBased) const override;
	};
}
