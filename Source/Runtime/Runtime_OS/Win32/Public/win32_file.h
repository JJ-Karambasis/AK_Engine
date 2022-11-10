#ifndef WIN32_FILE_H
#define WIN32_FILE_H

typedef struct win32_file
{
    HANDLE   Handle;
    uint32_t BitFlags;
    struct win32_file* Next;
} win32_file;

#endif
