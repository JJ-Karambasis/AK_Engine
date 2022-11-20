bool8_t OS_Read_Entire_File(buffer* Buffer, allocator* Allocator, str8 Path)
{
    os_file* File = OS_Create_File(Path, OS_CREATE_FILE_BIT_FLAG_READ);
    if(!File) return false;
    
    uint32_t Size  = Safe_U64_U32(OS_Get_File_Size(File));
    Buffer->Ptr    = Allocate(Allocator, Size, MEMORY_NO_CLEAR);
    bool8_t Result = OS_Read_File(File, Buffer->Ptr, Size, OS_INVALID_FILE_OFFSET);
    Buffer->Size   = Size;
    OS_Delete_File(File);
    return Result;
}

bool8_t OS_Read_Entire_File_Null_Term(buffer* Buffer, allocator* Allocator, str8 Path)
{
    os_file* File = OS_Create_File(Path, OS_CREATE_FILE_BIT_FLAG_READ);
    if(!File) return false;
    
    uint32_t Size     = Safe_U64_U32(OS_Get_File_Size(File));
    Buffer->Ptr       = Allocate(Allocator, Size+1, MEMORY_NO_CLEAR);
    bool8_t Result    = OS_Read_File(File, Buffer->Ptr, Size, OS_INVALID_FILE_OFFSET);
    Buffer->Size      = Size;
    Buffer->Ptr[Size] = 0;
    OS_Delete_File(File);
    return Result;
}

bool8_t OS_Write_Entire_File(str8 Path, const void* Data, uint32_t DataSize)
{
    os_file* File = OS_Create_File(Path, OS_CREATE_FILE_BIT_FLAG_WRITE);
    if(!File) return false;
    bool8_t Result = OS_Write_File(File, Data, DataSize, OS_INVALID_FILE_OFFSET);
    OS_Delete_File(File);
    return Result;
}