#include <core/core.h>
#include <os/os.h>

int main() {
    Core_Create();
    OS_Create({});
    return OS_Process_Exit(OS_Exec_Process(String_Lit("echo"), String_Lit("hello")));
}

#include <core/core.cpp>