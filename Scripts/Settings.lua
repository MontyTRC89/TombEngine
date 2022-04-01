-- TombEngine settings file
-- Created by MontyTRC
-- Place all the engine settings for your game in this LUA script.
-- WARNING: Bad values could make your game unplayable; please follow reference guide attentively.

local settings = Settings.new();
settings.screenWidth = 1920;
settings.screenHeight = 1080;
settings.enableDynamicShadows = true;
settings.enableWaterCaustics = true;
settings.windowed = true;
settings.drawingDistance = 102400;
settings.showRendererSteps = false;
settings.showDebugInfo = true;
settings.errorMode = ErrorMode.WARN;
SetSettings(settings);

local anims = Animations.new();
anims.hasPose = false;
anims.hasSlideExtended = false;
anims.hasSprintJump = true;
anims.hasMonkeyAutoJump = false;
anims.hasCrawlspaceDive = true;
anims.hasCrawlExtended = true;
anims.hasCrouchRoll = true;
anims.hasOverhangClimb = true;
SetAnimations(anims);