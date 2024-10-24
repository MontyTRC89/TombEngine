@echo off
title VCpkg Library Remover
color 0

echo "========================================"
echo "        This batch will remove       "
echo "     VCpkg libraries for TombEngine.    "
echo "========================================"
echo.
timeout /t 1 /nobreak > NUL

echo "========================================"
echo "         Removing x86 libraries.        "
echo "========================================"
timeout /t 1 /nobreak > NUL

echo Removing DirectX Tool Kit.
timeout /t 1 /nobreak > NUL
vcpkg remove directxtk[core,tools]:x86-windows

echo Removing FlatBuffers.
timeout /t 1 /nobreak > NUL
vcpkg remove flatbuffers[core]:x86-windows

echo Removing Lua.
timeout /t 1 /nobreak > NUL
vcpkg remove lua[core,cpp,tools]:x86-windows

echo Removing OIS.
timeout /t 1 /nobreak > NUL
vcpkg remove ois[core]:x86-windows

echo Removing Sol2 (Lua module).
timeout /t 1 /nobreak > NUL
vcpkg remove sol2[core]:x86-windows

echo Removing spdlog.
timeout /t 1 /nobreak > NUL
vcpkg remove spdlog[core]:x86-windows

echo Removing zlib.
timeout /t 1 /nobreak > NUL
vcpkg remove zlib:x86-windows

echo "========================================"
echo "         Removing x64 libraries.        "
echo "========================================"
timeout /t 1 /nobreak > NUL

echo Removing DirectX Tool Kit.
timeout /t 1 /nobreak > NUL
vcpkg remove directxtk[core,tools]:x64-windows

echo Removing FlatBuffers.
timeout /t 1 /nobreak > NUL
vcpkg remove flatbuffers[core]:x64-windows

echo Removing Lua.
timeout /t 1 /nobreak > NUL
vcpkg remove lua[core,cpp,tools]:x64-windows

echo Removing OIS.
timeout /t 1 /nobreak > NUL
vcpkg remove ois[core]:x64-windows

echo Removing Sol2 (Lua module).
timeout /t 1 /nobreak > NUL
vcpkg remove sol2[core]:x64-windows

echo Removing spdlog.
timeout /t 1 /nobreak > NUL
vcpkg remove spdlog[core]:x64-windows

echo Removing zlib.
timeout /t 1 /nobreak > NUL
vcpkg remove zlib:x64-windows

echo Removing integration with Visual Studio.
vcpkg integrate remove

pause
