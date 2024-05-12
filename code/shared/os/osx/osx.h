#ifndef OSX_H
#define OSX_H

#include <os/posix/posix.h>
#include <mach-o/dyld.h>

struct osx_os : os {
};

internal osx_os* OSX_Get_Or_Set(osx_os* OS);

#endif