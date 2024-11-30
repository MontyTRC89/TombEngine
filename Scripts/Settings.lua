-- TombEngine settings file
-- WARNING: Bad values could make your game unplayable; please follow reference guide attentively.

local Flow = TEN.Flow

local settings = Flow.Settings.new()

	settings.Animations.crawlExtended = true
	settings.Animations.crouchRoll = true
	settings.Animations.crawlspaceSwandive = true
	settings.Animations.overhangClimb = false
	settings.Animations.slideExtended = false
	settings.Animations.sprintJump = false
	settings.Animations.ledgeJumps = false
	settings.Animations.poseTimeout = 0
	
	settings.Flare.color = Color(240, 150, 0)
	settings.Flare.range = 10
	settings.Flare.timeout = 60
	settings.Flare.lensflareBrightness = 0.5
	settings.Flare.sparks = true
	settings.Flare.smoke = true
	settings.Flare.flicker = true
	
	settings.System.errorMode = Flow.ErrorMode.WARN
	settings.System.fastReload = true
	
Flow.SetSettings(settings)