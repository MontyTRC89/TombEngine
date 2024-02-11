@echo off
setlocal
set LDOC_DIR=.\packed\TEN-LDoc
set LUA_PATH=.\packed\?.lua
set LUA_CPATH=.\packed\?.dll
.\packed\lua.exe %LDOC_DIR%\\ldoc.lua %*
exit /b %ERRORLEVEL%
