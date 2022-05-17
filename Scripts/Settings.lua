-- TombEngine settings file
-- Created by MontyTRC
-- Place all the engine settings for your game in this Lua script.
-- WARNING: Bad values could make your game unplayable; please follow reference guide attentively.

local Flow = TEN.Flow

local settings = Flow.Settings.new();
settings.screenWidth = 1920;
settings.screenHeight = 1080;
settings.enableDynamicShadows = true;
settings.enableWaterCaustics = true;
settings.windowed = true;
settings.drawingDistance = 102400;
settings.showRendererSteps = false;
settings.showDebugInfo = true;
settings.errorMode = Flow.ErrorMode.WARN;
Flow.SetSettings(settings);

local anims = Flow.Animations.new();
anims.crawlExtended = true;
anims.crouchRoll = true;
anims.crawlspaceSwandive = true;
anims.monkeyAutoJump = false;
anims.pose = false;
Flow.SetAnimations(anims);