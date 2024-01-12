# TombEngine 

[Main project.](https://github.com/MontyTRC89/TombEngine/tree/master/TombEngine)

If you would like to participate in TEN discussion with other TEN devs whether it is contributing, bugs or general discussion, then join this discord server: https://discord.gg/h5tUYFmres

Tomb Engine should be used in conjuction with Tomb Editor. Tomb Editor is also open source written in C#, you can find the repository here: https://github.com/MontyTRC89/Tomb-Editor

# Compiling TombEngine
To compile TEN, ensure you have installed:
- Microsoft Visual Studio 
- TombEditor (if you would like to create and test levels)

Steps:
1) Clone the repository to your GitHub Desktop
2) Launch TombEngine.sln and compile
3) Once compiled, create a separate folder to serve as your main TEN directory
4) Copy everything inside the Build folder to the main TEN directory
5) Copy the Scripts folder to your main TEN directory
6) Ensure you have the necessary level data and texture files as well
7) In the case Windows warns about missing DLLs, (bass.dll, etc.) copy the missing DLL files found inside the Libs folder to your main TEN directory.

Visual Studio may also warn about NuGet packages. To fix:
1) Delete the Packages folder
2) Go back to Microsoft Visual Studio
3) Right-click on the TombEngine solution in the Solution Explorer tab and select "Restore NuGet Packages"
4) Compile again and once done, you should be able to compile a level with TombEditor and run it in TEN.

# About this Fork
Main goal is to implement the dual magnums alongside the revolver, regardless of which slots the animations will occupy. On a second time, I would like to reimplement FMVs and ring menus.

# Disclaimer
We do not and have never worked for Core Design, Eidos Interactive, or Square Enix. This is a hobby project. Tomb Raider is a registered trademark of Square Enix; TombEngine is not be sold. The code is open-source to encourage contributions and to be used for study purposes. We are not responsible for illegal uses of this source code. This source code is released as-is and continues to be maintained by non-paid contributors in their free time.

# Credit List

## Developers
- MontyTRC (Project Leader)

- Frjttr (magnums implementation) 
- Gancian (general coding)	
- Krystian (general coding)	
- Kubsy (Some cleanups and fixes)	
- l.m. (general coding, Lua enhancements, bug fixing)	
- Lwmte (sound refactoring, general coding, code cleanups, bug fixing)	
- Moooonyeah (Jumanji) (entity decompilation)	
- Raildex (renderer refactoring, particle coding, general coding) 	
- RicardoLuis0 (general coding)	
- Sezz (player state refactoring, general coding, code cleanups, bug fixing, assets)	
- Squidshire (Hispidence) (Lua implementation, bug fixing)	
- Stranger1992 (sound asset refactoring and organisation, assets)	
- TokyoSU (entity and vehicle decompilation)	
- Tomo (general coding, bug fixing)	
- Troye (general coding, refactoring)	
- WolfCheese (general coding)	

## Testers	
- Adngel	
- Caesum	
- Dustie
- Frjttr
- GeckoKid	
- JoeyQuint	
- Kamillos	
- Kubsy	
- LGG_PRODUCTION	
- Lore	
- RemRem	
- Stranger1992	
- WolfCheese	

## Assets and Miscellaneous	

- Geckokid (Sprites and test level creator).	

### Animations	
- SrDanielPonces (Diagonal shimmy transitions, backwards monkey swinging)	
- Krystian (Flexibility crawlspace, slope climbing animations)	
- Sezz (Additional Animations for player state refactoring) 	
- Naotheia (Underwater puzzle placement, crouch 180° turn, crawl 180° turn, water surface 180° turn)	
- JoeyQuint (Standing 180° turn, monkey swing 180° turn)	

### TombEngine Marketing 	
- Kubsy (Twitter and forum posts)	
- Stranger1992 (This website, Facebook, Instagram, Youtube and Twitch.
