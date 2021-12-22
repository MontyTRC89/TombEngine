-- Place in this LUA script all the strings of your game
-- Please follow the rules for IDs in the comments before each section

local strings = {
					new_game	=	{"New Game", "Nouvelle partie", "Nuova partita", "Neues Spiel", "Nueva partida"},
					load_game	=	{"Load Game", "Chargement", "Carica partita", "Spiel laden", "Cargar partida"}	
				}
GameFlow:SetStrings(strings)
local languages = {"English", "Français", "Italiano", "Deutsch", "Español"} 
GameFlow:SetLanguageNames(languages)

-- Global engine string, please don't change the IDs because internally TEN machine code refers to strings by fixed ID
GameFlow.strings[1] = "Game";
GameFlow.strings[2] = "Lara's home";
GameFlow.strings[3] = "Controls";
GameFlow.strings[4] = "Display";
GameFlow.strings[5] = "Sound";
GameFlow.strings[6] = "New game";
GameFlow.strings[7] = "Load game";
GameFlow.strings[8] = "Save game";
GameFlow.strings[9] = "Exit game";
GameFlow.strings[10] = "Exit to title";
GameFlow.strings[11] = "Uzi";
GameFlow.strings[12] = "Pistols";
GameFlow.strings[13] = "Shotgun";
GameFlow.strings[14] = "Revolver";
GameFlow.strings[15] = "Revolver + LaserSight";
GameFlow.strings[16] = "Desert Eagle";
GameFlow.strings[17] = "Desert Eagle + LaserSight";
GameFlow.strings[18] = "Desert Eagle Ammo";
GameFlow.strings[19] = "HK Gun";
GameFlow.strings[20] = "HK Gun (Silenced)";
GameFlow.strings[21] = "Shotgun Normal Ammo";
GameFlow.strings[22] = "Shotgun Wideshot Ammo";
GameFlow.strings[23] = "HK Sniper Mode";
GameFlow.strings[24] = "HK Burst Mode";
GameFlow.strings[25] = "HK Rapid Mode";
GameFlow.strings[26] = "HK Ammo";
GameFlow.strings[27] = "Revolver Ammo";
GameFlow.strings[28] = "Uzi Ammo";
GameFlow.strings[29] = "Pistol Ammo";
GameFlow.strings[30] = "LaserSight";
GameFlow.strings[31] = "Silencer";
GameFlow.strings[32] = "Large Medipack";
GameFlow.strings[33] = "Small Medipack";
GameFlow.strings[34] = "Binoculars";
GameFlow.strings[35] = "Headset";
GameFlow.strings[36] = "Flares";
GameFlow.strings[37] = "Timex-TMX";
GameFlow.strings[38] = "Crowbar";
GameFlow.strings[39] = "Use";
GameFlow.strings[40] = "Combine";
GameFlow.strings[41] = "Separe";
GameFlow.strings[42] = "Choose ammo";
GameFlow.strings[43] = "Select level";
GameFlow.strings[44] = "%02d days %02d:%02d:%02d";
GameFlow.strings[45] = "Not saved";
GameFlow.strings[46] = "Grenade launcher";
GameFlow.strings[47] = "Grenade Normal Ammo";
GameFlow.strings[48] = "Grenade Poisoned Ammo";
GameFlow.strings[49] = "Grenade Explosive Ammo";
GameFlow.strings[50] = "Harpoon gun";
GameFlow.strings[51] = "Harpoon ammo";
GameFlow.strings[52] = "Rocket launcher";
GameFlow.strings[53] = "Rocket ammo";
GameFlow.strings[54] = "Crossbow";
GameFlow.strings[55] = "Crossbow + LaserSight";
GameFlow.strings[56] = "Crossbow Normal Ammo";
GameFlow.strings[57] = "Crossbow Poisoned Ammo";
GameFlow.strings[58] = "Crossbow Explosive Ammo";
GameFlow.strings[59] = "Diary";
GameFlow.strings[60] = "Enabled";
GameFlow.strings[61] = "Disabled";
GameFlow.strings[62] = "Music volume";
GameFlow.strings[63] = "SFX volume";
GameFlow.strings[64] = "Screen resolution";
GameFlow.strings[65] = "Dynamic shadows";
GameFlow.strings[66] = "Underwater caustics";
GameFlow.strings[67] = "Volumetric fog";
GameFlow.strings[68] = "Apply";
GameFlow.strings[69] = "Cancel";
GameFlow.strings[70] = "Enable sound";
GameFlow.strings[71] = "Special effects";
GameFlow.strings[75] = "Small Waterskin (Empty)"
GameFlow.strings[76] = "Large Waterskin (Empty)"
GameFlow.strings[78] = "Move forward"
GameFlow.strings[79] = "Move backward"
GameFlow.strings[80] = "Move left"
GameFlow.strings[81] = "Move right"
GameFlow.strings[82] = "Duck"
GameFlow.strings[83] = "Dash"
GameFlow.strings[84] = "Walk"
GameFlow.strings[85] = "Jump"
GameFlow.strings[86] = "Action"
GameFlow.strings[87] = "Draw weapon"
GameFlow.strings[88] = "Use flare"
GameFlow.strings[89] = "Look"
GameFlow.strings[90] = "Roll"
GameFlow.strings[91] = "Inventory"
GameFlow.strings[92] = "Step left"
GameFlow.strings[93] = "Step right"
GameFlow.strings[94] = "Items"
GameFlow.strings[95] = "Puzzles"

-- Level names (from 100 to 199)
GameFlow.strings[100] = "Title level";
GameFlow.strings[101] = "Strets of Rome";
GameFlow.strings[102] = "Trajan Markets";
GameFlow.strings[103] = "The Colosseum";
GameFlow.strings[104] = "The base";
GameFlow.strings[105] = "The submarine";
GameFlow.strings[106] = "Deepsea Dive";
GameFlow.strings[107] = "Sinking Submarine";
GameFlow.strings[108] = "Gallows Tree";
GameFlow.strings[109] = "Labyrinth";
GameFlow.strings[110] = "Old Mill";
GameFlow.strings[111] = "The 13th Floor";
GameFlow.strings[112] = "Escape with the Iris";
GameFlow.strings[113] = "Security breach";
GameFlow.strings[114] = "Red alert!";
GameFlow.strings[115] = "Test level";

-- Puzzle names (from 200 to 499)
GameFlow.strings[200] = "Bronze key";
GameFlow.strings[201] = "Golden key";

-- Examines strings (from 500 to 799, use \n sequence for new line)
GameFlow.strings[500] = "Examine test \n This is a new line of text \n This is another new line of text";
