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
-- Creates a diary with extensive configuration options.
-- Parameters:
-- @tparam Objects.ObjID object The pickup object that will be used to create the diary. Access the diary by selecting the item in the inventory. (596-611)
-- @tparam Objects.ObjID objectIDbg Object ID for the diary's sprite.
-- @tparam number spriteIDbg SpriteID from the specified object for the diary's sprite.
-- @tparam Color colorbg Color of diary's sprite.
-- @tparam number posY, X position of the diary's sprite in screen percent (0-100).
-- @tparam number posX, Y position of the diary's sprite in screen percent (0-100).
-- @tparam float rotBG rotation of the diary's sprite (0-360).
-- @tparam number scaleX, X Scaling factor for the diary's sprite.
-- @tparam number scaleY, Y Scaling factor for the diary's sprite.
-- @tparam View.AlignMode alignModebg Alignment for the diary's sprite.
-- @tparam View.ScaleMode scaleModebg Scaling for the diary's sprite.
-- @tparam Effects.BlendID blendModebg Blending modes for the diary's sprite.
-- @tparam Sound pageSound Sound to play with page turn.
-- @tparam Sound exitSound Sound to play when existing the diary.
-- @treturn the Diary table
CustomDiary.Create = function(object, objectIDbg, spriteIDbg, colorbg, posX, posY, rot, scaleX, scaleY, alignMode, scaleMode, blendMode, alpha, pageSound, exitSound)

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
-- Adds a text entry to the specified page for the diary.
-- @tparam int pageIndex page number to add the text entry to.
-- @tparam string text Text entry to be added to the page.
-- @tparam number textPosX X position of the text.
-- @tparam number textPosX Y position of the text.
-- @tparam Strings.DisplayStringOption textOptions alignment and effects for the text. Default: None. Please note text is automatically aligned to the LEFT
-- change the code here
-- @tparam number textScale Scale factor for the text.
-- @tparam Color textColor Color of the text.
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
-- Adds an image entry to the specified page for the diary.
-- @tparam int pageIndex page number to add the image entry to.
-- @tparam Objects.ObjID objectIDbg Object ID for the image entry sprite.
-- @tparam number spriteID SpriteID from the specified object for the image entry.
-- @tparam Color color Color of image entry.
-- @tparam number posX,X position of the image entry in screen percent (0-100).
-- @tparam number posY,Y position of the image entry in screen percent (0-100).
-- @tparam number rot rotation of the image entry (0-360).
-- @tparam number scaleX, X Scaling factor for the image entry.
-- @tparam number scaleY, Y Scaling factor for the image entry.
-- @tparam View.AlignMode alignMode Alignment for the image entry.
-- @tparam View.ScaleMode scaleMode Scaling for the image entry.
-- @tparam Effects.BlendID blendMode Blending modes for the image entry.
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
-- @tparam Objects.ObjID objectIDbg Object ID for the diary's background.
-- @tparam number spriteIDbg SpriteID from the specified object for the diary's background.
-- @tparam Color colorbg Color of diary's background.
-- @tparam number posY, X position of the diary's background in screen percent (0-100).
-- @tparam number posX, Y position of the diary's background in screen percent (0-100).
-- @tparam float rotBG rotation of the diary's background sprite (0-360).
-- @tparam number scaleX, X Scaling factor for the diary's background.
-- @tparam number scaleY, Y Scaling factor for the diary's background.
-- @tparam View.AlignMode alignModebg Alignment for the diary's background.
-- @tparam View.ScaleMode scaleModebg Scaling for the diary's background.
-- @tparam Effects.BlendID blendModebg Blending modes for the diary's background.
function CustomDiary:addBackground(objectIDbg, spriteIDbg, colorbg, posX, posY, rot, scaleX, scaleY, alignMode, scaleMode, blendMode, alpha)
    if GameVars.Engine.Diaries[self.Name] then
        GameVars.Engine.Diaries[self.Name].Background.ObjectIDbg	    = objectIDbg
	    GameVars.Engine.Diaries[self.Name].Background.SpriteIDbg	    = spriteIDbg
	    GameVars.Engine.Diaries[self.Name].Background.ColorBG		    = colorbg
	    GameVars.Engine.Diaries[self.Name].Background.PosX			    = posX
	    GameVars.Engine.Diaries[self.Name].Background.PosY			    = posY
	    GameVars.Engine.Diaries[self.Name].Background.Rot			    = rot
	    GameVars.Engine.Diaries[self.Name].Background.ScaleX		    = scaleX
	    GameVars.Engine.Diaries[self.Name].Background.ScaleY		    = scaleY
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
-- @tparam Objects.ObjID objectIDbg Object ID for the notification icon.
-- @tparam number spriteID SpriteID from the specified object for the notification icon.
-- @tparam Color color Color of notification icon.
-- @tparam number posX,X position of the notification icon in screen percent (0-100).
-- @tparam number posY,Y position of the notification icon in screen percent (0-100).
-- @tparam number rot rotation of the notification icon (0-360).
-- @tparam number scaleX, X Scaling factor for the notification icon.
-- @tparam number scaleY, Y Scaling factor for the notification icon.
-- @tparam View.AlignMode alignMode Alignment for the notification icon.
-- @tparam View.ScaleMode scaleMode Scaling for the notification icon.
-- @tparam Effects.BlendID blendMode Blending modes for the notification icon.
-- @tparam Sound notificationSound Sound to play with notification icon.
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
-- @tparam int type Specifies the format for page numbers (1 or 2). 1: Displays only the current page number. 2: Formats the page number as: [Prefix][CurrentPage][Separator][UnlockedPages].
-- @tparam string prefix Prefix to be added for type 2 of page numbers.
-- @tparam string separator Separator to be added for type 2 of page numbers.
-- @tparam number textPosX X position of the page numbers.
-- @tparam number textPosX Y position of the page numbers.
-- @tparam Strings.DisplayStringOption textOptions alignment and effects for the text. Default: None. Please note text is automatically aligned to the LEFT
-- change the code here
-- @tparam number textScale Scale factor for the page numbers.
-- @tparam Color textColor Color of the page numbers.
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
-- Clears settings for the page numbers for the specified diary.
function CustomDiary:clearPageNumbers()
    if GameVars.Engine.Diaries[self.Name] then
        GameVars.Engine.Diaries[self.Name].PageNumbers = nil
		print("Page Numbers cleared")
    end
end

---
-- Customizes the controls bar for the diary.
-- @tparam number textPosX X position of the page numbers.
-- @tparam number textPosX Y position of the page numbers.
-- @tparam Strings.DisplayStringOption textOptions alignment and effects for the text. Default: None. Please note text is automatically aligned to the LEFT
-- change the code here
-- @tparam number textScale Scale factor for the page numbers.
-- @tparam Color textColor Color of the page numbers.
function CustomDiary:customizeControls(textX, textY, textAlignment, textEffects, textScale, textColor)
    if GameVars.Engine.Diaries[self.Name] then
        GameVars.Engine.Diaries[self.Name].Controls.text1           = "Space: Play Voice Note"
        GameVars.Engine.Diaries[self.Name].Controls.text2           = "Left Key: Previous Page" 
        GameVars.Engine.Diaries[self.Name].Controls.text3           = "Right Key: Next Page"
        GameVars.Engine.Diaries[self.Name].Controls.text4           = "Esc: Back"
        GameVars.Engine.Diaries[self.Name].Controls.textX           = textX
        GameVars.Engine.Diaries[self.Name].Controls.textY           = textY
        GameVars.Engine.Diaries[self.Name].Controls.textAlignment   = textAlignment
        GameVars.Engine.Diaries[self.Name].Controls.textEffects	    = textEffects
        GameVars.Engine.Diaries[self.Name].Controls.textScale		= textScale
        GameVars.Engine.Diaries[self.Name].Controls.textColor		= textColor
        
        print("Controls updated")
    end
end

---
-- Customizes the display text for controls for specified diary for the diary.
-- @tparam string1 prefix Prefix to be added for type 2 of page numbers.
-- @tparam string2 separator Separator to be added for type 2 of page numbers.
-- @tparam string3 separator Separator to be added for type 2 of page numbers.
-- @tparam string4 separator Separator to be added for type 2 of page numbers.
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
---
-- Imports diary entries from an external file.
-- @tparam string fileName Name of file without extension to import the diary entries from.
function CustomDiary:importData(filename)

    local diaryData = require(filename)

    for pageIndex, page in ipairs(diaryData.Pages) do
        for _, image in ipairs(page.Image) do
            self:addImageEntry(pageIndex, image.objectIDbg, image.spriteIDbg, image.colorBG, image.posX, image.posY, image.rot, image.scaleX, image.scaleY, image.alignMode, image.scaleMode, image.blendMode)
        end
        
        for _, text in ipairs(page.Text) do
            self:addTextEntry(pageIndex, text.text, text.textX, text.textY, text.textAlignment, text.textEffects, text.textScale, text.textColor)
        end
    end
    print("External file imported")
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

        local normalAlpha = (diary.CurrentAlpha/255)
        local normalAlphaEntry = (diary.EntryCurrentAlpha/255)

        if textEntries or imageEntries then
            -- Draw Diary sprite
            local dAlpha = math.min(diary.CurrentAlpha, diary.Alpha)
            local dColor = TEN.Color(diary.ColorBG.r, diary.ColorBG.g, diary.ColorBG.b, dAlpha)
            local dPos = TEN.Vec2(diary.PosX, diary.PosY)
            local dScale = TEN.Vec2(diary.ScaleX, diary.ScaleY)
            local dSprite = TEN.DisplaySprite(diary.ObjectIDbg, diary.SpriteIDbg, dPos, diary.Rot, dScale, dColor)
            dSprite:Draw(1, diary.AlignMode, diary.ScaleMode, diary.BlendMode)

            -- Draw Background Image
            if diary.Background and next(diary.Background) then
                local bgAlpha = math.min(diary.CurrentAlpha, diary.Background.Alpha)
                local bgColor = TEN.Color(diary.Background.ColorBG.r, diary.Background.ColorBG.g, diary.Background.ColorBG.b, bgAlpha)
                local bgPos = TEN.Vec2(diary.Background.PosX, diary.Background.PosY)
                local bgScale = TEN.Vec2(diary.Background.ScaleX, diary.Background.ScaleY)
                local bgSprite = TEN.DisplaySprite(diary.Background.ObjectIDbg, diary.Background.SpriteIDbg, bgPos, diary.Background.Rot, bgScale, bgColor)
                bgSprite:Draw(0, diary.Background.AlignMode, diary.Background.ScaleMode, diary.Background.BlendMode)
            end

            if diary.Controls.textX then
                --draw accent bars
                -- local bgColorC = TEN.Color(diary.ColorBG.r, diary.ColorBG.g,diary.ColorBG.b,diary.CurrentAlpha)
                -- local bgPosC = TEN.Vec2(diary.PosX, diary.PosY)
                -- local bgScaleC = TEN.Vec2(diary.ScaleX, diary.ScaleY)
                -- local bgSpriteC = TEN.DisplaySprite(diary.ObjectIDbg, diary.SpriteIDbg, bgPos, diary.Rot, bgScale, bgColor)
                -- bgSprite:Draw(3, diary.AlignMode, diary.ScaleMode, diary.BlendMode)

                --Controls
                
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

                local controlsText = LevelFuncs.Engine.Node.GenerateString(alignedText, diary.Controls.textX, diary.Controls.textY, diary.Controls.textScale, diary.Controls.textAlignment, diary.Controls.textEffects, diary.Controls.textColor, normalAlpha)
                ShowString(controlsText, 1 / 30)
            end


            
            --Draw Page Numbers
            if diary.PageNumbers and next(diary.PageNumbers) then
                
                local entry = diary.PageNumbers
                local pageNumbers = tostring(currentIndex)
                if entry.type == 2 then
                    pageNumbers = entry.prefix .. currentIndex  .. entry.separator .. diary.UnlockedPages
                end

                local pageNumberText = LevelFuncs.Engine.Node.GenerateString(pageNumbers, entry.textX, entry.textY, entry.textScale, entry.textAlignment, entry.textEffects, entry.textColor, normalAlpha)
                ShowString(pageNumberText, 1 / 30)

            end

            -- Draw entries based on type
            if textEntries then
                for _, entry in ipairs(textEntries) do
                        
                        local diaryText = LevelFuncs.Engine.Node.GenerateString(entry.text, entry.textX, entry.textY, entry.textScale, entry.textAlignment, entry.textEffects, entry.textColor, normalAlphaEntry)
                        ShowString(diaryText, 1 / 30)
                end
            end

            if imageEntries then
                for _, entry in ipairs(imageEntries) do
                    local entryColor = TEN.Color(entry.color.r,entry.color.g,entry.color.b,diary.EntryCurrentAlpha)
                    local entryPos = TEN.Vec2(entry.posX, entry.posY)
                    local entryScale = TEN.Vec2(entry.scaleX, entry.scaleY)
                    local entrySprite = TEN.DisplaySprite(entry.objectID, entry.spriteID, entryPos, entry.rot, entryScale, entryColor)
                
                    entrySprite:Draw(2, entry.alignMode, entry.scaleMode, entry.blendMode)
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

-- !Ignore
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