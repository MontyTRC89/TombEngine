-----
--- Custom Bars - Draws custom bars on screen.
--The module provides functions to create and manage Diaries. It maintains state through LevelVars.Engine.CustomBars, which stores bar definitions and configurations.
-- Each bar is controlled by its specific functions. 
--
-- Example usage:
--	local Diary = require("Engine.CustomBar")
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


--Action list
--add function for showing controls/icon for narration if exists
--decouple nodes from code
--fix the diary page numbers function with chatgpt
--change default color everywhere in nodes
local CustomDiary = {}
CustomDiary.__index = CustomDiary

LevelFuncs.Engine.Diaries = {}
GameVars.Engine.Diaries = {}
GameVars.Engine.LastUsedDiary=nil


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
CustomDiary.Create = function(object, objectIDbg, spriteIDbg, colorbg, posX, posY, rot, scaleX, scaleY, alignMode, scaleMode, blendMode, pageSound, exitSound)

	local dataName = object .. "_diarydata"
    local self = {Name = dataName}
    GameVars.Engine.LastUsedDiary = object

	if GameVars.Engine.Diaries[dataName] then
		return setmetatable(self, CustomDiary)
	end
	
	GameVars.Engine.Diaries[dataName]				        = {}
	GameVars.Engine.Diaries[dataName].Name			        = dataName
	GameVars.Engine.Diaries[dataName].currentPageIndex      = 1
    GameVars.Engine.Diaries[dataName].UnlockedPages         = 1
	GameVars.Engine.Diaries[dataName].Pages  		        = {NarrationTrack=nil,TextEntries={},ImageEntries={}}
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
    GameVars.Engine.Diaries[dataName].PostProcessMode       = View.PostProcessMode.NONE
	GameVars.Engine.Diaries[dataName].PostProcessPower      = 1
    GameVars.Engine.Diaries[dataName].PostProcessColor      = TEN.Color(255,255,255)
    GameVars.Engine.Diaries[dataName].CurrentAlpha		    = 0
	GameVars.Engine.Diaries[dataName].TargetAlpha		    = 255
    GameVars.Engine.Diaries[dataName].EntryCurrentAlpha		= 0
	GameVars.Engine.Diaries[dataName].EntryTargetAlpha		= 255
    GameVars.Engine.Diaries[dataName].Visible               = false
    GameVars.Engine.Diaries[dataName].Notification          = {}
    GameVars.Engine.Diaries[dataName].PageNumbers           = {}
    GameVars.Engine.Diaries[dataName].AlphaBlendSpeed       = 100
    GameVars.Engine.Diaries[dataName].EntryFadingIn         = true

	
	print("CustomDiary Constructed for CustomDiary: " .. dataName)
    return setmetatable(self, CustomDiary)
end

-- The function retrieves an existing bar instance by its unique identifier (barName). This function is useful when you need to access or manipulate a bar that has already been created.
-- 	@string barName: The unique identifier assigned to the bar when it was created using CustomBar.New
CustomDiary.Get = function(object)
	local dataName = object .. "_diarydata"
    if GameVars.Engine.Diaries[dataName] then
        local self = {Name = dataName}
        return setmetatable(self, CustomDiary)
    end
end

-- The function removes a custom bar and its associated data from the system. It ensures that the bar is no longer tracked or accessible in the LevelVars.Engine.CustomBars.bars table.
-- 	@string barName: The name of the custom bar to be deleted.
CustomDiary.Delete = function (object)
    local dataName = object .. "_diarydata"
	if GameVars.Engine.Diaries[dataName] then
		GameVars.Engine.Diaries[dataName] = nil
	end
end

CustomDiary.Status = function(value)

	if GameVars.Engine.Diaries then
		if value == true then
			TEN.Logic.AddCallback(TEN.Logic.CallbackPoint.POSTUSEITEM, LevelFuncs.Engine.Diaries.ActivateDiary)
            print("Diary Started")
		elseif value == false then
			TEN.Logic.RemoveCallback(TEN.Logic.CallbackPoint.POSTUSEITEM, LevelFuncs.Engine.Diaries.ActivateDiary)
            print("Diary Stopped")
		end
	end

end

--- The function checks whether the specified diary is currently visible.
-- @treturn bool true if the bar is visible and false if it is not.
function CustomDiary:IsVisible()
    
	if GameVars.Engine.Diaries[self.Name] then
		if GameVars.Engine.Diaries[self.Name].Visible then
			return true
		else
			return false
		end
	end
end

--- The function returns the number of pages in the diary.
-- @treturn int total number of pages in the diary.
function CustomDiary:getUnlockedPageCount()
    
	if GameVars.Engine.Diaries[self.Name] then
		return GameVars.Engine.Diaries[self.Name].UnlockedPages
	end
end

-- The function sets the value of a custom bar over a specified time period.
-- @int value: The new target to which the bar's current value should transition. (Must be a non-negative number; between 0 and the bar's maxValue.
function CustomDiary:unlockPages(index)
    if GameVars.Engine.Diaries[self.Name] then

        if index > #GameVars.Engine.Diaries[self.Name].Pages or index <=0 then
            print("Index provided is higher than the page count.")
            return
        end

        local diary = GameVars.Engine.Diaries[self.Name]
		diary.UnlockedPages = index
        diary.currentPageIndex = index
        diary.NextPageIndex = nil
        print("UnlockPages: currentPageIndex = " .. tostring(diary.currentPageIndex))

        if diary.Notification and next(diary.Notification) then
            PlaySound(diary.Notification.NotificationSound)
            diary.Notification.ElapsedTime = 0
            diary.TargetAlpha = 255
            TEN.Logic.AddCallback(TEN.Logic.CallbackPoint.PRELOOP, LevelFuncs.Engine.Diaries.ShowNotification)
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
function CustomDiary:addTextEntry(pageIndex, text, textX, textY, textAlignment, textEffects, textScale, textColor)
    local textEntry = {
            text = text,
            textX = textX,
            textY = textY,
            textAlignment = textAlignment,
            textEffects = textEffects,
            textScale = textScale,
            textColor = textColor
        }
        if not GameVars.Engine.Diaries[self.Name].Pages[pageIndex] then
            GameVars.Engine.Diaries[self.Name].Pages[pageIndex] = {NarrationTrack=nil, TextEntries = {}, ImageEntries = {}}
        end
        table.insert(GameVars.Engine.Diaries[self.Name].Pages[pageIndex].TextEntries, textEntry)
        print("Text entry added to page: ")
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
function CustomDiary:addImageEntry(pageIndex, objectID, spriteID, color, posX, posY, rot, scaleX, scaleY, alignMode, scaleMode, blendMode)
   local imageEntry = {
            objectID = objectID,
            spriteID = spriteID,
            color = color,
            posX = posX,
            posY = posY,
            rot = rot,
            scaleX = scaleX,
            scaleY = scaleY,
            alignMode = alignMode,
            scaleMode = scaleMode,
            blendMode = blendMode
        }
        if not GameVars.Engine.Diaries[self.Name].Pages[pageIndex] then
            GameVars.Engine.Diaries[self.Name].Pages[pageIndex] = {NarrationTrack=nil, TextEntries = {}, ImageEntries = {}}
        end
        table.insert(GameVars.Engine.Diaries[self.Name].Pages[pageIndex].ImageEntries, imageEntry)
        print("Image entry added to page: ")
end

---
-- Add a narration track in the voice channel to the page. Track is played with the draw button.
-- @tparam int pageIndex page number to add the narration track to.
-- @tparam string trackName of track (without file extension) to play.

function CustomDiary:addNarration(pageIndex, trackName)
    if not GameVars.Engine.Diaries[self.Name].Pages[pageIndex] then
        GameVars.Engine.Diaries[self.Name].Pages[pageIndex] = {NarrationTrack=nil, TextEntries = {}, ImageEntries = {}}
    end
    GameVars.Engine.Diaries[self.Name].Pages[pageIndex].NarrationTrack = trackName
    print("Narration added to page: ")
 end

---
-- Sets the postprocessing mode for diary background.
-- @tparam View.PostProcessMode postProcessMode effect type to set.
-- @tparam number power Postprocessing strength.
-- @tparam Color tintColor Set the tint color that overlays over the chosen color mode.

function CustomDiary:addPostProcess(postProcessMode, power, tintColor)
    if GameVars.Engine.Diaries[self.Name] then
        GameVars.Engine.Diaries[self.Name].PostProcessMode    = postProcessMode
        GameVars.Engine.Diaries[self.Name].PostProcessPower   = power
        GameVars.Engine.Diaries[self.Name].PostProcessColor   = tintColor
        print("Post Processing added to diary: ")
    end
end

-- Sets the postprocessing mode for diary background.
-- @tparam View.PostProcessMode postProcessMode effect type to set.
-- @tparam number power Postprocessing strength.
-- @tparam Color tintColor Set the tint color that overlays over the chosen color mode.

function CustomDiary:customizeNotification(notificationTime, objectID, spriteID, color, posX, posY, rot, scaleX, scaleY, alignMode, scaleMode, blendMode, notificationSound)
    if GameVars.Engine.Diaries[self.Name] then
        GameVars.Engine.Diaries[self.Name].Notification.ObjectID            = objectID
        GameVars.Engine.Diaries[self.Name].Notification.SpriteID	        = spriteID
        GameVars.Engine.Diaries[self.Name].Notification.Color		        = color
        GameVars.Engine.Diaries[self.Name].Notification.PosX			    = posX
        GameVars.Engine.Diaries[self.Name].Notification.PosY			    = posY
        GameVars.Engine.Diaries[self.Name].Notification.Rot			        = rot
        GameVars.Engine.Diaries[self.Name].Notification.ScaleX		        = scaleX
        GameVars.Engine.Diaries[self.Name].Notification.ScaleY		        = scaleY
        GameVars.Engine.Diaries[self.Name].Notification.AlignMode		    = alignMode
        GameVars.Engine.Diaries[self.Name].Notification.ScaleMode		    = scaleMode
        GameVars.Engine.Diaries[self.Name].Notification.BlendMode		    = blendMode
        GameVars.Engine.Diaries[self.Name].Notification.NotificationSound	= notificationSound
        GameVars.Engine.Diaries[self.Name].Notification.NotificationTime    = notificationTime
        GameVars.Engine.Diaries[self.Name].Notification.ElapsedTime         = 0
        
        print("Notification updated")
    end
end

function CustomDiary:customizePageNumbers(type, prefix, separator, textX, textY, textAlignment, textEffects, textScale, textColor)
    if GameVars.Engine.Diaries[self.Name] and type >0 and type <=2 then
        GameVars.Engine.Diaries[self.Name].PageNumbers.type             = type
        GameVars.Engine.Diaries[self.Name].PageNumbers.prefix           = prefix 
        GameVars.Engine.Diaries[self.Name].PageNumbers.separator        = separator 
        GameVars.Engine.Diaries[self.Name].PageNumbers.textX            = textX
        GameVars.Engine.Diaries[self.Name].PageNumbers.textY            = textY
        GameVars.Engine.Diaries[self.Name].PageNumbers.textAlignment    = textAlignment
        GameVars.Engine.Diaries[self.Name].PageNumbers.textEffects	    = textEffects
        GameVars.Engine.Diaries[self.Name].PageNumbers.textScale		= textScale
        GameVars.Engine.Diaries[self.Name].PageNumbers.textColor		= textColor
        
        print("Page Numbers updated")
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
function CustomDiary:importData(filename)

    local diaryData = require(filename)

    for pageIndex, page in ipairs(diaryData.Pages) do
        self:addImageEntry(pageIndex, page.Image.objectIDbg, page.Image.spriteIDbg, page.Image.colorBG, page.Image.posX, page.Image.posY, page.Image.rot, page.Image.scaleX, page.Image.scaleY, page.Image.alignMode, page.Image.scaleMode, page.Image.blendMode)
        for _, text in ipairs(page.Text) do
            self:addTextEntry(pageIndex, text.text, text.textX, text.textY, text.textAlignment, text.textEffects, text.textScale, text.textColor)
        end
    end
end

-- !Ignore
LevelFuncs.Engine.Diaries.ShowDiary = function()

    local objectNumber = GameVars.Engine.LastUsedDiary
	local dataName = objectNumber .. "_diarydata"

    if GameVars.Engine.Diaries[dataName] then
        
        local diary             = GameVars.Engine.Diaries[dataName]
        local currentIndex      = diary.currentPageIndex
        local maxPages          = diary.UnlockedPages
        local narrationTrack    = diary.Pages[currentIndex].NarrationTrack
        local alphaDelta        = diary.AlphaBlendSpeed 
        
        if diary.CurrentAlpha ~= diary.TargetAlpha then
            
            if diary.CurrentAlpha < diary.TargetAlpha then
                diary.CurrentAlpha = math.floor(math.min(diary.CurrentAlpha + alphaDelta, diary.TargetAlpha))
            else
                diary.CurrentAlpha = math.floor(math.max(diary.CurrentAlpha - alphaDelta, diary.TargetAlpha))
            end
        end

        -- Switch pages when entries have fully faded out
        if diary.EntryFadingOut then
            -- Fade out current entries
            if diary.EntryCurrentAlpha > 0 then
                diary.EntryCurrentAlpha = math.max(0, diary.EntryCurrentAlpha - alphaDelta)
            else
                -- Fade-out completed, switch page and start fade-in
                diary.EntryFadingOut = false
                diary.EntryFadingIn = true
                diary.EntryCurrentAlpha = 0
                currentIndex = diary.NextPageIndex and diary.NextPageIndex or 1-- Update to the new page
            end
        elseif diary.EntryFadingIn then
            -- Fade in new entries
            if diary.EntryCurrentAlpha < diary.EntryTargetAlpha then
                diary.EntryCurrentAlpha = math.min(diary.EntryTargetAlpha, diary.EntryCurrentAlpha + alphaDelta)
            else
                -- Fade-in completed
                diary.EntryFadingIn = false
            end
        end

        if KeyIsHit(ActionID.DRAW) then
            if narrationTrack then
            PlayAudioTrack(narrationTrack, Sound.SoundTrackType.VOICE)

            --add code for narration track icon to show
            end
        elseif KeyIsHit(ActionID.LEFT) and not (diary.EntryFadingOut or diary.EntryFadingIn) then
            -- Initiate fade-out to switch to the previous page
                if currentIndex > 1 then
                    diary.EntryFadingOut = true
                    diary.NextPageIndex = math.max(1, currentIndex - 1)
                    StopAudioTrack(Sound.SoundTrackType.VOICE)
                    PlaySound(diary.PageSound)
                end
        elseif KeyIsHit(ActionID.RIGHT) and not (diary.EntryFadingOut or diary.EntryFadingIn) then
                -- Initiate fade-out to switch to the next page
                if currentIndex < maxPages then
                    diary.EntryFadingOut = true
                    diary.NextPageIndex = math.min(maxPages, currentIndex + 1)
                    StopAudioTrack(Sound.SoundTrackType.VOICE)
                    PlaySound(diary.PageSound)
                end            
        elseif KeyIsHit(ActionID.INVENTORY) then
            diary.TargetAlpha = 0
            diary.EntryFadingOut = true
        end

       --Sets the currentindex so that the diary opens at the same page
        diary.currentPageIndex = currentIndex
        local textEntries = GameVars.Engine.Diaries[dataName].Pages[currentIndex].TextEntries
        local imageEntries = GameVars.Engine.Diaries[dataName].Pages[currentIndex].ImageEntries

        if diary.CurrentAlpha > 0 then
            diary.Visible = true
        elseif diary.CurrentAlpha == 0 and diary.EntryCurrentAlpha == 0 then
            diary.Visible = false
            StopAudioTrack(Sound.SoundTrackType.VOICE)
            PlaySound(diary.ExitSound)
            SetPostProcessMode(View.PostProcessMode.NONE)
            SetPostProcessStrength(1)
            SetPostProcessTint(Color(255,255,255))
            Flow.SetFreezeMode(Flow.FreezeMode.NONE)
            TEN.Logic.RemoveCallback(TEN.Logic.CallbackPoint.PREFREEZE, LevelFuncs.Engine.Diaries.ShowDiary)
            return
        end
        
        --show page numbers code
        if diary.PageNumbers and next(diary.PageNumbers) then
            
            local entry = diary.PageNumbers
            local pageNumbers = currentIndex
            if entry.type == 2 then
                pageNumbers = entry.prefix .. currentIndex  .. entry.separator .. diary.UnlockedPages
            end

            local pageNumberText = LevelFuncs.Engine.Node.GenerateString(pageNumbers, entry.textX, entry.textY, entry.textScale, entry.textAlignment, entry.textEffects, entry.textColor, (diary.CurrentAlpha/255))
            ShowString(pageNumberText, 1 / 30)

        end

        if textEntries or imageEntries then
            -- Draw background sprite
            local bgColor = TEN.Color(diary.ColorBG.r, diary.ColorBG.g,diary.ColorBG.b,diary.CurrentAlpha)
            local bgPos = TEN.Vec2(diary.PosX, diary.PosY)
            local bgScale = TEN.Vec2(diary.ScaleX, diary.ScaleY)
            local bgSprite = TEN.DisplaySprite(diary.ObjectIDbg, diary.SpriteIDbg, bgPos, diary.Rot, bgScale, bgColor)
            bgSprite:Draw(0, diary.AlignMode, diary.ScaleMode, diary.BlendMode)
            --add code for controls

            -- Draw entries based on type
            
            

            if textEntries then
                for _, entry in ipairs(textEntries) do
                        
                        local diaryText = LevelFuncs.Engine.Node.GenerateString(entry.text, entry.textX, entry.textY, entry.textScale, entry.textAlignment, entry.textEffects, entry.textColor, normalAlpha)
                        ShowString(diaryText, 1 / 30)
                end
            end

            if imageEntries then
                for _, entry in ipairs(imageEntries) do
                    local entryColor = TEN.Color(entry.color.r,entry.color.g,entry.color.b,diary.EntryCurrentAlpha)
                    local entryPos = TEN.Vec2(entry.posX, entry.posY)
                    local entryScale = TEN.Vec2(entry.scaleX, entry.scaleY)
                    local entrySprite = TEN.DisplaySprite(entry.objectID, entry.spriteID, entryPos, entry.rot, entryScale, entryColor)
                
                    entrySprite:Draw(1, entry.alignMode, entry.scaleMode, entry.blendMode)
                end
            end
        
            -- local myTextString = "Alpha: " .. diary.PostProcessMode  .."-"..	diary.PostProcessPower.."-"..tostring(diary.PostProcessColor)
            -- local myText = DisplayString(myTextString, 100, 400, Color.new(64,250,60))
            -- ShowString(myText,1/30)

        end
    end
end

-- !Ignore
LevelFuncs.Engine.Diaries.ActivateDiary = function(objectNumber)
	
    GameVars.Engine.LastUsedDiary = objectNumber
	local dataName = objectNumber .. "_diarydata"
	if GameVars.Engine.Diaries[dataName] then
        TEN.Inventory.ClearUsedItem()
        GameVars.Engine.Diaries[dataName].TargetAlpha = 255
        GameVars.Engine.Diaries[dataName].EntryTargetAlpha = 255
		TEN.Logic.AddCallback(TEN.Logic.CallbackPoint.PREFREEZE, LevelFuncs.Engine.Diaries.ShowDiary)
        if GameVars.Engine.Diaries[dataName].PostProcessMode then
            TEN.View.SetPostProcessMode(GameVars.Engine.Diaries[dataName].PostProcessMode)
            TEN.View.SetPostProcessStrength(GameVars.Engine.Diaries[dataName].PostProcessPower)
            TEN.View.SetPostProcessTint(GameVars.Engine.Diaries[dataName].PostProcessColor)
            --print("Post Process mode has been set: "..GameVars.Engine.Diaries[dataName].PostProcessMode)
        end
        Flow.SetFreezeMode(Flow.FreezeMode.FULL)
	end

end

-- !Ignore
LevelFuncs.Engine.Diaries.ShowNotification = function(dt)
	
    local dataName = GameVars.Engine.LastUsedDiary .. "_diarydata"
	
    if GameVars.Engine.Diaries[dataName] then
        local diary = GameVars.Engine.Diaries[dataName]

        if diary.CurrentAlpha ~= diary.TargetAlpha then
            
            if diary.CurrentAlpha < diary.TargetAlpha then
                diary.CurrentAlpha = math.floor(math.min(diary.CurrentAlpha + diary.AlphaBlendSpeed, diary.TargetAlpha))
            else
                diary.CurrentAlpha = math.floor(math.max(diary.CurrentAlpha - diary.AlphaBlendSpeed, diary.TargetAlpha))
            end

        end

        GameVars.Engine.Diaries[dataName].Notification.ElapsedTime  = GameVars.Engine.Diaries[dataName].Notification.ElapsedTime + dt
        
        if GameVars.Engine.Diaries[dataName].Notification.ElapsedTime <= GameVars.Engine.Diaries[dataName].Notification.NotificationTime then
            diary.TargetAlpha = 255
        else
            diary.TargetAlpha = 0
        end
        
        if diary.CurrentAlpha > 0 then
            LevelFuncs.Engine.Diaries.PrepareNotification()
        elseif diary.CurrentAlpha == 0 then
            GameVars.Engine.Diaries[dataName].Notification.ElapsedTime = 0
            TEN.Logic.RemoveCallback(TEN.Logic.CallbackPoint.PRELOOP, LevelFuncs.Engine.Diaries.ShowNotification)
            return
        end
	end

end

LevelFuncs.Engine.Diaries.PrepareNotification = function()
	
	local dataName = GameVars.Engine.LastUsedDiary .. "_diarydata"
	
    if GameVars.Engine.Diaries[dataName] then


        local diary = GameVars.Engine.Diaries[dataName]
        local notif = diary.Notification
        local spritePos = TEN.Vec2(notif.PosX, notif.PosY)
        local spriteScale = TEN.Vec2(notif.ScaleX, notif.ScaleY) 
        local spriteColor = Color(notif.Color.r,notif.Color.g,notif.Color.b,diary.CurrentAlpha)

        local sprite = TEN.DisplaySprite(notif.ObjectID, notif.SpriteID, spritePos, notif.Rot, spriteScale, spriteColor)
        sprite:Draw(0, notif.AlignMode,  notif.ScaleMode,  notif.BlendMode)
    end

end

return CustomDiary