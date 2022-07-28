# TombEngine 

In the year 2000, Core Design granted us a great gift: their TR4-based Level Editor, which allowed people to create custom levels. It was, unfortunately, quite limited, hence why over the decades it was upgraded massively with fan projects such as Tomb Raider Engine Patcher (TREP) and Tomb Raider Next Generation (TRNG).
- TREP was a tool which allowed modification of the executable to expand certain limits and implement new features.
- TRNG built upon TREP and provided many new tools, including a scripting language, expanding even more limits with its own .DLL.

Unfortunately, TRNG's toolset is poorly documented and not user-friendly; the program remains closed-source to this day and is in all practicality an abandonware. As a direct consequence, no one is able to fix the countless well-known bugs and issues extant in TRNG, rendering implementation of new features is impossible without an in-depth knowledge of C++ plugin creation and a solid understanding of the classic Tomb Raider engine's many idiosyncrasies.

TombEngine (TEN) is a new, open-source engine which aims to abolish all limits, fix bugs from the original games, introduce new features while refining old ones, and provide for a refined, user-friendly level creation process. Current support includes:
- Lua (as the native scripting language)
- All objects from the classic series (1-5)
- Many more exciting gameplay functionalities such as diagonal shimmying and expanded crawlspace flexibility
- An enlarged 2D map, allowing for the creation of massive levels (imagine one big level may previously have been split into five!)
- A streamlined player control scheme.

TEN is based on the Tomb Raider: Chronicles engine of the classic era and continues to build upon and replace its systems to modernize ...

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

# Disclaimer
We do not and have never worked for Core Design, Eidos Interactive, or Square Enix. This is a hobby project. Tomb Raider is a registered trademark of Square Enix; TombEngine is not be sold. The code is open-source to encourage contributions and to be used for study purposes. We are not responsible for illegal uses of this source code. This source code is released as-is and continues to be maintained by non-paid contributors in their free time.

# Credit List

## Developers

- MontyTRC (Project Leader)

- ChocolateFan (general coding)
- Gancian (general coding)
- Krystian (general coding)
- Kubsy (Some cleanups and fixes)
- Lwmte (sound refactoring, general coding, code cleanups, bug fixing)
- Moooonyeah (Jumanji) (entity decompilation)
- Raildex (renderer refactoring, particle coding, general coding) 
- RicardoLuis0 (general coding)
- Sezz (player state refactoring, general coding, code cleanups, bug fixing, assets)
- Squidshire (Hispidence) (Lua implementation, bug fixing)
- Stranger1992 (sound asset refactoring and organisation, assets)
- TokyoSU (entity and vehicle decompilation)
- Troye (general coding, refactoring)
- WolfCheese (general coding)

## Testers
- Adngel
- Caesum
- Dustie
- JoeyQuint
- Kamillos
- Kubsy
- LGG_PRODUCTION
- Lore
- RemRem
- Stranger1992
- WolfCheese

## Assets and Miscellaneous

### Animations
- SrDanielPonces (Diagonal shimmy transitions)
- Krystian (Flexibility crawlspace, slope climbing animations)
- Sezz (Additional Animations for player state refactoring) 

### TombEngine Wiki tutorials 
- Kubsy (Lua Basics tutorial)
- Stranger1992 (Sound Setups)
- Wolfcheese (Lua puzzles setups)
