@echo off
title VCpkg Library Installer
color 10

echo "========================================"
echo " This batch remove vcpkg library        "
echo "                      for TombEngine.   "
echo "========================================"
echo.
timeout /t 1 /nobreak > NUL

echo "========================================"
echo "      Starting with x86 libraries.      "
echo "========================================"
timeout /t 1 /nobreak > NUL

echo Removing DirectXTK.
timeout /t 1 /nobreak > NUL
vcpkg remove directxtk[core,tools]:x86-windows

echo Removing OIS.
timeout /t 1 /nobreak > NUL
vcpkg remove ois[core]:x86-windows

echo Removing FlatBuffers.
timeout /t 1 /nobreak > NUL
vcpkg remove flatbuffers[core]:x86-windows

echo Removing Spdlog.
timeout /t 1 /nobreak > NUL
vcpkg remove spdlog[core]:x86-windows

echo Removing Sol2 (Lua Module).
timeout /t 1 /nobreak > NUL
vcpkg remove sol2[core]:x86-windows

echo Removing Lua.
timeout /t 1 /nobreak > NUL
vcpkg remove lua[core,cpp,tools]:x86-windows

echo "========================================"
echo "        Now with x64 libraries.         "
echo "========================================"
timeout /t 1 /nobreak > NUL

echo Removing DirectXTK.
timeout /t 1 /nobreak > NUL
vcpkg remove directxtk[core,tools]:x64-windows

echo Removing OIS.
timeout /t 1 /nobreak > NUL
vcpkg remove ois[core]:x64-windows

echo Removing FlatBuffers.
timeout /t 1 /nobreak > NUL
vcpkg remove flatbuffers[core]:x64-windows

echo Removing Spdlog.
timeout /t 1 /nobreak > NUL
vcpkg remove spdlog[core]:x64-windows

echo Removing Sol2 (Lua Module).
timeout /t 1 /nobreak > NUL
vcpkg remove sol2[core]:x64-windows

echo Removing Lua.
timeout /t 1 /nobreak > NUL
vcpkg remove lua[core,cpp,tools]:x64-windows

pause
