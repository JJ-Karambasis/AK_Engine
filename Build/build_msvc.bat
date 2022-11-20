@echo off

set CurrentPath=%~dp0

call %CurrentPath%set_compiler_flags.bat
call %CurrentPAth%set_gpu_flags.bat

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

set Warnings=-W4 -wd4100 -wd4189 -wd4201 -wd4996 -wd4706 -wd4101 -wd4335
set CFlags=-nologo -Z7 -FC -D -D_HAS_EXCEPTIONS=0 -DCOMPILER_MSVC -GR- %Warnings% %BitnessFlag% %AssertFlags% %OptimizedFlags% %IncludePaths% %CommonFlags%

set Libs=advapi32.lib user32.lib gdi32.lib opengl32.lib

call %CurrentPath%set_free_type_flags.bat

pushd "%InstallPath%"
IF %CompileFreetype% == True (
	call %CurrentPath%set_free_type_flags.bat
	IF NOT EXIST "%InstallPath%\freetype" (
		mkdir "%InstallPath%\freetype"
	)		
	pushd "%InstallPath%\freetype"

	cl %CFlags% -DFT_DEBUG_LEVEL_ERROR -DFT2_BUILD_LIBRARY %FreeTypeIncludePath% -wd4244 -wd4267 -wd4701 -LD -c %FreetypeCFiles%
	lib -nologo -out:..\ftsystem.lib *obj

	popd
	RMDIR /S /Q "%InstallPath%\freetype"
)

set ProjectSharedIncludes=-I%CurrentPath%../Source/Projects/Shared

cl %CFlags% %ProjectSharedIncludes% %CurrentPath%../Source/Projects/Shader_Builder/shader_builder.c -link %Libs% -out:Shader_Builder.exe
Shader_Builder.exe --api GL --shaderPath "%CurrentPath%../Source/Shaders" --outputFilePath "%CurrentPath%../Source/Shaders"

cl %CFlags% %GPUFlags% -LD %GPUPath% -link %Libs% -out:GPU.dll
cl %CFlags% %CurrentPath%../Source/Editor/editor_tests.c -link %Libs% -out:AK_Engine_Tests.exe
cl %CFlags% %FreeTypeIncludePath% %CurrentPath%../Source/Editor/editor.c -link %Libs% ftsystem.lib -out:AK_Engine.exe
popd

EXIT /b %ERRORLEVEL%