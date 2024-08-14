@echo off
title VCpkg Library Uninnstaller
color 0

echo "========================================"
echo "   This batch removes vcpkg libraries   "
echo "             for TombEngine.            "
echo "========================================"
echo.
timeout /t 1 /nobreak > NUL

echo "========================================"
echo "       Uninstalling x86 libraries.      "
echo "========================================"
timeout /t 1 /nobreak > NUL

echo Uninstalling DirectX Tool Kit.
timeout /t 1 /nobreak > NUL
vcpkg remove directxtk[core,tools]:x86-windows

echo Uninstalling OIS.
timeout /t 1 /nobreak > NUL
vcpkg remove ois[core]:x86-windows

echo Uninstalling FlatBuffers.
timeout /t 1 /nobreak > NUL
vcpkg remove flatbuffers[core]:x86-windows

echo Uninstalling spdlog.
timeout /t 1 /nobreak > NUL
vcpkg remove spdlog[core]:x86-windows

echo Uninstalling Sol2 (Lua module).
timeout /t 1 /nobreak > NUL
vcpkg remove sol2[core]:x86-windows

echo Uninstalling Lua.
timeout /t 1 /nobreak > NUL
vcpkg remove lua[core,cpp,tools]:x86-windows

echo Uninstalling zlib.
timeout /t 1 /nobreak > NUL
vcpkg remove zlib:x86-windows

echo "========================================"
echo "       Uninstalling x64 libraries.      "
echo "========================================"
timeout /t 1 /nobreak > NUL

echo Uninstalling DirectX Tool Kit.
timeout /t 1 /nobreak > NUL
vcpkg remove directxtk[core,tools]:x64-windows

echo Uninstalling OIS.
timeout /t 1 /nobreak > NUL
vcpkg remove ois[core]:x64-windows

echo Uninstalling FlatBuffers.
timeout /t 1 /nobreak > NUL
vcpkg remove flatbuffers[core]:x64-windows

echo Uninstalling spdlog.
timeout /t 1 /nobreak > NUL
vcpkg remove spdlog[core]:x64-windows

echo Uninstalling Sol2 (Lua module).
timeout /t 1 /nobreak > NUL
vcpkg remove sol2[core]:x64-windows

echo Uninstalling Lua.
timeout /t 1 /nobreak > NUL
vcpkg remove lua[core,cpp,tools]:x64-windows

echo Uninstalling zlib.
timeout /t 1 /nobreak > NUL
vcpkg remove zlib:x64-windows

echo Removing integration with visual studio.
vcpkg integrate remove

pause
