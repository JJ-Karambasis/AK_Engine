#ifndef DEBUG_BUILD
#define DEBUG_BUILD
#endif

#include <engine.h>
#include <os/os.h>

#include "utest.h"

UTEST_STATE();
int main(int ArgCount, char* Args[]) {
    if(!Core_Create()) {
        return 1;
    }

    int Result = utest_main(ArgCount, Args);
    Core_Delete();

    return Result;
}

#include "shared/shared_tests.cpp"
#include <core/core.cpp>
