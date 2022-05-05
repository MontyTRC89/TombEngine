--[[ This module is for generating header files which contain very large constant
 containers which hold string->value mappings intended to be exposed as
 read-only tables to the level designer.

 We also need to document these to the level designer. Obviously, having to
 remember to manually edit the documentation each time we add/remove/change
 a member of containers would be a total nightmare. This module aims to make
 this much easier by reading the mapping from a text file (this text file
 is the only file that should be manually edited) and generating both the
 container and LDoc-compatible comments.

 e.g. if we want to generate a header with an unordered_map that maps Lua
 strings to GAME_OBJECT_IDs, our text file would contain

LARA ID_LARA
LARA_EXTRA_ANIMS ID_LARA_EXTRA_ANIMS
PISTOLS_ANIM ID_PISTOLS_ANIM
UZI_ANIM ID_UZI_ANIM

etc.

Running it through the script (see GenerateObjID.lua for details) gives us:

static const std::unordered_map<std::string, GAME_OBJECT_ID> kObjIDs {
	{"LARA", ID_LARA},
	{"LARA_EXTRA_ANIMS", ID_LARA_EXTRA_ANIMS},
	{"PISTOLS_ANIM", ID_PISTOLS_ANIM},
	{"UZI_ANIM", ID_UZI_ANIM},

etc.
]]

return function(enumName, enumIncludes, enumDesc, constantsDesc, tableDesc, decl, footer, enumFile, outputFile)
	local enum = {}
	local date = os.date("*t")

	-- Get the date last generated (in DD/MM/YYYY)
	local dateStr = date.day .. "/" .. date.month .. "/" .. date.year

	-- First, make a string with the includes and a 'do not modify manually'
	-- warning. Then we make the first block of comments. We're using
	-- 'classmod' for now. This does result in the documentation calling it a
	-- 'Class', but we can live with that for the time being.
	--
	-- @pragma nostrip lets us choose whether a function should be documented
	-- as having dot notation or colon notation (e.g. MyClass.new() vs
	-- MyClass:new()). Without this, the docs would just show colon notation
	-- for every function.
	enum.header = [==[
#pragma once

// Last generated on ]==] .. dateStr .. [==[.

]==] .. enumIncludes ..

"\n/***\n" .. enumDesc .. "\n@enum " .. enumName .. [==[

@pragma nostrip
*/
]==]

	-- Open the input file and read the contents into our table of key-values
	local f = io.open(enumFile, "r")
	local str = f:read()
	enum.vals = {}
	while (str) do
		str:gsub("(%g*) (%g*)", function(one, two) enum.vals[#enum.vals+1] = {one, two} end)
		str = f:read()
	end
	io.close(f)

	-- This part of the documentation will appear over the list of the constants
	-- in the enum. We have to title this as "<EnumName> constants" instead of
	-- just "Constants" or LDoc will get confused and put the wrong descriptions
	-- in the wrong place. The full stop is required for ldoc to put a proper
	-- line break before the description.
	local doc = "/*** " .. enumName .. " constants.\n\n" .. constantsDesc .. "\n\n"

	-- We put a tab, then the item, then two spaces.
	-- The tab will cause LDoc to recognise this as a block of code, and
	-- the two spaces are so that LDoc's built-in markdown generator will
	-- put a line break between each item.
	local docFormat = "\t%s  \n"
	for i, v in ipairs(enum.vals) do
		doc = doc .. string.format(docFormat, v[1])
	end

	-- Next, we add a @section and a @table. The table is pretty redundant,
	-- but we need both - if LDoc sees a @section without a @table or a @function,
	-- it won't generate any documentation. Why? Because LDoc.
	-- We use CONSTANT_STRING_HERE because we can't have a nameless table.
	doc = doc .. [===[

@section ]===] .. enumName .. [===[

*/

/*** ]===] .. tableDesc .. [===[

@table CONSTANT_STRING_HERE
*/

]===]

	-- now we prepare to generate the C++ container (assumed to be an
	-- unordered_map)
	local format = "\t{\"%s\", %s},\n"
	local oStr = ""
	for i, v in ipairs(enum.vals) do
		oStr = oStr .. string.format(format, v[1], v[2])
	end

	-- remove last newline and comma
	oStr = oStr:sub(1, -3)

	f = io.open(outputFile, "w")
	-- finally, write it to the file
	f:write(enum.header, "\n", doc, "\n", decl, "\n",oStr, "\n", footer)
	io.close(f)
	print("written to " .. outputFile)
end

