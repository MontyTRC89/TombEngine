-----
--- Custom Bars - Draws custom bars on screen.
--The module provides functions to create and manage custom progress bars. It maintains state through LevelVars.Engine.CustomBars, which stores bar definitions and configurations.
-- Each bar is controlled by its specific functions. 
--
-- Example usage:
--	local CustomBar = require("Engine.CustomBar")
--
--	local ObjID = TEN.Objects.ObjID.CUSTOM_BAR_GRAPHIC
--	local colorBar = TEN.Color(255,255,255)
--	local posBar =  TEN.Vec2(21.6, 5.79)
--	local scaleBar = TEN.Vec2(19.05, 19.1)
--	local posBG = TEN.Vec2(posBar.x+0.15, posBar.y)
--	local scaleBG = TEN.Vec2(scaleBar.x-0.35, scaleBar.y-0.62)
--	local alignMode = TEN.View.AlignMode.CENTER
--	local scaleMode = TEN.View.ScaleMode.FIT
--	local blendMode = TEN.Effects.BlendID.ALPHATEST
--	local textPos = TEN.Vec2(posBar.x, posBar.y+10)
--
--	-- This function creates the bar and displays it
--	CustomBar.Create("Test", 50, 1000,
--		ObjID, 2, colorBar, posBar, 0, scaleBar, alignMode, scaleMode, blendMode,
--		ObjID, 4, colorBar, posBG, 0, scaleBG, alignMode, scaleMode, blendMode,
--		"Test 1", textPos, {}, 1, colorBar, false, 50, true, 0.25)
--
--		-- This method displays the bar
--		bar:SetVisibility(true)
--	end
--
-- @luautil CustomBar


local CustomBar = {}

CustomBar.__index = CustomBar

LevelFuncs.Engine.CustomBar = {}
LevelVars.Engine.CustomBars = {bars = {}, EnemiesHpBar = {status = nil}}

---
-- Creates a custom progress bar with extensive configuration options.
-- Parameters:
-- @tparam string barName Unique name for the bar.
-- @tparam float startvalue Initial value of the bar.
-- @tparam float maxvalue Maximum value of the bar.
-- @tparam Objects.ObjID objectIDbg Object ID for the bar's background sprite.
-- @tparam number spriteIDbg SpriteID from the specified object for the bar's background.
-- @tparam Color colorbg Color of bar's background.
-- @tparam Vec2 posBG X,Y position of the bar's background in screen percent (0-100).
-- @tparam float rotBG rotation of the bar's background. sprite (0-360).
-- @tparam Vec2 scaleBG X,Y Scaling factor for the bar's background sprite.
-- @tparam View.AlignMode alignModebg Alignment for the bar's background.
-- @tparam View.ScaleMode scaleModebg Scaling for the bar's background.
-- @tparam Effects.BlendID blendModebg Blending modes for the bar's background.
-- @tparam Objects.ObjID objectIDbar Object ID for the bar sprite.
-- @tparam number spriteIDbar SpriteID from the specified object for the bar.
-- @tparam Color colorbar Color of the bar.
-- @tparam Vec2 posBar X,Y position of the bar in screen percent (0-100).
-- @tparam float rot rotation of the bar's sprite (0-360).
-- @tparam Vec2 scaleBar X,Y Scaling factor for the bar's sprite.
-- @tparam View.AlignMode alignMode Alignment for the bar.
-- @tparam View.ScaleMode scaleMode Scaling for the bar.
-- @tparam Effects.BlendID blendMode Blending modes for the bar.
-- @tparam string text Text to display on the bar.
-- @tparam Vec2 textPos X,Y position of the text.
-- @tparam Strings.DisplayStringOption textOptions alignment and effects for the text. Default: None. Please note text is automatically aligned to the LEFT
-- @tparam number textScale Scale factor for the text.
-- @tparam Color textColor Color of the text.
-- @tparam bool hideText Whether to hide the text.
-- @tparam number alphaBlendSpeed Speed of alpha blending for bar visibility (0-255).
-- @tparam bool blink Whether the bar blinks.
-- @tparam number blinkLimit % Limit below which bar starts blinking (0-1).
--
-- @treturn CustomBar The custombar in its hidden state
--
CustomBar.Create = function (barName, startvalue, maxvalue,
							objectIDbg, spriteIDbg, colorbg, posBG, rotBG, scaleBG, alignModebg, scaleModebg, blendModebg, 
							objectIDbar, spriteIDbar, colorbar, posBar, rot, scaleBar, alignMode, scaleMode, blendMode,
							text, textPos, textOptions, textScale, textColor, hideText, alphaBlendSpeed, blink, blinkLimit)

	local dataName	= barName .. "_bar_data"
	local self = {name = dataName}

	if LevelVars.Engine.CustomBars.bars[dataName] then
        print("Warning: a customBar with name " .. dataName .. " already exists; overwriting it with a new one...")
	end

	LevelVars.Engine.CustomBars.bars[dataName]			        = {}
	LevelVars.Engine.CustomBars.bars[dataName].name				= dataName
	LevelVars.Engine.CustomBars.bars[dataName].fixedInterval	= 1/30
	LevelVars.Engine.CustomBars.bars[dataName].progress			= startvalue / maxvalue -- Set initial progress from start value
	LevelVars.Engine.CustomBars.bars[dataName].objectIDbg		= objectIDbg
	LevelVars.Engine.CustomBars.bars[dataName].spriteIDbg		= spriteIDbg
	LevelVars.Engine.CustomBars.bars[dataName].colorBG			= colorbg
	LevelVars.Engine.CustomBars.bars[dataName].posBG			= posBG
	LevelVars.Engine.CustomBars.bars[dataName].scaleBG			= scaleBG
	LevelVars.Engine.CustomBars.bars[dataName].rotBG			= rotBG
	LevelVars.Engine.CustomBars.bars[dataName].alignModeBG	    = alignModebg
	LevelVars.Engine.CustomBars.bars[dataName].scaleModeBG	    = scaleModebg
	LevelVars.Engine.CustomBars.bars[dataName].blendModeBG	    = blendModebg
	LevelVars.Engine.CustomBars.bars[dataName].objectIDbar		= objectIDbar
	LevelVars.Engine.CustomBars.bars[dataName].spriteIDbar		= spriteIDbar
	LevelVars.Engine.CustomBars.bars[dataName].colorBar			= colorbar
	LevelVars.Engine.CustomBars.bars[dataName].posBar			= posBar
	LevelVars.Engine.CustomBars.bars[dataName].scaleBar			= scaleBar
	LevelVars.Engine.CustomBars.bars[dataName].rot				= rot
	LevelVars.Engine.CustomBars.bars[dataName].alignMode		= alignMode
	LevelVars.Engine.CustomBars.bars[dataName].scaleMode		= scaleMode
	LevelVars.Engine.CustomBars.bars[dataName].blendMode		= blendMode
	LevelVars.Engine.CustomBars.bars[dataName].oldValue			= startvalue  -- stores the current bar value
	LevelVars.Engine.CustomBars.bars[dataName].targetValue		= startvalue  -- target value to reach
	LevelVars.Engine.CustomBars.bars[dataName].maxValue			= maxvalue
	LevelVars.Engine.CustomBars.bars[dataName].text				= text
	LevelVars.Engine.CustomBars.bars[dataName].textPos			= textPos
	LevelVars.Engine.CustomBars.bars[dataName].textOptions		= textOptions
	LevelVars.Engine.CustomBars.bars[dataName].textScale		= textScale
	LevelVars.Engine.CustomBars.bars[dataName].textColor		= textColor
	LevelVars.Engine.CustomBars.bars[dataName].hideText			= hideText  -- required to hide bar text
	LevelVars.Engine.CustomBars.bars[dataName].visible			= false
	LevelVars.Engine.CustomBars.bars[dataName].currentAlpha		= 0
	LevelVars.Engine.CustomBars.bars[dataName].targetAlpha		= 0
	LevelVars.Engine.CustomBars.bars[dataName].alphaBlendSpeed	= alphaBlendSpeed
	LevelVars.Engine.CustomBars.bars[dataName].blink			= blink
	LevelVars.Engine.CustomBars.bars[dataName].blinkLimit		= blinkLimit
	LevelVars.Engine.CustomBars.bars[dataName].blinkSpeed		= 8
	LevelVars.Engine.CustomBars.bars[dataName].showBar			= nil --required to hide bar when enemy is not targeted
	LevelVars.Engine.CustomBars.bars[dataName].object			= nil
	LevelVars.Engine.CustomBars.bars[dataName].getActionType	= nil
	LevelVars.Engine.CustomBars.bars[dataName].currentTimer		= 0
    return setmetatable(self, CustomBar)
end

---
-- Creates a bar tied to Lara's attributes (Health, Air, Stamina).
-- @tparam Objects.ObjID objectIDbg Object ID for the bar's background sprite.
-- @tparam number spriteIDbg SpriteID from the specified object for the bar's background.
-- @tparam Color colorbg Color of bar's background.
-- @tparam Vec2 posBG X,Y position of the bar's background in screen percent (0-100).
-- @tparam number rotBG rotation of the bar's background. sprite (0-360).
-- @tparam Vec2 scaleBG X,Y Scaling factor for the bar's background sprite.
-- @tparam View.AlignMode alignModebg Alignment for the bar's background.
-- @tparam View.ScaleMode scaleModebg Scaling for the bar's background.
-- @tparam Effects.BlendID blendModebg Blending modes for the bar's background.
-- @tparam Objects.ObjID objectIDbar Object ID for the bar sprite.
-- @tparam number spriteIDbar SpriteID from the specified object for the bar.
-- @tparam Color colorbar Color of the bar.
-- @tparam Vec2 posBar X,Y position of the bar in screen percent (0-100).
-- @tparam number rot rotation of the bar's sprite (0-360).
-- @tparam Vec2 scaleBar X,Y Scaling factor for the bar's sprite.
-- @tparam View.AlignMode alignMode Alignment for the bar.
-- @tparam View.ScaleMode scaleMode Scaling for the bar.
-- @tparam Effects.BlendID blendMode Blending modes for the bar.
-- @tparam number alphaBlendSpeed Speed of alpha blending for bar visibility (0-255).
-- @tparam number getActionType Determines the bar type: 1: Health, 2: Air, 3: Stamina.
-- @tparam bool showBar Option to always show the bar. If set to false, the bars will automatically hide when they stop updating.
-- @tparam bool blink Whether the bar blinks.
-- @tparam number blinkLimit % Limit below which bar starts blinking (0-1).
--
-- @treturn CustomBar The custombar in its hidden state
CustomBar.CreateLaraBar = function (objectIDbg, spriteIDbg, colorbg, posBG, rotBG, scaleBG, alignModebg, scaleModebg, blendModebg,
							objectIDbar, spriteIDbar, colorbar, posBar, rot, scaleBar, alignMode, scaleMode, blendMode,
							alphaBlendSpeed, getActionType, showBar, blink, blinkLimit)

	local barName = "Lara" .. getActionType
	local dataName	= barName .. "_bar_data"

	if getActionType >= 1 and getActionType <= 3 then
		local startValue = getActionType == 1 and Lara:GetHP() or (getActionType == 2 and Lara:GetAir() or (getActionType == 3 and Lara:GetStamina()))
		local maxValue = getActionType == 1 and 1000 or (getActionType == 2 and 1800 or (getActionType == 3 and 120))
		
		CustomBar.Create(barName, startValue, maxValue,
						objectIDbg, spriteIDbg, colorbg, posBG, rotBG, scaleBG, alignModebg, scaleModebg, blendModebg, 
						objectIDbar, spriteIDbar, colorbar, posBar, rot, scaleBar, alignMode, scaleMode, blendMode,
						"BLANK", TEN.Vec2(0,0), {}, 0, TEN.Color(0,0,0), true, alphaBlendSpeed, blink, blinkLimit)
		
		-- LevelVars.Engine.CustomBars.bars[dataName].oldValue			= maxValue
	end

	
	LevelVars.Engine.CustomBars.bars[dataName].getActionType	= getActionType
	LevelVars.Engine.CustomBars.bars[dataName].showBar			= showBar
	LevelVars.Engine.CustomBars.bars[dataName].visible			= true
	LevelVars.Engine.CustomBars.bars[dataName].targetAlpha		= 255

	
end

---
-- Creates a custom progress bar for an enemy with extensive configuration options. Ensure this function is called before Lara aims at the enemy if using generic enemy HP bars.
-- @tparam string barName Unique name for the bar.
-- @tparam Objects.ObjID objectIDbg Object ID for the bar's background sprite.
-- @tparam number spriteIDbg SpriteID from the specified object for the bar's background.
-- @tparam Color colorbg Color of bar's background.
-- @tparam Vec2 posBG X,Y position of the bar's background in screen percent (0-100).
-- @tparam number rotBG rotation of the bar's background. sprite (0-360).
-- @tparam Vec2 scaleBG X,Y Scaling factor for the bar's background sprite.
-- @tparam View.AlignMode alignModebg Alignment for the bar's background.
-- @tparam View.ScaleMode scaleModebg Scaling for the bar's background.
-- @tparam Effects.BlendID blendModebg Blending modes for the bar's background.
-- @tparam Objects.ObjID objectIDbar Object ID for the bar sprite.
-- @tparam number spriteIDbar SpriteID from the specified object for the bar.
-- @tparam Color colorbar Color of the bar.
-- @tparam Vec2 posBar X,Y position of the bar in screen percent (0-100).
-- @tparam number rot rotation of the bar's sprite (0-360).
-- @tparam Vec2 scaleBar X,Y Scaling factor for the bar's sprite.
-- @tparam View.AlignMode alignMode Alignment for the bar.
-- @tparam View.ScaleMode scaleMode Scaling for the bar.
-- @tparam Effects.BlendID blendMode Blending modes for the bar.
-- @tparam string text Text to display for the enemy.
-- @tparam Vec2 textPos X,Y position of the text.
-- @tparam Strings.DisplayStringOption textOptions alignment and effects for the text. Default: None. Please note text is automatically aligned to the LEFT
-- @tparam number textScale Scale factor for the text.
-- @tparam Color textColor Color of the text.
-- @tparam bool hideText Whether to hide the text.
-- @tparam number alphaBlendSpeed Speed of alpha blending for bar visibility (0-255).
-- @tparam string object Enemy name set in Editor for which to create HP for.
-- @tparam bool showBar Option to always show the bar whether the enemy is current target or not. Useful for boss health bars.
-- @tparam bool blink Whether the bar blinks.
-- @tparam number blinkLimit %Limit below which bar starts blinking (0-1).
--
-- @treturn CustomBar The custombar in its hidden state
CustomBar.CreateEnemyHpBar = function (barName,
							objectIDbg, spriteIDbg, colorbg, posBG, rotBG, scaleBG, alignModebg, scaleModebg, blendModebg, 
							objectIDbar, spriteIDbar, colorbar, posBar, rot, scaleBar, alignMode, scaleMode, blendMode,
							text, textPos, textOptions, textScale, textColor, hideText, alphaBlendSpeed, object, showBar, blink, blinkLimit)

    local dataName	= barName .. "_bar_data"
	local enemyHP = TEN.Objects.GetMoveableByName(object):GetHP()


	CustomBar.Create(barName, enemyHP, enemyHP,
							objectIDbg, spriteIDbg, colorbg, posBG, rotBG, scaleBG, alignModebg, scaleModebg, blendModebg, 
							objectIDbar, spriteIDbar, colorbar, posBar, rot, scaleBar, alignMode, scaleMode, blendMode,
							text, textPos, textOptions, textScale, textColor, hideText, alphaBlendSpeed, blink, blinkLimit)
	
	LevelVars.Engine.CustomBars.bars[dataName].showBar			= showBar
	LevelVars.Engine.CustomBars.bars[dataName].object			= object
	LevelVars.Engine.CustomBars.bars[dataName].getActionType	= 0
	LevelVars.Engine.CustomBars.bars[dataName].visible			= true
	LevelVars.Engine.CustomBars.bars[dataName].fixedInterval	= 1/3
	LevelVars.Engine.CustomBars.bars[dataName].currentAlpha		= 0
	LevelVars.Engine.CustomBars.bars[dataName].targetAlpha		= 255
	
end

---
-- Generates health bars for all enemies. A new bar appears whenever Lara aims at an enemy. If the "hide text" option is disabled, the enemy's name (as set in the editor) is displayed. Multiple enemies can share the same name by appending _number to the name in the editor. If adjusting an enemy's max HP, ensure this is done before Lara targets the enemy. To create health bars for specific enemies, use CustomBar.CreateEnemyHpBar, ensuring the bar is created prior to targeting.
-- @tparam Objects.ObjID objectIDbg Object ID for the bar's background sprite.
-- @tparam number spriteIDbg SpriteID from the specified object for the bar's background.
-- @tparam Color colorbg Color of bar's background.
-- @tparam Vec2 posBG X,Y position of the bar's background in screen percent (0-100).
-- @tparam number rotBG rotation of the bar's background. sprite (0-360).
-- @tparam Vec2 scaleBG X,Y Scaling factor for the bar's background sprite.
-- @tparam View.AlignMode alignModebg Alignment for the bar's background.
-- @tparam View.ScaleMode scaleModebg Scaling for the bar's background.
-- @tparam Effects.BlendID blendModebg Blending modes for the bar's background.
-- @tparam Objects.ObjID objectIDbar Object ID for the bar sprite.
-- @tparam number spriteIDbar SpriteID from the specified object for the bar.
-- @tparam Color colorbar Color of the bar.
-- @tparam Vec2 posBar X,Y position of the bar in screen percent (0-100).
-- @tparam number rot rotation of the bar's sprite (0-360).
-- @tparam Vec2 scaleBar X,Y Scaling factor for the bar's sprite.
-- @tparam View.AlignMode alignMode Alignment for the bar.
-- @tparam View.ScaleMode scaleMode Scaling for the bar.
-- @tparam Effects.BlendID blendMode Blending modes for the bar.
-- @tparam number textPos X position of the text.
-- @tparam Strings.DisplayStringOption textOptions alignment and effects for the text. Default: None. Please note text is automatically aligned to the LEFT
-- @tparam number textScale Scale factor for the text.
-- @tparam Color textColor Color of the text.
-- @tparam bool hideText Whether to hide the enemy name text.
-- @tparam number alphaBlendSpeed Speed of alpha blending for bar visibility (0-255).
-- @tparam bool blink Whether the bar blinks.
-- @tparam number blinkLimit %Limit below which bar starts blinking (0-1).
--
CustomBar.SetEnemiesHpGenericBar = function (objectIDbg, spriteIDbg, colorbg, posBG, rotBG, scaleBG, alignModebg, scaleModebg, blendModebg, 
							objectIDbar, spriteIDbar, colorbar, posBar, rot, scaleBar, alignMode, scaleMode, blendMode,
							textPos, textOptions, textScale, textColor, hideText, alphaBlendSpeed, blink, blinkLimit)
	
	if LevelVars.Engine.CustomBars.EnemiesHpBar.objectIDbg then
        print("Warning: Overwriting enemy HP bar definitions")
	end

	LevelVars.Engine.CustomBars.EnemiesHpBar.objectIDbg			= objectIDbg
	LevelVars.Engine.CustomBars.EnemiesHpBar.spriteIDbg			= spriteIDbg
	LevelVars.Engine.CustomBars.EnemiesHpBar.colorBG			= colorbg
	LevelVars.Engine.CustomBars.EnemiesHpBar.posBG				= posBG
	LevelVars.Engine.CustomBars.EnemiesHpBar.scaleBG			= scaleBG
	LevelVars.Engine.CustomBars.EnemiesHpBar.rotBG				= rotBG
	LevelVars.Engine.CustomBars.EnemiesHpBar.alignModeBG	    = alignModebg
	LevelVars.Engine.CustomBars.EnemiesHpBar.scaleModeBG	    = scaleModebg
	LevelVars.Engine.CustomBars.EnemiesHpBar.blendModeBG	    = blendModebg
	LevelVars.Engine.CustomBars.EnemiesHpBar.objectIDbar		= objectIDbar
	LevelVars.Engine.CustomBars.EnemiesHpBar.spriteIDbar		= spriteIDbar
	LevelVars.Engine.CustomBars.EnemiesHpBar.colorBar			= colorbar
	LevelVars.Engine.CustomBars.EnemiesHpBar.posBar				= posBar
	LevelVars.Engine.CustomBars.EnemiesHpBar.scaleBar			= scaleBar
	LevelVars.Engine.CustomBars.EnemiesHpBar.rot				= rot
	LevelVars.Engine.CustomBars.EnemiesHpBar.alignMode			= alignMode
	LevelVars.Engine.CustomBars.EnemiesHpBar.scaleMode			= scaleMode
	LevelVars.Engine.CustomBars.EnemiesHpBar.blendMode			= blendMode
	LevelVars.Engine.CustomBars.EnemiesHpBar.textPos			= textPos
	LevelVars.Engine.CustomBars.EnemiesHpBar.textOptions		= textOptions
	LevelVars.Engine.CustomBars.EnemiesHpBar.textScale			= textScale
	LevelVars.Engine.CustomBars.EnemiesHpBar.textColor			= textColor
	LevelVars.Engine.CustomBars.EnemiesHpBar.hideText			= hideText  -- required to hide bar text
	LevelVars.Engine.CustomBars.EnemiesHpBar.alphaBlendSpeed	= alphaBlendSpeed
	LevelVars.Engine.CustomBars.EnemiesHpBar.blink				= blink
	LevelVars.Engine.CustomBars.EnemiesHpBar.blinkLimit			= blinkLimit


	LevelVars.Engine.CustomBars.EnemiesHpBar.status = true
end

-- The function retrieves an existing bar instance by its unique identifier (barName). This function is useful when you need to access or manipulate a bar that has already been created.
-- 	@string barName: The unique identifier assigned to the bar when it was created using CustomBar.New
CustomBar.Get = function(barName)
	local dataName = barName .. "_bar_data"
    if LevelVars.Engine.CustomBars.bars[dataName] then
        local self = {name = dataName}
        return setmetatable(self, CustomBar)
    end
end

-- The function removes a custom bar and its associated data from the system. It ensures that the bar is no longer tracked or accessible in the LevelVars.Engine.CustomBars.bars table.
-- 	@string barName: The name of the custom bar to be deleted.
CustomBar.Delete = function (barName)
    local dataName = barName .. "_bar_data"
	if LevelVars.Engine.CustomBars.bars[dataName] then
		LevelVars.Engine.CustomBars.bars[dataName] = nil
	end
end

-- The function sets the value of a custom bar over a specified time period.
-- @number value: The new target to which the bar's current value should transition. (Must be a non-negative number; between 0 and the bar's maxValue.
-- @number time: The time (in seconds) over which the bar's value should transition to the target value.
function CustomBar:SetBarValue(value, time)
	if LevelVars.Engine.CustomBars.bars[self.name] then
		if type(value) =="number" and value >= 0 then
			local currentValue = LevelVars.Engine.CustomBars.bars[self.name].oldValue
			local maxValue = LevelVars.Engine.CustomBars.bars[self.name].maxValue
			local newTargetValue = math.max(0, math.min(maxValue, value))
			LevelVars.Engine.CustomBars.bars[self.name].targetValue = newTargetValue
			LevelVars.Engine.CustomBars.bars[self.name].fixedInterval = (newTargetValue - currentValue) / (time * 30)
		end
	end
end

-- The function adjusts the bar's value relative to its current or target value over a specified time span.
-- @number value: The relative value to add (positive or negative) to the current bar value.
-- @number time: The duration (in seconds) over which the change should occur.
function CustomBar:ChangeBarValueOverTimespan(value, time)
    -- Check if bar data and timer exist
	if LevelVars.Engine.CustomBars.bars[self.name] then

		-- Get the current target value or old value if no target value exists
		local currentValue = LevelVars.Engine.CustomBars.bars[self.name].oldValue
		local maxValue = LevelVars.Engine.CustomBars.bars[self.name].maxValue
		local currentTarget = LevelVars.Engine.CustomBars.bars[self.name].targetValue or currentValue

		-- Calculate new target value by adding the relative 'value' and clamp between 0 and 1000
		local newTargetValue = math.max(0, math.min(maxValue, currentTarget + value))

		-- Set the new target value
		LevelVars.Engine.CustomBars.bars[self.name].targetValue = newTargetValue

		-- Calculate total frames based on time and FPS (30 FPS)
		local totalFrames = time * 30

		-- Calculate the fixed interval for the entire transition
		LevelVars.Engine.CustomBars.bars[self.name].fixedInterval = (newTargetValue - currentValue) / totalFrames
	end
end

-- The function controls the visibility of a custom bar.
-- 	@bool visible: true: Makes the bar visible.; false: Hides the bar.
function CustomBar:SetVisibility(visible)
    --the visible variable is a boolean
	if LevelVars.Engine.CustomBars.bars[self.name] then
		if visible and type(visible) == "boolean" then
			LevelVars.Engine.CustomBars.bars[self.name].targetAlpha = 255
			LevelVars.Engine.CustomBars.bars[self.name].visible = true
		else
			LevelVars.Engine.CustomBars.bars[self.name].targetAlpha = 0
		end
	end
end

--- The function checks whether a custom bar is currently visible.
-- @treturn bool true if the bar is visible and false if it is not.
function CustomBar:IsVisible()
    
	if LevelVars.Engine.CustomBars.bars[self.name] then
		if LevelVars.Engine.CustomBars.bars[self.name].visible then
			return true
		else
			return false
		end
	end
end

-- The function retrieves the current value of a custom bar.
-- @treturn float returns the current value of a custom bar.
function CustomBar:GetValue()
	
    if LevelVars.Engine.CustomBars.bars[self.name] then
        return LevelVars.Engine.CustomBars.bars[self.name].oldValue
	end
end

-- The function deletes all custom bars.
CustomBar.DeleteAllBars = function ()
	for _, customBar in pairs (LevelVars.Engine.CustomBars.bars) do
		LevelVars.Engine.CustomBars.bars[customBar.name] = nil
	end
end

-- This function prevents the creation of new health bars for enemies when set to false. However, it does not affect the health bars that have already been created.
-- @bool value Specifies whether new health bars for enemies should be created. 
CustomBar.ShowEnemiesHpGenericBar = function(value)
	if type(value) == "boolean" then
		LevelVars.Engine.CustomBars.EnemiesHpBar.status = value
	end
end

-- The function deletes all the enemy health bars excluding those created by CustomBar.CreateEnemyHpBar.
CustomBar.DeleteExistingHpGenericBars = function ()
	for _, customBar in pairs (LevelVars.Engine.CustomBars.bars) do
		if customBar.getActionType == 4 then
			LevelVars.Engine.CustomBars.bars[customBar.name] = nil
		end
	end
end

---Set background properties
---
-- Sets the custom bar background sprite position.
-- @tparam Vec2 posBG X,Y position of the bar's background in screen percent (0-100).
--
function CustomBar:SetBackgroundPosition(pos)
	if pos and LevelVars.Engine.CustomBars.bars[self.name] then
		LevelVars.Engine.CustomBars.bars[self.name].posBG = pos
	end
end

---
-- Sets the custom bar background sprite rotation.
-- @tparam number rotBG rotation of the bar's background. sprite (0-360).
--
function CustomBar:SetBackgroundRotation(rot)
	if rot and  LevelVars.Engine.CustomBars.bars[self.name] then
		LevelVars.Engine.CustomBars.bars[self.name].rotBG = rot
	end
end

---
-- Sets the custom bar background sprite color.
-- @tparam Objects.ObjID objectIDbg Object ID for the bar's background sprite.
-- @tparam number spriteIDbg SpriteID from the specified object for the bar's background.
-- @tparam Color colorbg Color of bar's background.
--
function CustomBar:SetBackgroundColor(color)
	if color and LevelVars.Engine.CustomBars.bars[self.name] then
		LevelVars.Engine.CustomBars.bars[self.name].colorBG = color
	end
end

---
-- Sets the custom bar background sprite scale.
-- @tparam Vec2 scaleBG X,Y Scaling factor for the bar's background sprite.
--
function CustomBar:SetBackgroundScale(scale)
	if scale and LevelVars.Engine.CustomBars.bars[self.name] then
		LevelVars.Engine.CustomBars.bars[self.name].scaleBG = scale
	end
end

---
-- Sets the custom bar background sprite slot and sprite ID.
-- @tparam Objects.ObjID objectIDbg Object ID for the bar's background sprite.
-- @tparam number spriteIDbg SpriteID from the specified object for the bar's background.
--
function CustomBar:SetBackgroundSpriteSlot(slot, id)
	if slot and id and LevelVars.Engine.CustomBars.bars[self.name] then
		LevelVars.Engine.CustomBars.bars[self.name].objectIDbar = slot
		LevelVars.Engine.CustomBars.bars[self.name].spriteIDbar = id
	end
end

---
-- Sets the custom bar background sprite align mode.
-- @tparam View.AlignMode alignModebg Alignment for the bar's background.
--
function CustomBar:SetBackgroudAlignMode(alignMode)
	if alignMode and LevelVars.Engine.CustomBars.bars[self.name] then
		LevelVars.Engine.CustomBars.bars[self.name].alignMode = alignMode
	end
end
---
-- Sets the custom bar background sprite scale mode.
-- @tparam View.ScaleMode scaleModebg Scaling for the bar's background.
--
function CustomBar:SetBackgroudScaleMode(scaleMode)
	if scaleMode and LevelVars.Engine.CustomBars.bars[self.name] then
		LevelVars.Engine.CustomBars.bars[self.name].scaleMode = scaleMode
	end
end
---
-- Sets the custom bar background sprite blend mode.
-- @tparam Effects.BlendID blendModebg Blending modes for the bar's background.
--
function CustomBar:SetBackgroudBlendMode(blendMode)
	if blendMode and LevelVars.Engine.CustomBars.bars[self.name] then
		LevelVars.Engine.CustomBars.bars[self.name].blendModeBG = blendMode
	end
end

---Set bar properties
---
-- Sets the custom bar sprite position.
-- @tparam Vec2 posBar X,Y position of the bar in screen percent (0-100).
--
function CustomBar:SetBarPosition(pos)
	if pos and LevelVars.Engine.CustomBars.bars[self.name] then
		LevelVars.Engine.CustomBars.bars[self.name].posBG = pos
	end
end
---
-- Sets the custom bar sprite rotation.
-- @tparam number rot rotation of the bar's sprite (0-360).
--
function CustomBar:SetBarRotation(rot)
	if rot and  LevelVars.Engine.CustomBars.bars[self.name] then
		LevelVars.Engine.CustomBars.bars[self.name].rot = rot
	end
end
---
-- Sets the custom bar sprite color.
-- @tparam Color colorbar Color of the bar.
--
function CustomBar:SetBarColor(color)
	if color and LevelVars.Engine.CustomBars.bars[self.name] then
		LevelVars.Engine.CustomBars.bars[self.name].colorBar = color
	end
end
---
-- Sets the custom bar sprite scale.
-- @tparam Vec2 scaleBar X,Y Scaling factor for the bar's sprite.
--
function CustomBar:SetBarScale(scale)
	if scale and LevelVars.Engine.CustomBars.bars[self.name] then
		LevelVars.Engine.CustomBars.bars[self.name].scaleBar = scale
	end
end
---
-- Sets the custom bar sprite slot and sprite ID.
-- @tparam Objects.ObjID objectIDbar Object ID for the bar sprite.
-- @tparam number spriteIDbar SpriteID from the specified object for the bar.
--
function CustomBar:SetBarSpriteSlot(slot, id)
	if slot and id and LevelVars.Engine.CustomBars.bars[self.name] then
		LevelVars.Engine.CustomBars.bars[self.name].objectIDbg = slot
		LevelVars.Engine.CustomBars.bars[self.name].spriteIDbg = id
	end
end
---
-- Sets the custom bar sprite alignment mode.
-- @tparam View.AlignMode alignMode Alignment for the bar.
--
function CustomBar:SetBarAlignMode(alignMode)
	if alignMode and LevelVars.Engine.CustomBars.bars[self.name] then
		LevelVars.Engine.CustomBars.bars[self.name].alignModeBG = alignMode
	end
end
---
-- Sets the custom bar sprite scale mode.
-- @tparam View.ScaleMode scaleMode Scaling for the bar.
--
function CustomBar:SetBarScaleMode(scaleMode)
	if scaleMode and LevelVars.Engine.CustomBars.bars[self.name] then
		LevelVars.Engine.CustomBars.bars[self.name].scaleModeBG = scaleMode
	end
end
---
-- Sets the custom bar sprite blend mode.
-- @tparam Effects.BlendID blendMode Blending modes for the bar.
--
function CustomBar:SetBarBlendMode(blendMode)
	if blendMode and LevelVars.Engine.CustomBars.bars[self.name] then
		LevelVars.Engine.CustomBars.bars[self.name].blendMode = blendMode
	end
end

LevelFuncs.Engine.CustomBar.UpdateCustomBars = function()
	
	local playerTarget = Lara:GetTarget()
	
	if playerTarget ~= nil and LevelVars.Engine.CustomBars.EnemiesHpBar.status then
		local playerTargetName = playerTarget:GetName()
		local displayName = LevelFuncs.Engine.Node.SplitString(playerTargetName, "_")
		local enemytable = playerTargetName .. "_bar_data"
		if LevelVars.Engine.CustomBars.bars[enemytable] == nil then
			local eB = LevelVars.Engine.CustomBars.EnemiesHpBar
			CustomBar.CreateEnemyHpBar(playerTargetName,
			eB.objectIDbg, eB.spriteIDbg, eB.colorBG, eB.posBG, eB.rotBG, eB.scaleBG, eB.alignModeBG, eB.scaleModeBG, eB.blendModeBG,
			eB.objectIDbar, eB.spriteIDbar, eB.colorBar, eB.posBar, eB.rot, eB.scaleBar, eB.alignMode, eB.scaleMode, eB.blendMode,
			displayName[1], eB.textPos, eB.textOptions, eB.textScale, eB.textColor, eB.hideText,
			eB.alphaBlendSpeed, playerTargetName, false, eB.blink, eB.blinkLimit)
			LevelVars.Engine.CustomBars.bars[enemytable].getActionType = 4
		end
	end
	
	for _, customBar in pairs (LevelVars.Engine.CustomBars.bars) do

		if customBar ~= nil then
			-- Smoothly transition to target value
			local currentValue = customBar.oldValue or 0
			local targetValue = customBar.targetValue or 0
			local delta = customBar.fixedInterval

			if customBar.object ~=nil and (customBar.getActionType == 0 or customBar.getActionType == 4) then
				local enemy = GetMoveableByName(customBar.object)
				currentValue = enemy:GetHP()

				targetValue = currentValue

				customBar.progress = math.max(0, math.min(currentValue / customBar.maxValue, 1))

				if customBar.showBar == true then
					-- If showBar is true, the bar is always visible at full alpha
					customBar.targetAlpha = 255
					customBar.visible = true
				else
					-- If showBar is false, only show the bar if the enemy is the player's current target
					if playerTarget == enemy then
						customBar.targetAlpha = 255  -- Set to full alpha if this enemy is the target
						customBar.visible = true
					else
						customBar.targetAlpha = 0  -- Set to 0 alpha if this enemy is not the target
					end
				end

				if currentValue <= 0 then
					customBar.targetAlpha = 0
				end
				
				-- When Alpha reaches 0 set visibility to false
				if currentValue <= 0 and customBar.currentAlpha == 0 then
					customBar.visible = false
					LevelVars.Engine.CustomBars.bars[customBar.name] = nil
				end
			end

			if customBar.getActionType == 1 then
								
				currentValue = Lara:GetHP()
				targetValue = currentValue
				
				customBar.progress = math.max(0, math.min(currentValue / customBar.maxValue, 1))

				-- Check if `hideBar` is true, which overrides all other behaviors
				if customBar.showBar == true then
					customBar.targetAlpha = 255  -- Bar is always visible
					customBar.visible = true

				elseif currentValue ~= (customBar.oldValue or currentValue) then
					customBar.targetAlpha = 255  -- Show the bar if value changes
					customBar.visible = true

					customBar.currentTimer = customBar.currentTimer + 1

						if customBar.currentTimer >= 90 then
							customBar.oldValue = currentValue
							customBar.currentTimer = 0
						end
				
				elseif Lara:GetHandStatus() == 0 and currentValue >= customBar.blinkLimit*1000 then
						-- Hide bar if hands are free and HP is 200 or more
						customBar.targetAlpha = 0
						customBar.visible = false

				elseif Lara:GetHandStatus() == 2 or Lara:GetHandStatus() == 3 or Lara:GetHandStatus() == 4 then
						-- Show bar if hand status is 2, 3, or 4 (weapon drawn)
						customBar.targetAlpha = 255
						customBar.visible = true

				elseif Lara:GetHandStatus() == 0 and currentValue < customBar.blinkLimit*1000 then
						-- Show bar if hands are free and HP is less than 200
						customBar.targetAlpha = 255
						customBar.visible = true
				end

			elseif customBar.getActionType == 2 then
				currentValue = Lara:GetAir()
				targetValue = currentValue

				customBar.progress = math.max(0, math.min(currentValue / customBar.maxValue, 1))

				if customBar.showBar == true then
					-- If showBar is true, the bar is always visible
					customBar.targetAlpha = 255
					customBar.visible = true
				else
					-- If showBar is false, hide the bar when currentValue is at max
					if currentValue == customBar.maxValue then
						customBar.targetAlpha = 0  -- Hide the bar when at max value
					else
						customBar.targetAlpha = 255  -- Show the bar if currentValue is not max
						customBar.visible = true
					end
				end
				
			elseif customBar.getActionType == 3 then 
				currentValue = Lara:GetStamina()
				targetValue = currentValue

				customBar.progress = math.max(0, math.min(currentValue / customBar.maxValue, 1))
				
				if customBar.showBar == true then
					-- If showBar is true, the bar is always visible
					customBar.targetAlpha = 255
					customBar.visible = true
				else
					-- If showBar is false, hide the bar when currentValue is at max
					if currentValue == customBar.maxValue then
						customBar.targetAlpha = 0  -- Hide the bar when at max value
					else
						customBar.targetAlpha = 255  -- Show the bar if currentValue is not max
						customBar.visible = true
					end
				end

			end

			if currentValue ~= targetValue then
				-- Update current value by delta (increment or decrement)
				if currentValue < targetValue then
					currentValue = math.min(currentValue + delta, targetValue)
				else
					currentValue = math.max(currentValue + delta, targetValue)
				end

				-- Update the bar's progress (0-1 scale)
				customBar.oldValue = currentValue
				customBar.progress = currentValue / customBar.maxValue
			end
			-- Smoothly transition alpha
			if customBar.currentAlpha ~= customBar.targetAlpha then
				local alphaDelta = customBar.alphaBlendSpeed
				if customBar.currentAlpha < customBar.targetAlpha then
					customBar.currentAlpha = math.floor(math.min(customBar.currentAlpha + alphaDelta, customBar.targetAlpha))
				else
					customBar.currentAlpha = math.floor(math.max(customBar.currentAlpha - alphaDelta, customBar.targetAlpha))
				end
			end
			
			-- Set parameters to draw the background
			local posBG = customBar.posBG
			local scaleBG = customBar.scaleBG
			local rotBG = customBar.rotBG
			local alignMBG = LevelFuncs.Engine.Node.GetDisplaySpriteAlignMode(customBar.alignModeBG)
			local scaleMBG = LevelFuncs.Engine.Node.GetDisplaySpriteScaleMode(customBar.scaleModeBG)
			local blendIDBG = LevelFuncs.Engine.Node.GetBlendMode(customBar.blendModeBG)
			local normalAlpha = (customBar.currentAlpha/255)
			-- Adjust color with alpha blending
			local bgColor = Color(customBar.colorBG.r,customBar.colorBG.g,customBar.colorBG.b,customBar.currentAlpha)
			
			-- Set parameters to draw the bar
			local pos = customBar.posBar
			local rot = customBar.rot
			local alignM = LevelFuncs.Engine.Node.GetDisplaySpriteAlignMode(customBar.alignMode)
			local scaleM = LevelFuncs.Engine.Node.GetDisplaySpriteScaleMode(customBar.scaleMode)
			local blendID = LevelFuncs.Engine.Node.GetBlendMode(customBar.blendMode)
			local barColor = TEN.Color(customBar.colorBar.r,customBar.colorBar.g,customBar.colorBar.b,customBar.currentAlpha)
			
			-- when Alpha reaches 0 set visibility to false
			if customBar.currentAlpha > 0 then
				customBar.visible = true
			elseif customBar.currentAlpha == 0 then
				customBar.visible = false
			end
			
			--draw bar if alpha is greater than 1 and visibility is true
			if customBar.visible and customBar.currentAlpha > 0 then
				-- Draw background sprite
				local bgSprite = TEN.DisplaySprite(customBar.objectIDbg, customBar.spriteIDbg, posBG, rotBG, scaleBG, bgColor)
				bgSprite:Draw(0, alignMBG, scaleMBG, blendIDBG)

				-- Draw foreground sprite (the bar itself) proportional to Progress
				local barScale = TEN.Vec2(customBar.scaleBar.x * customBar.progress, customBar.scaleBar.y)
				local barSprite = TEN.DisplaySprite(customBar.objectIDbar, customBar.spriteIDbar, pos, rot, barScale, barColor)
				
				if customBar.frameCounter == nil then
					customBar.frameCounter = 0
				end
				
				-- Calculate HP percentage
				local Percentage = (currentValue / customBar.maxValue)
				
				-- Update frame counter
				customBar.frameCounter = customBar.frameCounter + 1
				
				-- Check if blink is enabled and value is below blinkLimit
				if customBar.blink == true and Percentage <= customBar.blinkLimit then
					-- Only draw the sprite every other frame
					if customBar.frameCounter % (customBar.blinkSpeed * 2) < customBar.blinkSpeed then
						barSprite:Draw(1, alignM, scaleM, blendID)
					end
				else
					-- Draw the sprite normally if blink is off or value is above blinkLimit
					barSprite:Draw(1, alignM, scaleM, blendID)
				end
				
				-- Reset the frame counter if it reaches the blinkSpeed limit to prevent overflow
				if customBar.frameCounter >= customBar.blinkSpeed * 2 then
					customBar.frameCounter = 0
				end
			
				if customBar.hideText == false then
					-- Draw text (enemy name and health)
					local barText = tostring(customBar.text) --debug text	 .. " (" .. currentHP .. " / " .. totalHP .. ")"
					local textColor = TEN.Color(customBar.textColor.r, customBar.textColor.g, customBar.textColor.b, customBar.currentAlpha)
					local posInPixel = TEN.Vec2(TEN.Util.PercentToScreen(customBar.textPos.x, customBar.textPos.y))
					local IsString = TEN.Flow.IsStringPresent(barText)
					local myText = TEN.Strings.DisplayString(barText, posInPixel, customBar.textScale, textColor, IsString, customBar.textOptions)
					TEN.Strings.ShowString(myText, 1/30)
				end
				-- Debugging code
				--local barValue = math.floor(customBar.oldValue)
				--local myTextString = "Bar Value: " .. barValue
				--local myText = DisplayString(myTextString, customBar.posX, customBar.posY-10, customBar.colorBar)
				--TEN.Strings.ShowString(myText,1/30)
			end
		end
	end
end

LevelFuncs.Engine.CustomBar.GenerateStringOption = function (alignment, effects)
	local options = {}
	if (effects == 1 or effects == 3) then table.insert(options, TEN.Strings.DisplayStringOption.SHADOW) end
	if (effects == 2 or effects == 3) then table.insert(options, TEN.Strings.DisplayStringOption.BLINK) end
	if (alignment == 1) then table.insert(options, TEN.Strings.DisplayStringOption.CENTER) end
	if (alignment == 2) then table.insert(options, TEN.Strings.DisplayStringOption.RIGHT) end
	return options
end

TEN.Logic.AddCallback(TEN.Logic.CallbackPoint.PRELOOP, LevelFuncs.Engine.CustomBar.UpdateCustomBars)

return CustomBar