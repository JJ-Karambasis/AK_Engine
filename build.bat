@echo off

SET GitPath=where git
set CurrentPath=%~dp0

FOR /F "tokens=* USEBACKQ" %%F IN (`where git`) DO (
	SET GitPath=%%F
)

IF %ERRORLEVEL% NEQ 0 (Echo Git is not installed on the machine. Please download from https://git-scm.com/downloads EXIT /b 1)

"%GitPath%" submodule update --init --recursive

SET CompilerType=
FOR /F ="delims=" %%A IN ('call build/ini.bat /s CompilerSettings /i CompilerType %CurrentPath%Configs\build.ini') DO (
	SET CompilerType=%%A
)

IF NOT DEFINED CompilerType (
	EXIT /b 1
)

if %CompilerType% == MSVC (
	call build/build_msvc.bat	
) else if %CompilerType% == Clang (
	call build/build_clang.bat
) else ( Echo Unknown compiler type: %CompilerType%. Please specify MSVC)

EXIT /B %ERRORLEVEL%