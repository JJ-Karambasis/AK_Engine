#include <core.h>
#include "os/os.h"

int main() {
    if(!Core_Create()) {
        OS_Message_Box("A fatal error occurred during initialization!", "Error");
    }

    Core_Delete();

    return 0;
}

#include <core.cpp>

#if defined(OS_WIN32)
#pragma comment(lib, "user32.lib")
#endif