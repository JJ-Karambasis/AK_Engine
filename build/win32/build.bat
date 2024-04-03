@echo off

for %%a in (%*) do set "%%a=1"
if not "%release%"=="1" set debug=1
if not "%x86%"=="1" set x64=1
if not "%clang%"=="1" set msvc=1

if "%msvc%"=="1" if "%x86%"=="1" call init_compiler.bat x86
if "%msvc%"=="1" if "%x64%"=="1" call init_compiler.bat x64

set code_path=..\code
set dependencies_path=%code_path%\dependencies
set runtime_path=%code_path%\runtime
set editor_path=%code_path%\editor
set editor_os_path=%editor_path%\os
set vk_include=%dependencies_path%\Vulkan-Headers\include

set cl_warnings=/WX /Wall /wd4061 /wd4062 /wd4065 /wd4100 /wd4189 /wd4191 /wd4201 /wd4255 /wd4505 /wd4577 /wd4582 /wd4625 /wd4626 /wd4668 /wd4710 /wd4711 /wd4774 /wd4820 /wd5045 /wd5262
set cl_common=  /nologo /FC /Z7 /Gs- /D_CRT_SECURE_NO_WARNINGS
set cl_debug= 	call cl /Od /DDEBUG_BUILD /MTd %cl_common% %cl_warnings%
set cl_release= call cl /O2 %cl_common% %cl_warnings%
set cl_link= 	/link /MANIFEST:EMBED /INCREMENTAL:NO
set cl_out= 	/out:

if "%debug%"=="1" set cl_debug=%cl_debug% /DEBUG

set clang_warnings= -Werror -Wall -Wno-switch -Wno-unused-variable -Wno-unused-function
set clang_common=   -g -gcodeview -fdiagnostics-absolute-paths -D_CRT_SECURE_NO_WARNINGS

if "%clang%"=="1" (
    if "%env32%"=="1" set clang_common=%clang_common% -m32
)

set clang_debug=    call clang -O0 -DDEBUG_BUILD %clang_common% %clang_warnings%
set clang_release=  call clang -O2 %clang_common% %clang_warnings%
set clang_link=     
set clang_out=      -o

if "%msvc%"=="1" set compile_debug=   %cl_debug%
if "%msvc%"=="1" set compile_release= %cl_release%
if "%msvc%"=="1" set compile_link=    %cl_link%
if "%msvc%"=="1" set out=             %cl_out%
if "%msvc%"=="1" set only_compile=    /c
if "%msvc%"=="1" set inc=             /I   
if "%msvc%"=="1" set obj=             /Fo:
if "%msvc%"=="1" set def=             /D 
if "%msvc%"=="1" set cpp=             /std:c++20
if "%msvc%"=="1" set c=               /std:c17


if "%clang%"=="1" set compile_debug=   %clang_debug%
if "%clang%"=="1" set compile_release= %clang_release%
if "%clang%"=="1" set compile_link=    %clang_link%
if "%clang%"=="1" set out=             %clang_out%
if "%clang%"=="1" set only_compile=    -c
if "%clang%"=="1" set inc=             -I
if "%clang%"=="1" set obj=             -o
if "%clang%"=="1" set def=             -D
if "%clang%"=="1" set cpp=             -std=c++20
if "%clang%"=="1" set c=               -std=c17

set include_common=%inc%%dependencies_path%\ak_lib %inc%%dependencies_path%\stb %inc%%runtime_path%\core %inc%%runtime_path% %inc%%runtime_path%\engine %inc%%code_path%\shaders

if "%debug%"=="1"   set compile=%compile_debug% %include_common%
if "%release%"=="1" set compile=%compile_release% %include_common%

if not exist ..\..\bin mkdir ..\..\bin

set gdi_objs=vk_loader.obj gdi.obj
pushd ..\..\bin    
    %compile% %inc%%vk_include% %only_compile% %c% %runtime_path%\gdi\vk\loader\vk_win32_loader.c %obj%vk_loader.obj %compile_link% || exit /b 1
    %compile% %inc%%vk_include% %only_compile% %cpp% %runtime_path%\gdi\vk\vk_gdi.cpp %obj%gdi.obj %compile_link% || exit /b 1
    %compile% %only_compile% %cpp% %inc%%editor_os_path% %editor_os_path%\win32\win32_os.cpp %compile_link% %obj%win32_os.obj || exit /b 1
    %compile% %cpp% ..\code\editor\editor.cpp %compile_link% win32_os.obj %gdi_objs% %out%AK_Engine.exe || exit /b 1
    %compile% %def%TEST_BUILD %inc%%code_path%\editor %cpp% ..\code\tests\unit\unit_test.cpp %compile_link% %out%Unit_Test.exe || exit /b 1
    del /s *.ilk >nul 2>&1
    del /s *.obj >nul 2>&1
    del /s *.exp >nul 2>&1
popd

set shader_common=  -nologo -Zi -spirv
set shader_debug=   call %VULKAN_SDK%\Bin\dxc -Qembed_debug -Od %shader_common%
set shader_release= call %VULKAN_SDK%\Bin\dxc -O3 %shader_common%

if "%debug%"=="1"   set shader=%shader_debug% %include_common%
if "%release%"=="1" set shader=%shader_release% %include_common%

set vtx_shader=%shader% -T vs_6_0 -E VS_Main -fvk-invert-y
set pxl_shader=%shader% -T ps_6_0 -E PS_Main

if not exist ..\..\bin\data\shaders mkdir ..\..\bin\data\shaders

pushd ..\..\bin\data\shaders
    %vtx_shader% /Fo shader_vs.shader ..\..\..\code\shaders\shader.hlsl
    %pxl_shader% /Fo shader_ps.shader ..\..\..\code\shaders\shader.hlsl
popd