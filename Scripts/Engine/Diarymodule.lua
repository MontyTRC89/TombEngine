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
local Diary = {}
Diary.__index = Diary

LevelFuncs.Engine.Diaries = {}
GameVars.Engine.Diaries = {}
LevelVars.LastUsedDiary=nil


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
Diary.Create = function(object, objectIDbg, spriteIDbg, colorbg, posX, posY, rot, scaleX, scaleY, alignMode, scaleMode, blendMode, pageSound, exitSound, notificationSound)

	local dataName = object .. "_diarydata"
    local self = {name = dataName}

	if GameVars.Engine.Diaries[dataName] then
		return
	end
	
	GameVars.Engine.Diaries[dataName]				        = {}
	GameVars.Engine.Diaries[dataName].Name			        = dataName
	GameVars.Engine.Diaries[dataName].currentPageIndex      = 1	
    GameVars.Engine.Diaries[dataName].UnlockedPages         = 1	
	GameVars.Engine.Diaries[dataName].Pages  		        = {TextEntries={},ImageEntries={}}
	GameVars.Engine.Diaries[dataName].Object		        = object
	GameVars.Engine.Diaries[dataName].ObjectIDbg	        = objectIDbg
	GameVars.Engine.Diaries[dataName].SpriteIDbg	        = spriteIDbg
	GameVars.Engine.Diaries[dataName].ColorBG		        = colorbg
	GameVars.Engine.Diaries[dataName].PosX			        = posX
	GameVars.Engine.Diaries[dataName].PosY			        = posY
	GameVars.Engine.Diaries[dataName].Rot			        = rot
	GameVars.Engine.Diaries[dataName].ScaleX		        = scaleX
	GameVars.Engine.Diaries[dataName].ScaleY		        = scaleY
	GameVars.Engine.Diaries[dataName].AlignMode		        = alignMode
	GameVars.Engine.Diaries[dataName].ScaleMode		        = scaleMode
	GameVars.Engine.Diaries[dataName].BlendMode		        = blendMode
	GameVars.Engine.Diaries[dataName].PageSound		        = pageSound
	GameVars.Engine.Diaries[dataName].ExitSound		        = exitSound
    GameVars.Engine.Diaries[dataName].NotificationSound     = notificationSound
	
	--print("Diary Constructed for diary: " .. dataName)
    return setmetatable(self, Diary)
end

-- The function retrieves an existing bar instance by its unique identifier (barName). This function is useful when you need to access or manipulate a bar that has already been created.
-- 	@string barName: The unique identifier assigned to the bar when it was created using CustomBar.New
Diary.Get = function(object)
	local dataName = object .. "_diarydata"
    if GameVars.Engine.Diaries[dataName] then
        local self = {name = dataName}
        return setmetatable(self, Diary)
    end
end

-- The function removes a custom bar and its associated data from the system. It ensures that the bar is no longer tracked or accessible in the LevelVars.Engine.CustomBars.bars table.
-- 	@string barName: The name of the custom bar to be deleted.
Diary.Delete = function (object)
    local dataName = object .. "_diarydata"
	if GameVars.Engine.Diaries[dataName] then
		GameVars.Engine.Diaries[dataName] = nil
	end
end

-- The function sets the value of a custom bar over a specified time period.
-- @number value: The new target to which the bar's current value should transition. (Must be a non-negative number; between 0 and the bar's maxValue.
-- @number time: The time (in seconds) over which the bar's value should transition to the target value.
function Diary:unlockPages(index)
    if GameVars.Engine.Diaries[self.name] then
		GameVars.Engine.Diaries[self.name].UnlockedPages = index
        PlaySound(GameVars.Engine.Diaries[self.name].NotificationSound)
        
        -- Show notification sprite
        local spriteID = diary.NotificationSpriteID
        local spritePos = TEN.Vec2(diary.PosX, diary.PosY)
        local spriteScale = TEN.Vec2(1, 1) -- Adjust scale as needed
        local spriteColor = Color(255, 255, 255) -- White by default

        local function drawSprite()
            local sprite = TEN.DisplaySprite(diary.ObjectIDbg, spriteID, spritePos, 0, spriteScale, spriteColor)
            sprite:Draw(1, AlignMode.Center, ScaleMode.None, BlendMode.Normal)
        end

        -- Set timer to remove sprite after a few seconds
        local duration = 3 -- Seconds to show the sprite
        local elapsedTime = 0

        local function update(dt)
            elapsedTime = elapsedTime + dt
            if elapsedTime <= duration then
                drawSprite()
            else
                -- Stop the timer after the sprite has been displayed
                TEN.Logic.RemoveCallback(TEN.Logic.CallbackPoint.PRELOOP, update)
            end
        end

        -- Register the update callback
        TEN.Logic.AddCallback(TEN.Logic.CallbackPoint.PRELOOP, update)
    end
end
    end
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
-- @tparam number alphaBlendSpeed Speed whether the enemy is current target or not. Useful for boss health bars.
-- @tparam bool blink Whether the bar blinks.
-- @tparam number blinkLimit %Limit below which bar starts blinking (0-1).of alpha blending for bar visibility (0-255).
-- @tparam string object Enemy name set in Editor for which to create HP for.
-- @tparam bool showBar Option to always show the bar 
--
-- @treturn CustomBar The custombar in its hidden state
function Diary:addTextEntry(pageIndex, text, textX, textY, textAlignment, textEffects, textScale, textColor)
    local textEntry = {
            text = text,
            textX = textX,
            textY = textY,
            textAlignment = textAlignment,
            textEffects = textEffects,
            textScale = textScale,
            textColor = textColor
        }
        table.insert(self.Pages[pageIndex].TextEntry, textEntry)
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
-- @tparam number alphaBlendSpeed Speed whether the enemy is current target or not. Useful for boss health bars.
-- @tparam bool blink Whether the bar blinks.
-- @tparam number blinkLimit %Limit below which bar starts blinking (0-1).of alpha blending for bar visibility (0-255).
-- @tparam string object Enemy name set in Editor for which to create HP for.
-- @tparam bool showBar Option to always show the bar 
--
-- @treturn CustomBar The custombar in its hidden state
function Diary:addImageEntry(pageIndex, objectIDbg, spriteIDbg, colorbg, posX, posY, rot, scaleX, scaleY, alignMode, scaleMode, blendMode)
   local imageEntry = {
            objectIDbg = objectIDbg,
            spriteIDbg = spriteIDbg,
            colorBG = colorbg,
            posX = posX,
            posY = posY,
            rot = rot,
            scaleX = scaleX,
            scaleY = scaleY,
            alignMode = alignMode,
            scaleMode = scaleMode,
            blendMode = blendMode
        }
        table.insert(self.Pages[pageIndex].ImageEntry, imageEntry)
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
-- @tparam number alphaBlendSpeed Speed whether the enemy is current target or not. Useful for boss health bars.
-- @tparam bool blink Whether the bar blinks.
-- @tparam number blinkLimit %Limit below which bar starts blinking (0-1).of alpha blending for bar visibility (0-255).
-- @tparam string object Enemy name set in Editor for which to create HP for.
-- @tparam bool showBar Option to always show the bar 
--
-- @treturn CustomBar The custombar in its hidden state
function Diary:importData(filename)

    local diaryData = require(filename)

    for pageIndex, page in ipairs(diaryData.Pages) do
        Diary:addImageEntry(pageIndex, page.Image.objectIDbg, page.Image.spriteIDbg, page.Image.colorBG, page.Image.posX, page.Image.posY, page.Image.rot, page.Image.scaleX, page.Image.scaleY, page.Image.alignMode, page.Image.scaleMode, page.Image.blendMode)
        for _, text in ipairs(page.Text) do
            Diary:addTextEntry(pageIndex, text.text, text.textX, text.textY, text.textAlignment, text.textEffects, text.textScale, text.textColor)
        end
    end
end

-- !Ignore
LevelFuncs.Engine.Diaries.ShowDiary = function()

    local objectNumber = LevelVars.LastUsedDiary
	local dataName = objectNumber .. "_diarydata"

    if GameVars.Engine.Diaries[dataName] then

        local currentIndex = GameVars.Engine.Diaries[dataName].currentPageIndex
        local maxPages = GameVars.Engine.Diaries[dataName].UnlockedPages

		if KeyIsHit(ActionID.LEFT) then
			currentIndex = math.max(1, currentIndex - 1)
			PlaySound(diary.PageSound)
		elseif KeyIsHit(ActionID.RIGHT) then
			currentIndex = math.min(maxPages, currentIndex + 1)
			PlaySound(diary.PageSound)
        elseif KeyIsHit(ActionID.INVENTORY) then
            PlaySound(diary.ExitSound)
            SetPostProcessMode(View.PostProcessMode.NONE)
            SetPostProcessStrength(1)
            SetPostProcessTint(Color(255,255,255))
            Flow.SetFreezeMode(Flow.FreezeMode.NONE)
            TEN.Logic.RemoveCallback(TEN.Logic.CallbackPoint.PREFREEZE, LevelFuncs.Engine.Node.ShowDiary)
        end

        --Sets the currentindex so that the diary opens at the same page
        GameVars.Diaries[dataName].currentPageIndex = currentIndex

        local diary = GameVars.Engine.Diaries[dataName]
        local textEntries = GameVars.Engine.Diaries[dataName].Pages[currentIndex].TextEntries
        local imageEntries = GameVars.Engine.Diaries[dataName].Pages[currentIndex].ImageEntries

        if textEntries or imageEntries then
            -- Draw background
            local bgPos = TEN.Vec2(diary.posX, diary.posY)
            local bgScale = TEN.Vec2(diary.scaleX, diary.scaleY)
            local bgSprite = TEN.DisplaySprite(diary.objectIDbg, diary.spriteIDbg, bgPos, diary.rot, bgScale, diary.colorBG)
            bgSprite:Draw(0, diary.alignMode, diary.scaleMode, diary.blendMode)
            
            -- Draw entries based on type
            --add check for empty table here
            for _, entry in ipairs(textEntries) do
                
                    local diaryText = LevelFuncs.Engine.Node.GenerateString(entry.text, entry.textX, entry.textY, entry.textScale, entry.textAlignment, entry.textEffects, entry.textColor)
                    ShowString(diaryText, 1 / 30)
            end
--add check for empty table here
            for _, entry in ipairs(imageEntries) do

                    local entryPos = TEN.Vec2(entry.posX, entry.posY)
                    local entryScale = TEN.Vec2(entry.scaleX, entry.scaleY)
                    local entrySprite = TEN.DisplaySprite(entry.objectIDbg, entry.spriteIDbg, entryPos, entry.rot, entryScale, entry.colorBG)
                    entrySprite:Draw(1, entry.alignMode, entry.scaleMode, entry.blendMode)
            end

        end
    end
end

-- !Ignore
LevelFuncs.Engine.Diaries.ActivateDiary = function(objectNumber)
	
    LevelVars.LastUsedDiary = objectNumber
	local dataName = objectNumber .. "_diarydata"
	if GameVars.Diaries[dataName] then
		TEN.Logic.AddCallback(TEN.Logic.CallbackPoint.PREFREEZE, LevelFuncs.Engine.Diaries.ShowDiary)
		Flow.SetFreezeMode(Flow.FreezeMode.Full)
	end
end

-- !Ignore
LevelFuncs.Engine.Diaries.DiaryStatus = function(value)

	if GameVars.Diaries then
		if value == true then
			TEN.Logic.AddCallback(TEN.Logic.CallbackPoint.PREUSEITEM, LevelFuncs.Engine.Diaries.ActivateDiary)
		elseif value == false then
			TEN.Logic.RemoveCallback(TEN.Logic.CallbackPoint.PREUSEITEM, LevelFuncs.Engine.Diaries.ActivateDiary)
		end
	end
end


-- local diaryEntries = { Pages = {} }

-- for i = 1, 30 do
--     table.insert(diaryEntries.Pages, {
--         Image = {
--             objectIDbg = i,
--             spriteIDbg = 100 + i,
--             colorBG = {255 - i, 255 - i, 255 - i}, -- Gradual darkening
--             posX = 100 + i,
--             posY = 100 + i,
--             rot = 0,
--             scaleX = 1,
--             scaleY = 1,
--             alignMode = "Center",
--             scaleMode = "None",
--             blendMode = "Normal"
--         },
--         Text = {
--             {
--                 text = "Page " .. i .. ": A journey begins.",
--                 textX = 150,
--                 textY = 120,
--                 textAlignment = "Left",
--                 textEffects = "Shadow",
--                 textScale = 1,
--                 textColor = {255, 255, 255}
--             },
--             {
--                 text = "Line 2 on Page " .. i,
--                 textX = 150,
--                 textY = 140,
--                 textAlignment = "Left",
--                 textEffects = "Shadow",
--                 textScale = 1,
--                 textColor = {255, 255, 255}
--             }
--         }
--     })
-- end


