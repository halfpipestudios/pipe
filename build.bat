@echo off

if not exist .\build mkdir .\build
if not exist .\thirdparty\tgui\lib mkdir .\thirdparty\tgui\lib

echo ----------------------------------------
echo Building UI library ...
echo ----------------------------------------

set TARGET=tgui
set CFLAGS=/nologo /Od /Zi
set LIBS=User32.lib Ole32.lib Xinput.lib Xaudio2.lib d3d11.lib d3dcompiler.lib Dwmapi.lib
set SOURCES=.\thirdparty\tgui\src\*.c
set OBJS=.\build\stb_truetype.obj .\build\tgui.obj .\build\tgui_memory.obj .\build\tgui_gfx.obj .\build\tgui_os_win32.obj .\build\tgui_painter.obj .\build\tgui_geometry.obj .\build\tgui_docker.obj .\build\tgui_serializer.obj
set OUT_DIR=/Fo.\build\ /Fe.\build\%TARGET% /Fm.\build\
set INC_DIR=/I.\thirdparty /I.\thirdparty\tgui\include 
set LNK_DIR=

cl /c %CFLAGS% %INC_DIR% %SOURCES% %OUT_DIR% 
lib /OUT:".\thirdparty\tgui\lib\tgui.lib"  %OBJS%


echo ----------------------------------------
echo Build game ...
echo ----------------------------------------

set TARGET=pipe
set CFLAGS=/std:c++17 /W2 /nologo /Od /Zi /EHsc
set LIBS=User32.lib Ole32.lib Xinput.lib Xaudio2.lib d3d11.lib d3dcompiler.lib Dwmapi.lib tgui.lib
set SOURCES=.\src\cmp\*.cpp .\src\*.cpp
set OUT_DIR=/Fo.\build\ /Fe.\build\%TARGET% /Fm.\build\
set INC_DIR=/I.\ /I.\thirdparty /I.\thirdparty\tgui\include
set LNK_DIR=/LIBPATH:.\thirdparty\tgui\lib

cl %CFLAGS% %INC_DIR% %SOURCES% %OUT_DIR% /link %LNK_DIR% %LIBS% /SUBSYSTEM:CONSOLE

