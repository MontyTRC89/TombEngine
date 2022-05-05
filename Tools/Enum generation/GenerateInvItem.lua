local generateheader = require("generateheader")

local enumName = "Flow.InvItem"
local enumIncludes = [==[
#include "ItemEnumPair.h"
#include <unordered_map>
#include <string>
]==]

local enumDesc = "Constants for items that can be used with GetInvItem and SetInvItem."
local constantsDesc = "The following constants are inside InvItem."
local tableDesc = "Table of constants to use with GetInvItem and SetInvItem."

local decl = "static const std::unordered_map<std::string, ItemEnumPair> kInventorySlots {"
local footer = [==[
};
]==]

local enumFile = "inventorySlots"
local outputFile = "TR5Main/Scripting/InventorySlots.h"

generateheader(enumName, enumIncludes, enumDesc, constantsDesc, tableDesc, decl, footer, enumFile, outputFile)

