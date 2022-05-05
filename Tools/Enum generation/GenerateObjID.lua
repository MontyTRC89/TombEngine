local generateheader = require("generateheader")

local enumName = "Objects.ObjID"
local enumIncludes = [==[
#include "objectslist.h"
#include <unordered_map>
#include <string>
]==]

local enumDesc = "Constants for object IDs."
local constantsDesc = "The following constants are inside ObjID."
local tableDesc = "Table of constants."

local decl = "static const std::unordered_map<std::string, GAME_OBJECT_ID> kObjIDs {"
local footer = [==[
};
]==]

local enumFile = "objIDs.txt"
local outputFile = "TR5Main/Scripting/ObjectIDs.h"

generateheader(enumName, enumIncludes, enumDesc, constantsDesc, tableDesc, decl, footer, enumFile, outputFile)

