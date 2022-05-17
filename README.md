# TombEngine 

In 2000, Core gave us a great gift of a level editor where people can create their own custom levels based on TR4.
Over the decades, the engine was upgraded massively via TREP and TRNG:
TREP (Tomb Raider Engine Patcher) is a tool that allows you to modify the exe to expand certain limits and implement new features.
TRNG builds upon TREP and gives a lot of new features including its own scripting language and even more extended limits.

However, TRNG is a closed-source and essentially an abandonware. No one is able to fix well-known bugs in TRNG and implement new features, 
although you can still implement with plugin (with C++ knowledge) however it is not user-friendly (you need to know C++) and it is poorly documented.

TEN (TombEngine) is supposed to be a new engine which is (most importantly) open-source, removes limits, and fixes bugs from the original game,
it also provides support for Lua (a programming language), all objects from TR1-5, and many more exciting features and gameplay functionalities such as: corner shimmying or enlarged 2D map (where you can build a massive level).

# To compile TombEngine

In order to compile TombEngine, make sure you have:
- Visual Studio
- TombEditor (if you would like to create and test levels)

Steps:
1) Clone the repo to your Github Desktop
2) launch TombEngine.sln and compile
3) Once compiled, make a separate folder and copy everything from the Build folder and also copy the scripts folder to your TombEngine directory (make sure you have data files aswell)
  3.1) Windows will also complain about missing dll files (Bass etc) in that case copy the dlls from the Libs folder
4) Once that is done, you should now be able to go to TombEditor, make a simple level, compile and run it with TombEngine
