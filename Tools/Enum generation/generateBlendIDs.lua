local generateheader = require("generateheader")

local enumName = "Effects.BlendID"
local enumIncludes = [==[
#include "Renderer11Enums.h"
#include <unordered_map>
#include <string>
]==]

local enumDesc = "Constants for blend mode IDs."
local constantsDesc = "The following constants are inside BlendID."
local tableDesc = "Table of constants to use when specifying a blend mode (e.g. when using particles)."

local decl = "static const std::unordered_map<std::string, BLEND_MODES> kBlendIDs {"
local footer = [==[
};
]==]

local enumFile = "blendIDs.txt"
local outputFile = "BlendIDs.h"

generateheader(enumName, enumIncludes, enumDesc, constantsDesc, tableDesc, decl, footer, enumFile, outputFile)

