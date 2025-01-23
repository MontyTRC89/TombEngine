# TombEngine 

![Logo](https://github.com/MontyTRC89/TombEngine/blob/7c50d26ca898c74978336d41e16ce3ce0c8ecacd/TEN%20logo.png)

TombEngine (TEN) is an open-source, custom level engine which aims to abolish limits and fix bugs of the classic Tomb Raider games, introduce new features while refining old ones, and provide user-friendly level creation process. Current support includes:
- Lua (as the native scripting language)
- Many objects from the original series (1-5)
- Support for high framerate, antialiasing, mipmapping and SSAO
- Full diagonal geometry support
- Uncapped map size
- A streamlined player control scheme.

If you would like to participate in TEN discussion with other TEN devs whether it is contributing, bugs or general discussion, then join this discord server: https://discord.gg/h5tUYFmres

Tomb Engine should be used in conjuction with Tomb Editor. Tomb Editor is also open source written in C#, you can find the repository here: https://github.com/MontyTRC89/Tomb-Editor

# Compiling TombEngine
To compile TEN, ensure you have installed:
- Microsoft Visual Studio 2022 Community Edition.
- Tomb Editor (if you would like to create and test levels).

Steps:
1) Clone this repository: https://github.com/microsoft/vcpkg with your GitHub Desktop.
2) Use bootstrap-vcpkg.bat and wait until vcpkg.exe is done.
3) Register vcpkg folder in your PATH, use this if you don't know how to: https://www.architectryan.com/2018/03/17/add-to-the-path-on-windows-10/
4) Clone TombEngine repository to your GitHub Desktop.
5) Use vcpkg_install_libraries.bat and wait until it finish.
6) Launch TombEngine.sln and compile.
7) Once compiled, you need to use the Game folder as assets folder.
8) Copy everything inside the Build/bin/Win32 or x64/Debug or Release/ folder to the Game directory.
9) Copy the Scripts folder to the Game directory.
10) Ensure you have the necessary level data and textures files as well.
11) In the case Windows warns about missing DLLs, (bass.dll, etc.) copy the missing DLL files found inside the Libs folder to your main TEN directory (vcpkg copy them auto).

# Disclaimer
This is a community project which is not affiliated with Core Design, Eidos Interactive, or Embracer Group AB. Tomb Raider is a registered trademark of Embracer Group AB. TombEngine is not be sold. The code is open-source to encourage contributions and to be used for study purposes. We are not responsible for illegal uses of this source code. This source code is released as-is and continues to be maintained by non-paid contributors in their free time.

