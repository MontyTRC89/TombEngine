local generateenum = require("generateenum")

local enumName = "Objects.StaticID"
local enumIncludes = [==[
#include <unordered_map>
#include <string>
]==]

local enumDesc = "Constants for static IDs."
local constantsDesc = "The following constants are inside StaticID."
local tableDesc = "Table of constants."

local decl = "static const std::unordered_map<std::string, int> kStaticIDs {"
local footer = [==[
};
]==]

local enumFile = "StaticIDs.txt"
local outputFile = "StaticIDs.h"

generateenum(enumName, enumIncludes, enumDesc, constantsDesc, tableDesc, decl, footer, enumFile, outputFile)

