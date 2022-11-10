@echo off

set CurrentPath=%~dp0
call %CurrentPath%set_compiler_flags.bat

SET ClangPath=
FOR /F ="delims=" %%A IN ('call build/ini.bat /s Clang /i ClangPath %CurrentPath%..\Configs\build.ini') DO (
	SET ClangPath=%%A
)

IF NOT DEFINED ClangPath (
	EXIT /b 1
)

IF NOT EXIST "%ClangPath%" (
	ECHO Clang compiler "%ClangPath%" does not exist
	EXIT /b 1
)

IF %Optimized% == False (
	set OptimizedFlags=%OptimizedFlags% -O0	
) ELSE IF %Optimized% == True (
	set OptimizedFlags=%OptimizedFlags% -O3
)

set Warnings=-Wextra -Wno-unused-parameter -Wno-switch
set CFlags=-std=c99 -g -fdiagnostics-absolute-paths -DOS_WIN32 -DCOMPILER_CLANG %Warnings% %BitnessFlag% %AssertFlags% %OptimizedFlags% %IncludePaths% %CommonFlags%

set Libs=-ladvapi32 -luser32 -lgdi32

pushd "%InstallPath%"
"%ClangPath%" %CFlags% %Libs% %CurrentPath%../Source/Editor/editor_tests.c -o AK_Engine_Tests.exe
"%ClangPath%" %CFlags% %CurrentPath%../Source/Editor/editor.c -o AK_Engine.exe
popd

EXIT /b %ERRORLEVEL%