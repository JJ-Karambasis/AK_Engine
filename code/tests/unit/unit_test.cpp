#ifndef DEBUG_BUILD
#define DEBUG_BUILD
#endif

#include <engine.h>
#include <os/os_event.h>


#include "utest.h"
#include "os_event_tests.cpp"

UTEST_STATE();
int main(int ArgCount, char* Args[]) {
    if(!Core_Create()) {
        return 1;
    }

    int Result = utest_main(ArgCount, Args);
    Core_Delete();

    return Result;
}

#include <core.cpp>
#include <os/os_event.cpp>