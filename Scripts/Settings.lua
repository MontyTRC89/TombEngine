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
	
	settings.Flare.color = Color(128, 60, 0)
	settings.Flare.range = 10
	settings.Flare.timeout = 60
	settings.Flare.lensflareBrightness = 0.5
	settings.Flare.sparks = true
	settings.Flare.smoke = true
	settings.Flare.flicker = true
	
	settings.Hud.statusBars = true
	settings.Hud.loadingBar = true
	settings.Hud.speedometer = true
	settings.Hud.pickupNotifier = true
	
	settings.Physics.gravity = 6
	settings.Physics.swimVelocity = 50
	
	settings.System.errorMode = Flow.ErrorMode.WARN
	settings.System.fastReload = true
	
	-- Example hair setting entry. You can use types [2] and [3] as well for young Lara hair.
	-- To know default parameters, you can fetch settings using Flow.GetSettings() method.
	
	settings.Hair[1].offset = Vec3(-4, -4, -48)
	settings.Hair[1].indices = { 37, 39, 40, 38 }
	
	-- Example  setting entry. You can use other weapon types as well.
	-- To know default parameters, you can fetch settings using Flow.GetSettings() method.
	
	settings.Weapons[WeaponType.PISTOL].accuracy = 8
	settings.Weapons[WeaponType.PISTOL].distance = 8192
	settings.Weapons[WeaponType.PISTOL].interval = 9
	settings.Weapons[WeaponType.PISTOL].waterLevel = 650
	settings.Weapons[WeaponType.PISTOL].flashDuration = 3
	settings.Weapons[WeaponType.PISTOL].damage = 1
	settings.Weapons[WeaponType.PISTOL].alternateDamage = 1
	
Flow.SetSettings(settings)