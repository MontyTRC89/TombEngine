local parseFile = require('xmllpegparser').parseFile

local args = {...}
local helpStr = [[Usage: lua generateobjects.lua CATALOGPATH

This tool reads the TrCatalog.xml file whose path is given as input and creates game_object_ids.h (which defines GAME_OBJECT_ID enum used by TEN) and ObjectIDs.h (which creates constants and documentation for the API).

To work properly, this script must be in the folder TombEngine/Tools/Enum generation.]]

if not args[1] or args[1] == "--help" then
	print(helpStr)
	return
end

local srcFile = args[1]

local outputFileList = [[..\..\TombEngine\Objects\game_object_ids.h]]
local outputFileIDs = [[..\..\TombEngine\Scripting\Internal\TEN\Objects\ObjectIDs.h]]

local doc, err = parseFile(srcFile)
local theTab = doc.children[1]

local editorNames = {}
local engineNames = {}

local maxID = 0

function printelem(path, index, e, prefix)
	prefix = prefix or ''
	if e.tag and path[index] then
		local tag = path[index]
		if e.tag == tag.name then
			local attrsMatch = not tag.attrs

			if not attrsMatch then
				attrsMatch = true
				for k, v in pairs(tag.attrs) do
					if e.attrs[k] ~= v then
						attrsMatch = false
					end
				end
			end

			if attrsMatch then
				if tag.process then
					tag.process(e)
				else
					for i, child in pairs(e.children) do
						printelem(path, index+1, child, prefix)
					end
				end
			end
		end
	end
end

function getObject(e)
	local idStr = e.attrs.id
	local id= tonumber(idStr)
	local name = e.attrs.name
	if idStr and name and not e.attrs.t5m then
		maxID = math.max(maxID, id)
		editorNames[id] = name
		engineNames[id] = "ID_" .. name
	end
end

function printdoc(doc)
	local path = {{name = "xml"}, {name = "game", attrs = {id = "TombEngine"}}, {name="moveables"}, {name="moveable", process = getObject}}
	local path2 = {{name = "xml"}, {name = "game", attrs = {id = "TombEngine"}}, {name="sprite_sequences"}, {name="sprite_sequence", process = getObject}}
	for i, child in pairs(doc.children) do
		printelem(path, 1, child, ' ')
		printelem(path2, 1, child, ' ')
	end
end

printdoc(doc)

local namePairs = {}

for i = 0, maxID do
	namePairs[i] = {editor = editorNames[i], engine = engineNames[i]}
	local p = namePairs[i]
end

local objlistH = [====[#pragma once

enum GAME_OBJECT_ID : short
{
	ID_NO_OBJECT = -1,
]====]

local lastIsNull = false
for i = 0, maxID do
	if not namePairs[i].engine then
		lastIsNull = true
	else
		if lastIsNull then objlistH = objlistH .. "\n" end
		objlistH = objlistH .. "\t" .. namePairs[i].engine
		if lastIsNull then
			objlistH = objlistH .. " = " .. i
			lastIsNull = false
		end
		objlistH = objlistH .. ",\n"
end
end

	objlistH = objlistH .. [====[
	ID_NUMBER_OBJECTS
};
]====]

local file = io.open(outputFileList, 'w')
file:write(objlistH)
io.close(file)

print("Written " .. outputFileList)


local objidsH = [=[#pragma once

// Last generated on ]=]

local enum = {}
local date = os.date("*t")

-- Get the date last generated (in DD/MM/YYYY)
local dateStr = date.day .. "/" .. date.month .. "/" .. date.year

objidsH = objidsH .. dateStr .. [=[


#include "game_object_ids.h"
#include <unordered_map>
#include <string>

/***
Constants for object IDs.
@enum Objects.ObjID
@pragma nostrip
*/

/*** Objects.ObjID constants.

The following constants are inside ObjID.

]=]


for i = 0, maxID do
	if namePairs[i].editor then
		objidsH = objidsH .. "\t" .. namePairs[i].editor .. "\n"
	end
end

objidsH = objidsH .. [=[
Table of constants.
@table Members
*/

/*** Objects.ObjID pickup constants.

The following ObjID members refer to pickups.

]=]

for i = 0, maxID do
	if namePairs[i].editor and namePairs[i].editor:find("_ITEM") then
		objidsH = objidsH .. "\t" .. namePairs[i].editor .. "\n"
	end
end

objidsH = objidsH .. [=[
Table of constants.
@table PickupConstants
*/

static const std::unordered_map<std::string, GAME_OBJECT_ID> kObjIDs {
]=]

for i = 0, maxID do
	if namePairs[i].engine then
		objidsH = objidsH .. "\t{ \"" .. namePairs[i].editor .. "\", " .. namePairs[i].engine .. " },\n"
	end
end
objidsH = objidsH:sub(1, -3)
objidsH = objidsH .. [=[

};
]=]

file, err = io.open(outputFileIDs, 'w')

if not file then
	print(err)
else
	file:write(objidsH)
	io.close(file)
end

print("Written " .. outputFileIDs)
