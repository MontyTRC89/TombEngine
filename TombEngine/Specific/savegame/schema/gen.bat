@echo off
echo Generated ten_itemdata_generated.h from ten_itemdata.fbs
flatc.exe --cpp --strict-json --unknown-json --gen-object-api --force-empty --force-empty-vectors --cpp-std c++17 --scoped-enums ten_itemdata.fbs
echo Generated ten_savegame_generated.h from ten_savegame.fbs
flatc.exe --cpp --strict-json --unknown-json --gen-object-api --force-empty --force-empty-vectors --cpp-std c++17 --scoped-enums ten_savegame.fbs
echo Move ten_itemdata_generated.h
move /y ten_itemdata_generated.h ..\flatbuffers\ten_itemdata_generated.h
echo Move ten_savegame_generated.h
move /y ten_savegame_generated.h ..\flatbuffers\ten_savegame_generated.h
echo Error level: %errorlevel%
pause