-----
--- Utility - This molule is a collection of help functions. To use the functions within the scripts, the module must be called:
--	local Utility = require("Engine.Util")
-- @luautil Utility

local Utility = {}

local Type= require("Engine.Type")

--- Helper function for value comparisons.
-- @tparam any operand the value to be compared
-- @tparam any reference the value one wants to search for
-- @tparam int operator the type of comparison
--@advancedDesc
-- Operators:
--
-- 0 : If operand is equal to the reference
--
-- 1 : If operand is different from the reference
--
-- 2 : If operand is less the value
--
-- 3 : If operand is less or equal to the reference
--
-- 4 : If operand is greater the reference
--
-- 5 : If operand is greater or equal the reference
-- @treturn bool __true__ if comparison is true, __false__ if comparison is false
Utility.CompareValue = function(operand, reference, operator)
	local result = false

	-- Fix Lua-specific treatment of bools as non-numerical values
	if (operand == false) then operand = 0 end;
	if (operand == true) then operand = 1 end;
	if (reference == false) then reference = 0 end;
	if (reference == true) then reference = 1 end;

	if (operator == 0 and operand == reference) then result = true end
	if (operator == 1 and operand ~= reference) then result = true end
	if (operator == 2 and operand < reference) then result = true end
	if (operator == 3 and operand <= reference) then result = true end
	if (operator == 4 and operand > reference) then result = true end
	if (operator == 5 and operand >= reference) then result = true end
	return result
end

--- Smoothstep
-- @number source
-- @treturn float result
Utility.Smoothstep = function(source)
	if not Type.IsNumber(source) then
		TEN.Util.PrintLog("Error in Utility.Smoothstep() function: invalid source", TEN.Util.LogLevel.ERROR)
		return 0.0
	end
	source = math.max(0, math.min(1, source))
	return ((source ^ 3) * (source * (source * 6 - 15) + 10))
end

--- Linear interpolation. Calculates a number between two numbers at a specific increment.
-- @number val1 starting value
-- @number val2 final value
-- @number factor amount to interpolate between the two values
-- @treturn float result lerped value
Utility.Lerp = function(val1, val2, factor)
	if not (Type.IsNumber(val1) and Type.IsNumber(val2) and Type.IsNumber(factor)) then
		TEN.Util.PrintLog("Error in Utility.Lerp() function: invalid arguments", TEN.Util.LogLevel.ERROR)
		return 0.0
	end
	return val1 * (1 - factor) + val2 * factor
end

--- Counts the number of elements in a table. Nil values are not counted
-- @tparam table table table to be examined
-- @treturn int the length of the table
Utility.TableLength = function (table)
	if not Type.IsTable(table) then
		TEN.Util.PrintLog("Error in Utility.TableLength() function: invalid table", TEN.Util.LogLevel.ERROR)
		return 0
	end
	local count = 0
	for _ in pairs(table) do count = count + 1 end
	return count
end

--- Verify that two float numbers are equal based on a tolerance
-- @tparam float num1 the first float number
-- @tparam float num2 the second float number
-- @tparam[opt] float epsilon tolerance __Default: 0.00001__
-- @treturn bool __true__ if numbers are equal, __false__ if numbers are't equal
Utility.FloatEquals = function(num1, num2, epsilon)
    epsilon = epsilon or .00001
	if not (Type.IsNumber(num1) and Type.IsNumber(num2) and Type.IsNumber(epsilon)) then
		TEN.Util.PrintLog("Error in Utility.FloatEquals() function: invalid arguments", TEN.Util.LogLevel.ERROR)
		return false
	end
	return math.abs(num1 - num2) < epsilon
end

--- Split string using specified delimiter.
-- @tparam string inputStr the string to be split
-- @tparam string delimiter __Default: space__
-- @treturn table the collection of strings generated
Utility.SplitString = function(inputStr, delimiter)
	delimiter = delimiter or "%s"
	local t = {}
	if not (Type.IsString(inputStr) and Type.IsString(delimiter)) then
		TEN.Util.PrintLog("Error in Utility.SplitString() function: invalid arguments", TEN.Util.LogLevel.ERROR)
	else
		for str in string.gmatch(inputStr, "([^" .. delimiter .. "]+)") do
			table.insert(t, str)
		end
	end
	return t
end

-- Helper function to check time format for timer and stopwatch.
Utility.CheckTimeFormat = function (timerFormat)
	if Type.IsTable(timerFormat) then
        local validKeys = {hours = true, minutes = true, seconds = true, deciseconds = true}
        for k, v in pairs(timerFormat) do
        	if not validKeys[k] or type(v) ~= "boolean" then
				TEN.Util.PrintLog("Warning: Incorrect time format ", TEN.Util.LogLevel.ERROR)
				return false
			end
        end
		return timerFormat
	elseif Type.IsBoolean(timerFormat) then
		return timerFormat and {seconds = true} or timerFormat
	end
	TEN.Util.PrintLog("Warning: Incorrect time format ", TEN.Util.LogLevel.ERROR)
	return false
end

--- Generate time formatted string for Timer object
-- @tparam Time time time in game frames
-- @tparam ?table|bool timerFormat the string to be split. If a table is given, the remaining time will be shown as a string, formatted according to the values in the table. If true, the only seconds
--@advancedDesc
-- Examples of timerFormat:
--		-- hours:mins:secs.decisecs
--		local myTimeFormat1 = {hours = true, minutes = true, seconds = true, deciseconds = false}
--		
--		-- mins:secs.decisecs
--		local myTimeFormat1 = {minutes = true, seconds = true, deciseconds = false}
--		
--		-- mins:secs
--		local myTimeFormat2 = {minutes = true, seconds = true}
--		
--		-- secs.decisecs
--		local myTimeFormat3 = {seconds = true, deciseconds = true}
--		
--		-- secs
--		local myTimeFormat4 = {seconds = true}
-- @treturn string the formatted time
-- @usage
--  -- Example:
--	local Utility = require("Engine.Util")
--
--	local timerFormat = {seconds = true, deciseconds = true}
--	local time = TEN.Time(30*120)
--	local text = Utility.GenerateTimeFormattedString(time, timerFormat)
--	local pos = TEN.Vec2(TEN.Util.PercentToScreen(10, 10))
--	local str = TEN.Strings.DisplayString("Timer: " .. text, pos)
--	TEN.Strings.ShowString(str, 1)
Utility.GenerateTimeFormattedString = function (time, timerFormat)
    if not Type.IsTime(time) then
        TEN.Util.PrintLog("Error in Utility.GenerateTimeFormattedString() function: invalid time", TEN.Util.LogLevel.ERROR)
        return ""
    end
    timerFormat = Utility.CheckTimeFormat(timerFormat)

	if not timerFormat then
		return ""
	else
		local result = {}
		local index = 1
		if timerFormat.hours then
        	result[index] = string.format("%02d", time.h)
        	index = index + 1
    	end
    	if timerFormat.minutes then
        	result[index] = string.format("%02d", timerFormat.hours and time.m or (time.m + (60 * time.h)))
        	index = index + 1
    	end
    	if timerFormat.seconds then
        	result[index] = string.format("%02d", timerFormat.minutes and time.s or (time.s + (60 * time.m)))
        	index = index + 1
    	end
	local formattedString = table.concat(result, ":")

    	if timerFormat.deciseconds then
        	local deciseconds = string.sub(string.format("%02d", time.c), 1, -2)
		return (index == 1) and deciseconds or formattedString .. "." .. deciseconds
    	end
    	return formattedString
	end
end

Utility.ShortenTENCalls = function()
	TEN.Util.PrintLog("Util.ShortenTENCalls is deprecated; its functionality is now performed automatically by TombEngine.", TEN.Util.LogLevel.INFO)
end

return Utility
