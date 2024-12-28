-----
--- Diaries:
--The module provides functions to create and manage Diaries. It maintains state through LevelVars.Engine.Diaries which stores diary definitions and configurations.
-- Each diary is accessed by the object that was used to create it. 
--
-- @luautil Diary
local CustomDiary = {}
CustomDiary.__index = CustomDiary

LevelFuncs.Engine.Diaries = {}
GameVars.Engine.Diaries = {}
GameVars.Engine.LastUsedDiary=nil

---
-- Imports diary from an external file.
-- @tparam string fileName Name of file in the script folder without extension to import the diary from.
function CustomDiary.ImportDiary(fileName)
    
    local importDiary = nil
    local diaryData = require(fileName)
    
    --Create the diary
    for _, entry in ipairs(diaryData) do
        if entry.type == "diary" then
            CustomDiary.Create(
                entry.object,
                entry.objectIdBg,
                entry.spriteIdBg,
                entry.colorBg,
                entry.pos,
                entry.rot,
                entry.scale,
                entry.alignMode,
                entry.scaleMode,
                entry.blendMode,
                entry.alpha,
                entry.pageSound,
                entry.exitSound
            )
            
            importDiary = CustomDiary.Get(entry.object)
            importDiary:unlockPages(entry.pagesToUnlock, false)
        end

    end

    --Start the diary
    CustomDiary.Status(true)

    for _, entry in ipairs(diaryData) do
        if entry.type == "background" then
            importDiary:addBackground(
                entry.objectIdBg,
                entry.spriteIdBg,
                entry.colorBg,
                entry.pos,
                entry.rot,
                entry.scale,
                entry.alignMode,
                entry.scaleMode,
                entry.blendMode,
                entry.alpha
            )
        elseif entry.type == "pageNumbers" then
            importDiary:customizePageNumbers(
                entry.pageNoType,
                entry.prefix,
                entry.separator,
                entry.textPos,
                entry.textOptions,
                entry.textScale,
                entry.textColor
            )
        elseif entry.type == "controls" then
            importDiary:customizeControls(
                entry.textPos,
                entry.textOptions,
                entry.textScale,
                entry.textColor)
            importDiary:customizeControlsText(
                entry.string1,
                entry.string2,
                entry.string3,
                entry.string4)
        elseif entry.type == "notification" then
            importDiary:customizeNotification(
                entry.notificationTime,
                entry.objectId, 
                entry.spriteId, 
                entry.color, 
                entry.pos, 
                entry.rot, 
                entry.scale, 
                entry.alignMode, 
                entry.scaleMode, 
                entry.blendMode, 
                entry.notificationSound
            )
        elseif entry.type == "image" then
            importDiary:addImageEntry(
                entry.pageIndex, 
                entry.objectIdBg, 
                entry.spriteIdBg, 
                entry.colorBg, 
                entry.pos,
                entry.rot, 
                entry.scale,
                entry.alignMode,
                entry.scaleMode,
                entry.blendMode
            )
        elseif entry.type == "text" then
            importDiary:addTextEntry(
                entry.pageIndex,
                entry.text,
                entry.textPos, 
                entry.textOptions,
                entry.textScale,
                entry.textColor
            )
        elseif entry.type == "narration" then
            importDiary:addNarration(
                entry.pageIndex,
                entry.trackName
            )
        else
            print("Unknown entry type: " .. tostring(entry.type))
        end
    end

    print("External diary from file: "..tostring(filename).." imported")
end

---
-- Creates a diary with extensive configuration options.
-- Parameters:
-- @tparam Objects.ObjID object The pickup object that will be used to create the diary. Access the diary by selecting the item in the inventory. (596-611)
-- @tparam Objects.ObjID objectIdBg Object ID for the diary's sprite.
-- @tparam number spriteIdBg SpriteID from the specified object for the diary's sprite.
-- @tparam Color colorBg Color of diary's sprite.
-- @tparam Vec2 posBg X,Y position of the bar's background in screen percent (0-100).
-- @tparam float rotBG rotation of the diary's sprite (0-360).
-- @tparam Vec2 scaleBg X,Y Scaling factor for the bar's background sprite.
-- @tparam View.AlignMode alignModebg Alignment for the diary's sprite.
-- @tparam View.ScaleMode scaleModebg Scaling for the diary's sprite.
-- @tparam Effects.BlendID blendModebg Blending modes for the diary's sprite.
-- @tparam Sound pageSound Sound to play with page turn.
-- @tparam Sound exitSound Sound to play when existing the diary.
-- @treturn the Diary table
CustomDiary.Create = function(object, objectIdBg, spriteIdBg, colorBg, pos, rot, scale, alignMode, scaleMode, blendMode, alpha, pageSound, exitSound)

    if (object < 596 or object > 611) and object ~= 986 then
        print("Error: Invalid object slot for diary creation. Please use a pickup object slot in the range 596 to 611 and 986.")
        return
    end

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
	GameVars.Engine.Diaries[dataName].ObjectIDbg	        = objectIdBg
	GameVars.Engine.Diaries[dataName].SpriteIDbg	        = spriteIdBg
	GameVars.Engine.Diaries[dataName].ColorBg		        = colorBg
	GameVars.Engine.Diaries[dataName].Pos			        = pos
	GameVars.Engine.Diaries[dataName].Rot			        = rot
	GameVars.Engine.Diaries[dataName].Scale 		        = scale
	GameVars.Engine.Diaries[dataName].AlignMode		        = alignMode
	GameVars.Engine.Diaries[dataName].ScaleMode		        = scaleMode
	GameVars.Engine.Diaries[dataName].BlendMode		        = blendMode
    GameVars.Engine.Diaries[dataName].Alpha     	        = alpha
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
    GameVars.Engine.Diaries[dataName].Controls              = {}
    GameVars.Engine.Diaries[dataName].Background            = {}
    GameVars.Engine.Diaries[dataName].AlphaBlendSpeed       = 100
    GameVars.Engine.Diaries[dataName].EntryFadingIn         = true

	
	print("CustomDiary Constructed for CustomDiary: " .. dataName)
    return setmetatable(self, CustomDiary)
end

---
-- The function retrieves a diary by its unique object. This function is useful when you need to access or manipulate a diary that has already been created .
-- @tparam Objects.ObjID object The pickup object that was used to create the diary (596-611).
-- @treturn The diary
CustomDiary.Get = function(object)
	local dataName = object .. "_diarydata"
    if GameVars.Engine.Diaries[dataName] then
        local self = {Name = dataName}
        return setmetatable(self, CustomDiary)
    end
end

---
-- The function removes a custom diary and its associated data from the system. It ensures that the diary is no longer tracked or accessible in the LevelVars.Engine.Diaries.
-- @tparam Objects.ObjID object The pickup object that was used to create the diary (596-611).
CustomDiary.Delete = function (object)
    local dataName = object .. "_diarydata"
	if GameVars.Engine.Diaries[dataName] then
		GameVars.Engine.Diaries[dataName] = nil
	end
end

-- The function add the callback to enabl diaries in levels. This needs to be added to every level preferably in the startLevelFuncs.OnStart.
-- 	@bool value True enables the diaries to be activated. False would disable the diaries.
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

--- 
-- The function checks whether the specified diary is currently visible.
-- @treturn bool true if the diary is visible and false if it is not.
function CustomDiary:IsVisible()
    
	if GameVars.Engine.Diaries[self.Name] then
		return GameVars.Engine.Diaries[self.Name].Visible
	end
end

--- 
-- The function displays the specified diary.
function CustomDiary:showDiary()
    
	if GameVars.Engine.Diaries[self.Name] then

		local object = GameVars.Engine.Diaries[self.Name].Object

        LevelFuncs.Engine.Diaries.ActivateDiary(object)

	end

end

--- 
-- The function returns the number of pages in the diary.
-- @treturn int total number of unlocked pages in the diary.
function CustomDiary:getUnlockedPageCount()
    
	if GameVars.Engine.Diaries[self.Name] then
		return GameVars.Engine.Diaries[self.Name].UnlockedPages
	end
end

---
-- The function sets the value of a custom bar over a specified time period.
-- @tparam int pageIndex The page number up to which the diary should be unlocked.
-- @tparam bool notification If true, and notification has been defined, a notification icon and sound will be played.
function CustomDiary:unlockPages(index, notification)
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

        if notification and diary.Notification and next(diary.Notification) then
            PlaySound(diary.Notification.NotificationSound)
            diary.Notification.ElapsedTime = 0
            diary.TargetAlpha = 255
            diary.CurrentAlpha = 1
            TEN.Logic.AddCallback(TEN.Logic.CallbackPoint.PRELOOP, LevelFuncs.Engine.Diaries.ShowNotification)
        end
    end
end

---
-- The function clears the page number for the diary.
-- @tparam int pageIndex The page number to be cleared.
function CustomDiary:clearPage(index)
    if GameVars.Engine.Diaries[self.Name] then
        GameVars.Engine.Diaries[self.Name].Pages[index]= {NarrationTrack=nil,TextEntries={},ImageEntries={}}
        print("Page Cleared")
    end
end

---
-- Adds a text entry to the specified page for the diary.
-- @tparam int pageIndex page number to add the text entry to.
-- @tparam string text Text entry to be added to the page.
-- @tparam Vec2 textPos X,Y position of the text.
-- @tparam Strings.DisplayStringOption textOptions alignment and effects for the text. Default: None. Please note text is automatically aligned to the LEFT
-- @tparam number textScale Scale factor for the text.
-- @tparam Color textColor Color of the text.
function CustomDiary:addTextEntry(pageIndex, text, textPos, textOptions, textScale, textColor)
    local textEntry = {
            text = text,
            textPos = textPos,
            textOptions = textOptions,
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
-- Adds an image entry to the specified page for the diary.
-- @tparam int pageIndex page number to add the image entry to.
-- @tparam Objects.ObjID objectIDbg Object ID for the image entry sprite.
-- @tparam number spriteID SpriteID from the specified object for the image entry.
-- @tparam Color color Color of image entry.
-- @tparam Vec2 posBg X,Y position of the image entry in screen percent (0-100).
-- @tparam number rot rotation of the image entry (0-360).
-- @tparam Vec2 scaleBg X,Y Scaling factor for the image entry.
-- @tparam View.AlignMode alignMode Alignment for the image entry.
-- @tparam View.ScaleMode scaleMode Scaling for the image entry.
-- @tparam Effects.BlendID blendMode Blending modes for the image entry.
function CustomDiary:addImageEntry(pageIndex, objectId, spriteId, color, pos, rot, scale, alignMode, scaleMode, blendMode)
   local imageEntry = {
            objectId = objectId,
            spriteId = spriteId,
            color = color,
            pos = pos,
            rot = rot,
            scale = scale,
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
-- Remove the narration track from the page of the specified diary.
-- @tparam int pageIndex page number to add the narration track to.
function CustomDiary:removeNarration(pageIndex)
    if GameVars.Engine.Diaries[self.Name].Pages[pageIndex] then
		GameVars.Engine.Diaries[self.Name].Pages[pageIndex].NarrationTrack = {}
	end
    print("Narration added to page: ".. pageIndex)
 end

---
-- Add a background image for the diary.
-- @tparam Objects.ObjID objectIdBg Object ID for the diary's background.
-- @tparam number spriteIdBg SpriteID from the specified object for the diary's background.
-- @tparam Color colorBg Color of diary's background.
-- @tparam Vec2 pos X,Y position of the diary's background in screen percent (0-100).
-- @tparam float rot rotation of the diary's background sprite (0-360).
-- @tparam Vec2 scale X,Y Scaling factor for the diary's background.
-- @tparam View.AlignMode alignModebg Alignment for the diary's background.
-- @tparam View.ScaleMode scaleModebg Scaling for the diary's background.
-- @tparam Effects.BlendID blendModebg Blending modes for the diary's background.
function CustomDiary:addBackground(objectIdBg, spriteIdBg, colorBg, pos, rot, scale, alignMode, scaleMode, blendMode, alpha)
    if GameVars.Engine.Diaries[self.Name] then
        GameVars.Engine.Diaries[self.Name].Background.ObjectIdBg	    = objectIdBg
	    GameVars.Engine.Diaries[self.Name].Background.SpriteIdBg	    = spriteIdBg
	    GameVars.Engine.Diaries[self.Name].Background.ColorBg		    = colorBg
	    GameVars.Engine.Diaries[self.Name].Background.Pos			    = pos
	    GameVars.Engine.Diaries[self.Name].Background.Rot			    = rot
	    GameVars.Engine.Diaries[self.Name].Background.Scale		        = scale
	    GameVars.Engine.Diaries[self.Name].Background.AlignMode		    = alignMode
	    GameVars.Engine.Diaries[self.Name].Background.ScaleMode		    = scaleMode
	    GameVars.Engine.Diaries[self.Name].Background.BlendMode 	    = blendMode
        GameVars.Engine.Diaries[self.Name].Background.Alpha		        = alpha
        print("Background added to diary: ")
    end
end

---
-- Clears settings for the background for the specified diary.
function CustomDiary:clearBackground()
    if GameVars.Engine.Diaries[self.Name] then
        GameVars.Engine.Diaries[self.Name].Background = {}
        
        print("Background cleared")
    end
end

---
-- Customizes the notification icon and sound for the diary.
-- @tparam number notificationTime Time in seconds the notification icon will show on screen.
-- @tparam Objects.ObjID objectId Object ID for the notification icon.
-- @tparam number spriteId SpriteID from the specified object for the notification icon.
-- @tparam Color color Color of notification icon.
-- @tparam Vec2 pos X,Y position of the notification icon in screen percent (0-100).
-- @tparam number rot rotation of the notification icon (0-360).
-- @tparam Vec2 scale X,Y Scaling factor for the notification icon.
-- @tparam View.AlignMode alignMode Alignment for the notification icon.
-- @tparam View.ScaleMode scaleMode Scaling for the notification icon.
-- @tparam Effects.BlendID blendMode Blending modes for the notification icon.
-- @tparam Sound notificationSound Sound to play with notification icon.
function CustomDiary:customizeNotification(notificationTime, objectId, spriteId, color, pos, rot, scale, alignMode, scaleMode, blendMode, notificationSound)
    if GameVars.Engine.Diaries[self.Name] then
        GameVars.Engine.Diaries[self.Name].Notification.ObjectID            = objectId
        GameVars.Engine.Diaries[self.Name].Notification.SpriteID	        = spriteId
        GameVars.Engine.Diaries[self.Name].Notification.Color		        = color
        GameVars.Engine.Diaries[self.Name].Notification.Pos			        = pos
        GameVars.Engine.Diaries[self.Name].Notification.Rot			        = rot
        GameVars.Engine.Diaries[self.Name].Notification.Scale		        = scale
        GameVars.Engine.Diaries[self.Name].Notification.AlignMode		    = alignMode
        GameVars.Engine.Diaries[self.Name].Notification.ScaleMode		    = scaleMode
        GameVars.Engine.Diaries[self.Name].Notification.BlendMode		    = blendMode
        GameVars.Engine.Diaries[self.Name].Notification.NotificationSound	= notificationSound
        GameVars.Engine.Diaries[self.Name].Notification.NotificationTime    = notificationTime
        GameVars.Engine.Diaries[self.Name].Notification.ElapsedTime         = 0
        
        print("Notification updated")
    end
end

---
-- Clears settings for the notification system for the specified diary.
function CustomDiary:clearNotification()
    if GameVars.Engine.Diaries[self.Name] then
        GameVars.Engine.Diaries[self.Name].Notification = {}
        
        print("Notifications cleared")
    end
end

---
-- Customizes the page numbers for the diary.
-- @tparam int pageNoType Specifies the format for page numbers (1 or 2). 1: Displays only the current page number. 2: Formats the page number as: [Prefix][CurrentPage][Separator][UnlockedPages].
-- @tparam string prefix Prefix to be added for type 2 of page numbers.
-- @tparam string separator Separator to be added for type 2 of page numbers.
-- @tparam Vec2 textPos X,Y position of the page numbers.
-- @tparam Strings.DisplayStringOption textOptions alignment and effects for the text. Default: None. Please note text is automatically aligned to the LEFT
-- @tparam number textScale Scale factor for the page numbers.
-- @tparam Color textColor Color of the page numbers.
function CustomDiary:customizePageNumbers(pageNoType, prefix, separator, textPos, textOptions, textScale, textColor)
    if GameVars.Engine.Diaries[self.Name] and pageNoType >0 and pageNoType <=2 then
        GameVars.Engine.Diaries[self.Name].PageNumbers.pageNoType       = pageNoType
        GameVars.Engine.Diaries[self.Name].PageNumbers.prefix           = prefix 
        GameVars.Engine.Diaries[self.Name].PageNumbers.separator        = separator 
        GameVars.Engine.Diaries[self.Name].PageNumbers.textPos          = textPos
        GameVars.Engine.Diaries[self.Name].PageNumbers.textOptions      = textOptions
        GameVars.Engine.Diaries[self.Name].PageNumbers.textScale		= textScale
        GameVars.Engine.Diaries[self.Name].PageNumbers.textColor		= textColor
        
        print("Page Numbers updated")
    end
end

---
-- Clears settings for the page numbers for the specified diary.
function CustomDiary:clearPageNumbers()
    if GameVars.Engine.Diaries[self.Name] then
        GameVars.Engine.Diaries[self.Name].PageNumbers = nil
		print("Page Numbers cleared")
    end
end

---
-- Customizes the controls bar for the diary.
-- @tparam Vec2 textPos X,Y position of the controls text.
-- @tparam Strings.DisplayStringOption textOptions alignment and effects for the text. Default: None. Please note text is automatically aligned to the LEFT.
-- @tparam number textScale Scale factor for the controls.
-- @tparam Color textColor Color of the page controls.
function CustomDiary:customizeControls(textPos, textOptions, textScale, textColor)
    if GameVars.Engine.Diaries[self.Name] then
        GameVars.Engine.Diaries[self.Name].Controls.text1           = "Space: Play Voice Note"
        GameVars.Engine.Diaries[self.Name].Controls.text2           = "Left Key: Previous Page" 
        GameVars.Engine.Diaries[self.Name].Controls.text3           = "Right Key: Next Page"
        GameVars.Engine.Diaries[self.Name].Controls.text4           = "Esc: Back"
        GameVars.Engine.Diaries[self.Name].Controls.textPos         = textPos
        GameVars.Engine.Diaries[self.Name].Controls.textOptions     = textOptions
        GameVars.Engine.Diaries[self.Name].Controls.textScale		= textScale
        GameVars.Engine.Diaries[self.Name].Controls.textColor		= textColor
        
        print("Controls updated")
    end
end

---
-- Customizes the display text for controls for specified diary.
-- @tparam string Text for Space key controls text.
-- @tparam string Text for Left key controls text.
-- @tparam string Text for Right key controls text.
-- @tparam string Text for Esc key controls text.
function CustomDiary:customizeControlsText(string1, string2, string3, string4)
    if GameVars.Engine.Diaries[self.Name] then
        GameVars.Engine.Diaries[self.Name].Controls.text1           = string1
        GameVars.Engine.Diaries[self.Name].Controls.text2           = string2
        GameVars.Engine.Diaries[self.Name].Controls.text3           = string3
        GameVars.Engine.Diaries[self.Name].Controls.text4           = string4
        print("Controls text updated")
    end
end


---
-- Clears settings for the control display system for the specified diary.
function CustomDiary:clearControls()
    if GameVars.Engine.Diaries[self.Name] then
        GameVars.Engine.Diaries[self.Name].Controls ={}
        
        print("Controls cleared")
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
            PlaySound(diary.ExitSound)
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
            Flow.SetFreezeMode(Flow.FreezeMode.NONE)
            TEN.Logic.RemoveCallback(TEN.Logic.CallbackPoint.PREFREEZE, LevelFuncs.Engine.Diaries.ShowDiary)
            return
        end

        if textEntries or imageEntries then
            -- Draw Diary sprite
            local dAlpha = math.min(diary.CurrentAlpha, diary.Alpha)
            local dColor = TEN.Color(diary.ColorBg.r, diary.ColorBg.g, diary.ColorBg.b, dAlpha)
            local dSprite = TEN.DisplaySprite(diary.ObjectIDbg, diary.SpriteIDbg, diary.Pos, diary.Rot, diary.Scale, dColor)
            dSprite:Draw(1, diary.AlignMode, diary.ScaleMode, diary.BlendMode)

            -- Draw Background Image
            if diary.Background and next(diary.Background) then
                local bgAlpha = math.min(diary.CurrentAlpha, diary.Background.Alpha)
                local bgColor = TEN.Color(diary.Background.ColorBg.r, diary.Background.ColorBg.g, diary.Background.ColorBg.b, bgAlpha)
                local bgSprite = TEN.DisplaySprite(diary.Background.ObjectIdBg, diary.Background.SpriteIdBg, diary.Background.Pos, diary.Background.Rot, diary.Background.Scale, bgColor)
                bgSprite:Draw(0, diary.Background.AlignMode, diary.Background.ScaleMode, diary.Background.BlendMode)
            end

            if diary.Controls.textPos then

                local controlTexts = {}

                if narrationTrack then
                    table.insert(controlTexts, diary.Controls.text1)
                end
                if currentIndex > 1 then
                    table.insert(controlTexts, diary.Controls.text2)
                end
                if currentIndex < maxPages then
                    table.insert(controlTexts, diary.Controls.text3)
                end

                -- Add the always-present back control
                table.insert(controlTexts, diary.Controls.text4)
                local alignedText = table.concat(controlTexts, " | ")
                local controlPosInPixel = TEN.Vec2(TEN.Util.PercentToScreen(diary.Controls.textPos.x, diary.Controls.textPos.y))
                local IsString = TEN.Flow.IsStringPresent(alignedText)
                local textColor = TEN.Color(diary.Controls.textColor.r, diary.Controls.textColor.g, diary.Controls.textColor.b, diary.CurrentAlpha)

                local controlsText = TEN.Strings.DisplayString(alignedText, controlPosInPixel, diary.Controls.textScale, textColor, IsString, diary.Controls.textOptions)
                ShowString(controlsText, 1 / 30)

            end



            --Draw Page Numbers
            if diary.PageNumbers and next(diary.PageNumbers) then

                local pageNo = diary.PageNumbers
                local pageNumbers = tostring(currentIndex)
                if pageNo.pageNoType == 2 then
                    pageNumbers = pageNo.prefix .. currentIndex  .. pageNo.separator .. diary.UnlockedPages
                end

                local pageNoPosInPixel = TEN.Vec2(TEN.Util.PercentToScreen(pageNo.textPos.x, pageNo.textPos.y))
                local IsString = TEN.Flow.IsStringPresent(pageNumbers)
                local textColor = TEN.Color(pageNo.textColor.r, pageNo.textColor.g, pageNo.textColor.b, diary.CurrentAlpha)

                local pageNumberText = TEN.Strings.DisplayString(pageNumbers, pageNoPosInPixel, pageNo.textScale, textColor, IsString, pageNo.textOptions)
                ShowString(pageNumberText, 1 / 30)

            end

            -- Draw entries based on type
            if textEntries then
                for _, entry in ipairs(textEntries) do

                    local entryPosInPixel = TEN.Vec2(TEN.Util.PercentToScreen(entry.textPos.x, entry.textPos.y))
                    local IsString = TEN.Flow.IsStringPresent(entry.text)
                    local textColor = TEN.Color(entry.textColor.r, entry.textColor.g, entry.textColor.b, diary.EntryCurrentAlpha)

                    local entryText = TEN.Strings.DisplayString(entry.text, entryPosInPixel, entry.textScale, textColor, IsString, entry.textOptions)
                    ShowString(entryText, 1 / 30)

                end
            end

            if imageEntries then
                for _, entry in ipairs(imageEntries) do

                    local entryColor = TEN.Color(entry.color.r,entry.color.g,entry.color.b,diary.EntryCurrentAlpha)
                    local entrySprite = TEN.DisplaySprite(entry.objectId, entry.spriteId, entry.pos, entry.rot, entry.scale, entryColor)

                    entrySprite:Draw(2, entry.alignMode, entry.scaleMode, entry.blendMode)

                end
            end
        end
    end
end

-- !Ignore
LevelFuncs.Engine.Diaries.ActivateDiary = function(objectNumber)
	
    local dataName = objectNumber .. "_diarydata"

	if GameVars.Engine.Diaries[dataName] then
        GameVars.Engine.LastUsedDiary = objectNumber
        TEN.Inventory.ClearUsedItem()
        GameVars.Engine.Diaries[dataName].TargetAlpha = 255
        GameVars.Engine.Diaries[dataName].EntryTargetAlpha = 255
		TEN.Logic.AddCallback(TEN.Logic.CallbackPoint.PREFREEZE, LevelFuncs.Engine.Diaries.ShowDiary)
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
            diary.Notification.ElapsedTime = 0
            TEN.Logic.RemoveCallback(TEN.Logic.CallbackPoint.PRELOOP, LevelFuncs.Engine.Diaries.ShowNotification)
            print("Notification Callback removed")
            return
        end
	end

end

-- !Ignore
LevelFuncs.Engine.Diaries.PrepareNotification = function()
	
	local dataName = GameVars.Engine.LastUsedDiary .. "_diarydata"
	
    if GameVars.Engine.Diaries[dataName] then

        local diary = GameVars.Engine.Diaries[dataName]
        local notif = diary.Notification
        local spriteColor = Color(notif.Color.r, notif.Color.g, notif.Color.b, diary.CurrentAlpha)

        local sprite = TEN.DisplaySprite(notif.ObjectID, notif.SpriteID, notif.Pos, notif.Rot, notif.Scale, spriteColor)
        sprite:Draw(0, notif.AlignMode,  notif.ScaleMode,  notif.BlendMode)

    end

end

return CustomDiary