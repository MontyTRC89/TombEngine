@echo off
setlocal
set LDOC_DIR=.\compiler\ldoc
set LUA_PATH=.\compiler\?.lua
set LUA_CPATH=.\compiler\?.dll
.\compiler\lua.exe %LDOC_DIR%\\ldoc.lua %*
del output.xml
exit /b %ERRORLEVEL%
