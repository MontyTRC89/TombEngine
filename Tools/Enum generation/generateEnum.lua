return function(enumName, enumIncludes, enumDesc, constantsDesc, tableDesc, decl, footer, enumFile, outputFile)
	local enum = {}
	local date = os.date("*t")
	local dateStr = date.day .. "/" .. date.month .. "/" .. date.year
	enum.header = [==[
	#pragma once

	// Last generated on ]==] .. dateStr .. [==[.
	// Please do not manually modify.

	]==] .. enumIncludes ..

	"\n/***\n" .. enumDesc .. "\n@classmod " .. enumName .. [==[

	@pragma nostrip
	*/
	]==]

	local f = io.open(enumFile, "r")
	local str = f:read()
	enum.vals = {}
	while (str) do
		str:gsub("(%g*) (%g*)", function(one, two) enum.vals[#enum.vals+1] = {one, two} end)
		str = f:read()
	end
	io.close(f)

	local doc = [===[
	/*** Constants.

	]===] .. constantsDesc .. "\n\n"
	local docFormat = "\t%s  \n"
	for i, v in ipairs(enum.vals) do
		doc = doc .. string.format(docFormat, v[1])
	end
	doc = doc .. [===[

	@section ]===] .. enumName .. [===[

	*/

	/*** ]===] .. tableDesc .. [===[

	@table CONSTANT_STRING_HERE
	*/

	]===]

	local format = "\t{\"%s\", %s},\n"
	local oStr = ""
	for i, v in ipairs(enum.vals) do
		oStr = oStr .. string.format(format, v[1], v[2])
	end

	-- remove last newline and comma
	oStr = oStr:sub(1, -3)

	f = io.open(outputFile, "w")
	f:write(enum.header, "\n", doc, "\n", decl, "\n",oStr, "\n", footer)
	io.close(f)
	print("written to " .. outputFile)
end

