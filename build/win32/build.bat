@echo off

for %%a in (%*) do set "%%a=1"
if not "%release%"=="1" set debug=1
if not "%x86%"=="1" set x64=1
if not "%clang%"=="1" set msvc=1

if "%x86%"=="1" call init_compiler.bat x86
if "%x64%"=="1" call init_compiler.bat x64

set code_path=..\code
set dependencies_path=%code_path%\dependencies
set runtime_path=%code_path%\runtime
set editor_path=%code_path%\editor
set editor_os_path=%editor_path%\os

set cl_warnings=/WX /Wall /wd4062 /wd4065 /wd4100 /wd4189 /wd4191 /wd4201 /wd4255 /wd4577 /wd4668 /wd4710 /wd4711 /wd4774 /wd4820 /wd5045 /wd5262
set cl_common=  /nologo /FC /Z7 /Gs- /std:c++20 /D_CRT_SECURE_NO_WARNINGS
set cl_debug= 	call cl /Od /DDEBUG_BUILD %cl_common% %cl_warnings%
set cl_release= call cl /O2 %cl_common% %cl_warnings%
set cl_link= 	/link /MANIFEST:EMBED /INCREMENTAL:NO
set cl_out= 	/out:

set clang_warnings= -Werror -Wall 
set clang_common=   -g -gcodeview -fdiagnostics-absolute-paths -std=c++20 -D_CRT_SECURE_NO_WARNINGS

if "%clang%"=="1" (
    if "%env32%"=="1" set clang_common=%clang_common% -m32
)

set clang_debug=    call clang -O0 %clang_common%
set clang_release=  call clang -O2 %clang_common%
set clang_link=     -fuse-ld=lld
set clang_out=      -o

if "%msvc%"=="1" set compile_debug=   %cl_debug%
if "%msvc%"=="1" set compile_release= %cl_release%
if "%msvc%"=="1" set compile_link=    %cl_link%
if "%msvc%"=="1" set out=             %cl_out%
if "%msvc%"=="1" set only_compile=    /c
if "%msvc%"=="1" set inc=             /I          

if "%clang%"=="1" set compile_debug=   %clang_debug%
if "%clang%"=="1" set compile_release= %clang_release%
if "%clang%"=="1" set compile_link=    %clang_link%
if "%clang%"=="1" set out=             %clang_out%
if "%clang%"=="1" set only_compile=    -c
if "%clang%"=="1" set inc=             -I

set include_common=%inc%%dependencies_path%\ak_lib %inc%%dependencies_path%\stb %inc%%runtime_path%\core

if "%debug%"=="1"   set compile=%compile_debug% %include_common%
if "%release%"=="1" set compile=%compile_release% %include_common%

if not exist ..\..\bin mkdir ..\..\bin

pushd ..\..\bin    
    %compile% %only_compile% %inc%%editor_os_path% %editor_os_path%\win32\win32_os.cpp %compile_link% || exit /b 1
    %compile% ..\code\editor\editor.cpp win32_os.obj %compile_link% %out%AK_Engine.exe || exit /b 1
popd