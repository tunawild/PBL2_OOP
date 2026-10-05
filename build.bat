@echo off
set CXX=g++
set CXXFLAGS=-Wall -std=c++11 -O2
set INCLUDES=-I. -I"C:\raylib\raylib\src" -I"EVC_System\Header Files"
set LIBS=-L"C:\raylib\raylib\src" -lraylib -lopengl32 -lgdi32 -lwinmm

echo Compiling...
%CXX% %CXXFLAGS% %INCLUDES% main.cpp "EVC_System\Source Files\Models.cpp" "EVC_System\Source Files\Managers.cpp" "EVC_System\Source Files\UI.cpp" -o EVC_System.exe %LIBS%
if %ERRORLEVEL% equ 0 (
    echo Build Successful! Starting EVC_System.exe...
    start EVC_System.exe
) else (
    echo Build Failed!
    pause
)
