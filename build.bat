@echo off

if not exist .\build mkdir .\build

set TARGET=pipe
set CFLAGS=/nologo /Od /Zi /EHsc
set LIBS=User32.lib Ole32.lib Xinput.lib Xaudio2.lib d3d11.lib d3dcompiler.lib Dwmapi.lib
REM set SOURCES=.\src\main.cpp .\src\math.cpp .\src\memory.cpp .\src\input.cpp .\src\win32_platform.cpp .\src\platform_manager.cpp .\src\memory_manager.cpp
set SOURCES=.\src\*.cpp
set OUT_DIR=/Fo.\build\ /Fe.\build\%TARGET% /Fm.\build\
set INC_DIR=/I.\ /I.\thirdparty
set LNK_DIR=

cl %CFLAGS% %INC_DIR% %SOURCES% %OUT_DIR% /link %LNK_DIR% %LIBS% /SUBSYSTEM:CONSOLE

