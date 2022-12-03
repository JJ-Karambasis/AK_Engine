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
set LibCommon=-nologo -D_HAS_EXCEPTIONS=0 -DCOMPILER_MSVC %Warnings%
set CFlags=-Z7 -FC -GR- %LibCommon% %BitnessFlag% %AssertFlags% %OptimizedFlags% %IncludePaths% %CommonFlags%

set Libs=advapi32.lib user32.lib gdi32.lib opengl32.lib

call %CurrentPath%set_free_type_flags.bat

pushd "%InstallPath%"
IF %CompileFreetype% == True (
	call %CurrentPath%set_free_type_flags.bat
	IF NOT EXIST "%InstallPath%\freetype" (
		mkdir "%InstallPath%\freetype"
	)		
	pushd "%InstallPath%\freetype"

	cl %LibCommon% -O2 -DFT_DEBUG_LEVEL_ERROR -DFT2_BUILD_LIBRARY %FreeTypeIncludePath% -wd4244 -wd4267 -wd4701 -Gy -LD -c %FreetypeCFiles%
	lib -nologo -out:..\ftsystem.lib *obj

	popd
	RMDIR /S /Q "%InstallPath%\freetype"
)

IF %CompileHarfbuzz% == True (
	IF NOT EXIST "%InstallPath%\harfbuzz" (
		mkdir "%InstallPath%\harfbuzz"
	)		
	pushd "%InstallPath%\harfbuzz"

	cl %LibCommon% -wd4244 -wd4267 -wd4458 -wd4456 -wd4127 -wd4245 -wd4459 -wd4457 -wd4702 -wd4701 -wd4065 -wd4146 -O2 -Dhb_malloc_impl=HB_Heap_Alloc -Dhb_calloc_impl=HB_Heap_Calloc -Dhb_free_impl=HB_Heap_Free -Dhb_realloc_impl=HB_Heap_Realloc -DHB_TINY %FreeTypeIncludePath% -Gy -LD -c %CurrentPath%../Source/ThirdParty/Harfbuzz/src/harfbuzz.cc
	lib -nologo -out:..\hb.lib *obj

	popd
	RMDIR /S /Q "%InstallPath%\harfbuzz"
)

set ProjectSharedIncludes=-I%CurrentPath%../Source/Projects/Shared

cl %CFlags% %ProjectSharedIncludes% %CurrentPath%../Source/Projects/Shader_Builder/shader_builder.c -link %Libs% -out:Shader_Builder.exe
Shader_Builder.exe --api GL --shaderPath "%CurrentPath%../Source/Shaders" --outputFilePath "%CurrentPath%../Source/Shaders"

cl %CFlags% %GPUFlags% -LD %GPUPath% -link %Libs% -out:GPU.dll
cl %CFlags% %CurrentPath%../Source/Editor/editor_tests.c -link %Libs% -out:AK_Engine_Tests.exe
cl %CFlags% %FreeTypeIncludePath% %CurrentPath%../Source/Editor/editor.c -link %Libs% ftsystem.lib hb.lib -opt:ref -out:AK_Engine.exe
popd

EXIT /b %ERRORLEVEL%