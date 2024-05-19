debug=0 
release=0
clang=0
env32=0
env64=0

for var in "$@"
do
    if [ "$var" = "release" ]; then release=1; fi
    if [ "$var" = "debug" ]; then debug=1; fi
    if [ "$var" = "clang" ]; then clang=1; fi
    if [ "$var" = "env32" ]; then env32=1; fi
    if [ "$var" = "env64" ]; then env64=1; fi
done

if [ ! $release -eq 1 ]; then debug=1; fi
if [ ! $env32 -eq 1 ]; then env64=1; fi

script=$(realpath "$0")
script_path=$(dirname "$script")
base_path="$script_path/../.."
code_path="$base_path/code"
dependencies_path="${code_path}/dependencies"
runtime_path="${code_path}/runtime"
shared_path="${code_path}/shared"
shader_path="${code_path}/shaders"
editor_path="${code_path}/editor"
editor_os_path="${editor_path}/os"
vk_include="${dependencies_path}/Vulkan-Headers/include"
packages_path="${base_path}/bin/packages"

clang_warnings="-Werror -Wall -Wno-switch -Wno-unused-variable -Wno-unused-function"
clang_common="-g -fdiagnostics-absolute-paths -flto"

if [ $clang -eq 1 ]; then
    if [ $env32 -eq 1 ]; then clang_common="$clang_common -m32"; fi
fi

clang_debug="clang++ -O0 -DDEBUG_BUILD $clang_common $clang_warnings"
clang_release="clang++ -O2 $clang_common $clang_warnings"
clang_link=     
clang_out="-o"

if [ $clang -eq 1 ]; then compile_link=${clang_link}; fi
if [ $clang -eq 1 ]; then compile_debug=${clang_debug}; fi
if [ $clang -eq 1 ]; then compile_release=${clang_release}; fi
if [ $clang -eq 1 ]; then compile_out=${clang_out}; fi
if [ $clang -eq 1 ]; then cpp="-std=c++20"; fi
if [ $clang -eq 1 ]; then inc="-I"; fi
if [ $clang -eq 1 ]; then only_compile="-c"; fi
if [ $clang -eq 1 ]; then def="-D"; fi

if [ $debug -eq 1 ]; then compile=${compile_debug}; fi
if [ $release -eq 1 ]; then compile=${compile_release}; fi

include_common="${inc}${dependencies_path}/ak_lib ${inc}${dependencies_path}/stb ${inc}${dependencies_path}/harfbuzz/src ${inc}${shared_path}/text/uba/sheenbidi/config ${inc}${dependencies_path}/SheenBidi/Headers ${inc}${shared_path}/glyph_rasterizer/freetype/config ${inc}${dependencies_path}/freetype/include ${inc}${runtime_path}/core ${inc}${runtime_path} ${inc}${runtime_path}/engine ${inc}${code_path}/shaders ${inc}${code_path}/shared"

compile="$compile $include_common"

if [ ! -d "$base_path/bin" ]; then
    mkdir "$base_path/bin"
fi

pushd "$base_path/bin"
    ${compile} ${def}EDITOR_PACKAGE_FILE_SYSTEM ${inc}${code_path}/editor/os ${cpp} -framework AppKit ${code_path}/editor/editor.cpp ${code_path}/editor/os/osx/osx.mm ${compile_link} ${compile_out} AK_Engine 
    # ${compile} ${cpp} ${inc}${code_path}/editor/os -framework AppKit ${code_path}/os_test.cpp ${code_path}/editor/os/osx/osx.mm ${compile_link} ${compile_out} os_test 
popd