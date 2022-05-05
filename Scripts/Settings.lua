-- TombEngine settings file
-- Created by MontyTRC
<<<<<<< HEAD
-- Place in this Lua script all the engine settings for your game
-- WARNING: bad values could make your game unplayable, please follow with attention the reference guide
=======
-- Place all the engine settings for your game in this Lua script.
-- WARNING: Bad values could make your game unplayable; please follow reference guide attentively.
>>>>>>> state_cleaning_tier_2

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

<<<<<<< HEAD
local anims = Flow.Animations.new();
anims.crawlExtended = true;
anims.crouchRoll = true;
anims.crawlspaceSwandive = true;
anims.monkeyTurn180 = true;
anims.monkeyAutoJump = false;
anims.oscillateHang = true;
anims.pose = false;
Flow.SetAnimations(anims);
=======
local anims = Animations.new();
anims.hasPose = false;
anims.hasSlideExtended = false;
anims.hasSprintJump = true;
anims.hasMonkeyAutoJump = false;
anims.hasCrawlspaceDive = true;
anims.hasCrawlExtended = true;
anims.hasCrouchRoll = true;
anims.hasOverhangClimb = false;
SetAnimations(anims);
>>>>>>> state_cleaning_tier_2
