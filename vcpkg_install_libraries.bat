@echo off
title VCpkg Library Installer
color 10

echo "========================================"
echo " This batch install vcpkg library       "
echo "                      for TombEngine.   "
echo "========================================"
echo.
timeout /t 1 /nobreak > NUL

echo "========================================"
echo "      Starting with x86 libraries.      "
echo "========================================"
timeout /t 1 /nobreak > NUL

echo Installing DirectXTK.
timeout /t 1 /nobreak > NUL
vcpkg install directxtk[core,tools]:x86-windows

echo Installing OIS.
timeout /t 1 /nobreak > NUL
vcpkg install ois[core]:x86-windows

echo Installing FlatBuffers.
timeout /t 1 /nobreak > NUL
vcpkg install flatbuffers[core]:x86-windows

echo Installing Spdlog.
timeout /t 1 /nobreak > NUL
vcpkg install spdlog[core]:x86-windows

echo Installing Sol2 (Lua Module).
timeout /t 1 /nobreak > NUL
vcpkg install sol2[core]:x86-windows

echo Installing Lua.
timeout /t 1 /nobreak > NUL
vcpkg install lua[core,cpp,tools]:x86-windows

echo "========================================"
echo "        Now with x64 libraries.         "
echo "========================================"
timeout /t 1 /nobreak > NUL

echo Installing DirectXTK.
timeout /t 1 /nobreak > NUL
vcpkg install directxtk[core,tools]:x64-windows

echo Installing OIS.
timeout /t 1 /nobreak > NUL
vcpkg install ois[core]:x64-windows

echo Installing FlatBuffers.
timeout /t 1 /nobreak > NUL
vcpkg install flatbuffers[core]:x64-windows

echo Installing Spdlog.
timeout /t 1 /nobreak > NUL
vcpkg install spdlog[core]:x64-windows

echo Installing Sol2 (Lua Module).
timeout /t 1 /nobreak > NUL
vcpkg install sol2[core]:x64-windows

echo Installing Lua.
timeout /t 1 /nobreak > NUL
vcpkg install lua[core,cpp,tools]:x64-windows

echo "========================================"
echo "  Updating libraries to latest version  "
echo "========================================"
timeout /t 1 /nobreak > NUL
vcpkg upgrade --no-dry-run
vcpkg integrate install

pause
