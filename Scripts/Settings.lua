-- TR5Main engine settings file
-- Created by MontyTRC
-- Place in this LUA script all the engine settings for your game
-- WARNING: bad values could make your game unplayable, please follow with attention the reference guide

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
anims.crawlExtra = true;
anims.crouchRoll = true;
anims.monkeyRoll = true;
anims.monkeyVault = true;
anims.oscillateHanging = true;
anims.swandiveRollRun = true;
SetAnimations(anims);