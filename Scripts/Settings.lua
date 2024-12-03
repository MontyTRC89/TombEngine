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
	
	settings.Hud.statusBars = true
	settings.Hud.loadingBar = true
	settings.Hud.speedometer = true
	settings.Hud.pickupNotifier = true
	
	settings.Weapons[WeaponType.PISTOL].accuracy = 8
	settings.Weapons[WeaponType.PISTOL].distance = 8192
	settings.Weapons[WeaponType.PISTOL].recoilTimeout = 9
	settings.Weapons[WeaponType.PISTOL].waterLevel = 650
	settings.Weapons[WeaponType.PISTOL].flashDuration = 3
	settings.Weapons[WeaponType.PISTOL].damage = 1
	settings.Weapons[WeaponType.PISTOL].secondaryDamage = 1
	
	settings.Weapons[WeaponType.REVOLVER].accuracy = 8
	settings.Weapons[WeaponType.REVOLVER].distance = 8192
	settings.Weapons[WeaponType.REVOLVER].recoilTimeout = 16
	settings.Weapons[WeaponType.REVOLVER].waterLevel = 650
	settings.Weapons[WeaponType.REVOLVER].flashDuration = 3
	settings.Weapons[WeaponType.REVOLVER].damage = 21
	settings.Weapons[WeaponType.REVOLVER].secondaryDamage = 21
	
	settings.Weapons[WeaponType.UZI].accuracy = 8
	settings.Weapons[WeaponType.UZI].distance = 8192
	settings.Weapons[WeaponType.UZI].recoilTimeout = 3
	settings.Weapons[WeaponType.UZI].waterLevel = 650
	settings.Weapons[WeaponType.UZI].flashDuration = 3
	settings.Weapons[WeaponType.UZI].damage = 1
	settings.Weapons[WeaponType.UZI].secondaryDamage = 1
	
	settings.Weapons[WeaponType.SHOTGUN].accuracy = 10
	settings.Weapons[WeaponType.SHOTGUN].distance = 8192
	settings.Weapons[WeaponType.SHOTGUN].recoilTimeout = 9
	settings.Weapons[WeaponType.SHOTGUN].waterLevel = 500
	settings.Weapons[WeaponType.SHOTGUN].flashDuration = 3
	settings.Weapons[WeaponType.SHOTGUN].damage = 3
	settings.Weapons[WeaponType.SHOTGUN].secondaryDamage = 3
	
	settings.Weapons[WeaponType.CROSSBOW].accuracy = 8
	settings.Weapons[WeaponType.CROSSBOW].distance = 8192
	settings.Weapons[WeaponType.CROSSBOW].recoilTimeout = 0
	settings.Weapons[WeaponType.CROSSBOW].waterLevel = 500
	settings.Weapons[WeaponType.CROSSBOW].flashDuration = 2
	settings.Weapons[WeaponType.CROSSBOW].damage = 5
	settings.Weapons[WeaponType.CROSSBOW].secondaryDamage = 20
	
	settings.Weapons[WeaponType.HK].accuracy = 4
	settings.Weapons[WeaponType.HK].distance = 12288
	settings.Weapons[WeaponType.HK].recoilTimeout = 0
	settings.Weapons[WeaponType.HK].waterLevel = 500
	settings.Weapons[WeaponType.HK].flashDuration = 3
	settings.Weapons[WeaponType.HK].damage = 4
	settings.Weapons[WeaponType.HK].secondaryDamage = 4
	
	settings.Weapons[WeaponType.GRENADE_LAUNCHER].accuracy = 8
	settings.Weapons[WeaponType.GRENADE_LAUNCHER].distance = 8192
	settings.Weapons[WeaponType.GRENADE_LAUNCHER].recoilTimeout = 0
	settings.Weapons[WeaponType.GRENADE_LAUNCHER].waterLevel = 500
	settings.Weapons[WeaponType.GRENADE_LAUNCHER].flashDuration = 2
	settings.Weapons[WeaponType.GRENADE_LAUNCHER].damage = 20
	settings.Weapons[WeaponType.GRENADE_LAUNCHER].secondaryDamage = 30
	
	settings.Weapons[WeaponType.ROCKET_LAUNCHER].accuracy = 8
	settings.Weapons[WeaponType.ROCKET_LAUNCHER].distance = 8192
	settings.Weapons[WeaponType.ROCKET_LAUNCHER].recoilTimeout = 0
	settings.Weapons[WeaponType.ROCKET_LAUNCHER].waterLevel = 500
	settings.Weapons[WeaponType.ROCKET_LAUNCHER].flashDuration = 2
	settings.Weapons[WeaponType.ROCKET_LAUNCHER].damage = 30
	settings.Weapons[WeaponType.ROCKET_LAUNCHER].secondaryDamage = 30
	
	settings.Weapons[WeaponType.HARPOON_GUN].accuracy = 8
	settings.Weapons[WeaponType.HARPOON_GUN].distance = 8192
	settings.Weapons[WeaponType.HARPOON_GUN].recoilTimeout = 0
	settings.Weapons[WeaponType.HARPOON_GUN].waterLevel = 500
	settings.Weapons[WeaponType.HARPOON_GUN].flashDuration = 2
	settings.Weapons[WeaponType.HARPOON_GUN].damage = 6
	settings.Weapons[WeaponType.HARPOON_GUN].secondaryDamage = 6
	
	settings.System.errorMode = Flow.ErrorMode.WARN
	settings.System.fastReload = true
	
Flow.SetSettings(settings)