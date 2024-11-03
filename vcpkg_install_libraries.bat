@echo off
title VCpkg Library Installer
color 0

echo "========================================"
echo "        This batch will install         "
echo "     VCpkg libraries for TombEngine.    "
echo "========================================"
echo.
timeout /t 1 /nobreak > NUL

echo "========================================"
echo "        Installing x86 libraries.       "
echo "========================================"
timeout /t 1 /nobreak > NUL

echo Installing DirectX Tool Kit.
timeout /t 1 /nobreak > NUL
vcpkg install directxtk[core,tools]:x86-windows

echo Installing FlatBuffers.
timeout /t 1 /nobreak > NUL
vcpkg install flatbuffers[core]:x86-windows

echo Installing Lua.
timeout /t 1 /nobreak > NUL
vcpkg install lua[core,cpp,tools]:x86-windows

echo Installing OIS.
timeout /t 1 /nobreak > NUL
vcpkg install ois[core]:x86-windows

echo Installing Sol2 (Lua module).
timeout /t 1 /nobreak > NUL
vcpkg install sol2[core]:x86-windows

echo Installing spdlog.
timeout /t 1 /nobreak > NUL
vcpkg install spdlog[core]:x86-windows

echo Installing zlib.
timeout /t 1 /nobreak > NUL
vcpkg install zlib:x86-windows

echo "========================================"
echo "        Installing x64 libraries.       "
echo "========================================"
timeout /t 1 /nobreak > NUL

echo Installing DirectX Tool Kit.
timeout /t 1 /nobreak > NUL
vcpkg install directxtk[core,tools]:x64-windows

echo Installing FlatBuffers.
timeout /t 1 /nobreak > NUL
vcpkg install flatbuffers[core]:x64-windows

echo Installing Lua.
timeout /t 1 /nobreak > NUL
vcpkg install lua[core,cpp,tools]:x64-windows

echo Installing OIS.
timeout /t 1 /nobreak > NUL
vcpkg install ois[core]:x64-windows

echo Installing Sol2 (Lua module).
timeout /t 1 /nobreak > NUL
vcpkg install sol2[core]:x64-windows

echo Installing spdlog.
timeout /t 1 /nobreak > NUL
vcpkg install spdlog[core]:x64-windows

echo Installing zlib.
timeout /t 1 /nobreak > NUL
vcpkg install zlib:x64-windows

echo "========================================"
echo " Updating libraries to latest versions. "
echo "========================================"
timeout /t 1 /nobreak > NUL
vcpkg upgrade --no-dry-run

echo Installing integration with Visual Studio.
vcpkg integrate install

echo Creating the game directory.
mkdir Game
mkdir Game\Audio
mkdir Game\Bin
mkdir Game\Bin\x64
mkdir Game\Bin\x86
mkdir Game\Bin\x64\Debug
mkdir Game\Bin\x64\Release
mkdir Game\Bin\x86\Debug
mkdir Game\Bin\x86\Release
mkdir Game\Data
mkdir Game\Scripts
mkdir Game\Scripts\Engine
mkdir Game\Scripts\Levels
mkdir Game\Shaders
mkdir Game\Textures

pause
