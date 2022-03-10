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
anims.hasSlideExtended = true;
anims.hasCrawlExtended = true;
anims.hasCrouchRoll = true;
anims.hasCrawlspaceSwandive = true;
anims.hasOverhangClimb = true;
anims.hasPose = false;
anims.hasMonkeyAutoJump = false;
SetAnimations(anims);