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