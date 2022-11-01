@echo off

set CurrentPath=%~dp0

call %CurrentPath%set_compiler_flags.bat

set BatchfileVersion=
IF %Compile32Bit% == True (	
	set BatchfileVersion=x86
) ELSE IF %Compile32Bit% == False (	
	set BatchfileVersion=x64
)

SET VCVarsAllPath=
FOR /F ="delims=" %%A IN ('call build/ini.bat /s MSVC /i VCVarsAllPath %CurrentPath%..\Configs\build.ini') DO (
	SET VCVarsAllPath=%%A
)

IF NOT DEFINED VCVarsAllPath (
	EXIT /b 1
)

IF NOT EXIST "%VCVarsAllPath%" (
	ECHO VCVarsall batch file "%VCVarsAllPath%" does not exist
	EXIT /b 1
)

where cl > nul
IF %ERRORLEVEL% NEQ 0 (call "%VCVarsAllPath%" %BatchfileVersion%)

IF %Optimized% == False (
	set OptimizedFlags=%OptimizedFlags% -Od	
) ELSE IF %Optimized% == True (
	set OptimizedFlags=%OptimizedFlags% -O2
)

set Warnings=-W4 -wd4100 -wd4189 -wd4201
set CFlags=-nologo -Z7 -FC -D -D_HAS_EXCEPTIONS=0 -DCOMPILER_MSVC -GR- %Warnings% %BitnessFlag% %AssertFlags% %OptimizedFlags% %IncludePaths% %CommonFlags%

pushd "%InstallPath%"
cl %CFlags% %CurrentPath%../Source/Editor/editor_tests.c -link -out:AK_Engine_Tests.exe
cl %CFlags% %CurrentPath%../Source/Editor/editor.c -link -out:AK_Engine.exe
popd

EXIT /b %ERRORLEVEL%