# TombEngine 

In the year 2000, Core Design gave us a great gift: level editor where people can create their own custom levels based on TR4 engine. Unfortunately, it was quite limited, hence over the decades, the engine was upgraded massively via TREP and TRNG:
- TREP (Tomb Raider Engine Patcher) is a tool that allows you to modify the exe to expand certain limits and implement new features.
- TRNG (Tomb Raider Next Generation) builds upon TREP and gives a lot of new features including its own scripting language and even more extended limits and its own DLL.
Unfortunately, TRNG is closed-source, and essentially an abandonware program. No one can fix well-known bugs in TRNG, and implement new features, although you can still implement some features with plugin (with C++ knowledge) however it is not user-friendly (you need to know C++ which is quite complex) and it is poorly documented.
TEN (TombEngine) is supposed to be a new engine which is (most importantly) open-source, removes limits, and fixes bugs from the original game,
it also provides support for Lua (a programming language), all objects from TR1-5, and many more exciting features and gameplay functionalities such as corner shimmying or an enlarged 2D map which lets you build a massive level (Imagine 5 split levels into 1 big level).
TEN (TombEngine) is based on TR5 engine which has been gradually built upon that.
# To compile TombEngine
In order to compile TombEngine, make sure you have:
- Microsoft Visual Studio 
- TombEditor (if you would like to create and test levels)
Steps:
1) Clone the repo to your Github Desktop
2) Launch TombEngine.sln and compile
3) Once compiled, make a separate folder and copy everything from the Build folder (before compilation there will be nothing in that folder) and also copy the scripts folder to your TombEngine executable directory (make sure you have data, texture files as well)
4) Windows will also complain about missing DLL files (Bass etc) in that case copy the missing DLL(s) from the Libs folder
Visual Studio might also complain about the NuGet packages, to fix that:
1) Delete the packages folder
2) Go back to Visual studio
3) Right-click on solution explorer and restore Nuget Packages
4) Compile again and once that is done, you should now be able to go to TombEditor, make a simple level, compile and run it with TombEngine.

## Developers

- MontyTRC (Project Leader)

- ChocolateFan (General coding)
- Gancian (general coding)
- Krys (general coding)
- Lwmte (general coding, code cleanups, and bug fixing)
- Moooonyeah (Jumanji) (entity decompilation)
- Raildex (renderer refactoring & general coding) 
- RicardoLuis0 (general coding)
- Sezz (state refactoring, general coding and bug fixing)
- Squidshire (Hispidence) (lua implementation and bug fixing)
- Stranger1992 (Sounds Project)
- TokyoSU (Decompiling Vehicles & Entities)
- Troye (General coding, refactoring)
- WolfCheese (General coding)

## Testers
- Caesum
- Dustie
- JoeyQuint
- Kamillos
- Kubsy
- Lgg_productions
- Lore
- RemRem
- Stranger1992
- WolfCheese
