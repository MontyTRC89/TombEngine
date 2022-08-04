-- TombEngine settings file
-- WARNING: Bad values could make your game unplayable; please follow reference guide attentively.

local Flow = TEN.Flow

local settings = Flow.Settings.new()
settings.errorMode = Flow.ErrorMode.WARN
Flow.SetSettings(settings)

local anims = Flow.Animations.new()
anims.crawlExtended = true
anims.crouchRoll = true
anims.crawlspaceSwandive = true
anims.monkeyAutoJump = false
anims.overhangClimb = false
anims.slideExtended = false
anims.sprintJump = false
anims.pose = false
Flow.SetAnimations(anims)