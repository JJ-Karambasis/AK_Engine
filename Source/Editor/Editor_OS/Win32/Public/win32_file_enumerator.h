#ifndef WIN32_FILE_ENUMERATOR_H
#define WIN32_FILE_ENUMERATOR_H

typedef struct os_file_enumerator
{
    arena*       Arena;
    arena_marker Marker;
    HANDLE       Handle;
    str8         Directory;
} os_file_enumerator;

#endif