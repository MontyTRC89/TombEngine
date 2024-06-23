# TombEngine 

![Logo](https://github.com/MontyTRC89/TombEngine/assets/80340234/f22c9ca9-7159-467f-b8ad-7bb32274a278)

In the year 2000, Core Design granted us a great gift: their TR4-based Level Editor, which allowed people to create custom levels. It was, unfortunately, quite limited, hence why over the decades it was upgraded massively with fan patcher projects such as Tomb Raider Engine Patcher (TREP) and Tomb Raider Next Generation (TRNG).

TombEngine (TEN) is a new, open-source engine which aims to abolish all limits, fix bugs from the original games, introduce new features while refining old ones, and provide for a refined, user-friendly level creation process. Current support includes:
- Lua (as the native scripting language)
- All objects from the classic series (1-5)
- Many more exciting gameplay functionalities such as diagonal shimmying and expanded crawlspace flexibility
- An enlarged 2D map, allowing for the creation of massive levels (imagine one big level may previously have been split into five!)
- A streamlined player control scheme.

If you would like to participate in TEN discussion with other TEN devs whether it is contributing, bugs or general discussion, then join this discord server: https://discord.gg/h5tUYFmres

Tomb Engine should be used in conjuction with Tomb Editor. Tomb Editor is also open source written in C#, you can find the repository here: https://github.com/MontyTRC89/Tomb-Editor

# Compiling TombEngine
To compile TEN, ensure you have installed:
- Microsoft Visual Studio 
- Tomb Editor (if you would like to create and test levels)

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

