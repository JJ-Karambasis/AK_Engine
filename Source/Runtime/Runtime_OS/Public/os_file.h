#ifndef OS_FILE_H
#define OS_FILE_H

typedef struct os_file os_file;

#define OS_INVALID_FILE_OFFSET ((uint64_t)-1)

enum 
{
    OS_CREATE_FILE_BIT_FLAG_NONE  = 0,
    OS_CREATE_FILE_BIT_FLAG_READ  = 1,
    OS_CREATE_FILE_BIT_FLAG_WRITE = 2
};

os_file* OS_Create_File(str8 Path, uint32_t BitFlag);
uint64_t OS_Get_File_Size(os_file* File);
bool8_t  OS_Read_File(os_file* File,  void* Data, uint32_t DataSize, uint64_t Offset);
bool8_t  OS_Write_File(os_file* File, void* Data, uint32_t DataSize, uint64_t Offset);
void     OS_Delete_File(os_file* File);

bool8_t OS_Read_Entire_File(buffer* Buffer, allocator* Allocator, str8 Path);

#endif