#ifndef OS_FILE_ENUMERATOR_H
#define OS_FILE_ENUMERATOR_H

typedef struct os_file_enumerator os_file_enumerator;
typedef struct os_file_enumerator_entry
{
    str8 Filename;
    str8 Path;
} os_file_enumerator_entry;

os_file_enumerator*       OS_File_Enumerator_Create(allocator* Allocator);
os_file_enumerator_entry* OS_File_Enumerator_Begin(os_file_enumerator* Enumerator, str8 Directory);
os_file_enumerator_entry* OS_File_Enumerator_Next(os_file_enumerator* Enumerator);
void                      OS_File_Enumerator_End(os_file_enumerator* Enumerator);
void                      OS_File_Enumerator_Delete(os_file_enumerator* Enumerator);

#endif