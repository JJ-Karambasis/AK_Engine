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
shared_path="$code_path/shared"
dependencies_path="$code_path/dependencies"

clang_warnings="-Werror -Wall -Wno-switch -Wno-unused-variable -Wno-unused-function"
clang_common="-g -fdiagnostics-absolute-paths -flto"

if [ $clang -eq 1 ]; then
    if [ $env32 -eq 1 ]; then clang_common="$clang_common -m32"; fi
fi

clang_debug="-O0 -DDEBUG_BUILD $clang_common $clang_warnings"
clang_release="-O2 $clang_common $clang_warnings"
clang_link=     
clang_out="-o"

if [ $clang -eq 1 ]; then compile_link=${clang_link}; fi
if [ $clang -eq 1 ]; then compile_debug=${clang_debug}; fi
if [ $clang -eq 1 ]; then compile_release=${clang_release}; fi
if [ $clang -eq 1 ]; then compile_out=${clang_out}; fi
if [ $clang -eq 1 ]; then cpp="-std=c++20"; fi
if [ $clang -eq 1 ]; then inc="-I"; fi
if [ $clang -eq 1 ]; then only_compile="-c"; fi

if [ $debug -eq 1 ]; then compile=${compile_debug}; fi
if [ $release -eq 1 ]; then compile=${compile_release}; fi

include_common="${inc}${dependencies_path}/ak_lib ${inc}${dependencies_path}/stb ${inc}${shared_path}"

compile="$compile $include_common"

if [ ! -d "$base_path/bin" ]; then
    mkdir "$base_path/bin"
fi

echo $compile

pushd "$base_path/bin"
    clang ${compile} ${cpp} ${only_compile} $shared_path/os/osx/osx.mm ${compile_link} ${compile_out} osx.o
    clang++ ${compile} ${cpp} $code_path/osx_test.cpp ${compile_link} osx.o ${compile_out} osx_test
popd