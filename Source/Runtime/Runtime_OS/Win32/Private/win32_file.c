os_file* OS_Create_File(str8 Path, uint32_t BitFlag)
{
    win32_runtime_os* OS = (win32_runtime_os*)OS_Get();
    if(!OS) return NULL;
    
    DWORD DesiredAttributes = 0;
    DWORD CreationDisposition = 0;
    
    bool32_t ReadAndWrite = (BitFlag & OS_CREATE_FILE_BIT_FLAG_READ|OS_CREATE_FILE_BIT_FLAG_WRITE) == (OS_CREATE_FILE_BIT_FLAG_READ|OS_CREATE_FILE_BIT_FLAG_WRITE);
    
    if(ReadAndWrite)
    {
        DesiredAttributes = GENERIC_READ|GENERIC_WRITE;
        CreationDisposition = OPEN_ALWAYS;
    }
    else if(BitFlag & OS_CREATE_FILE_BIT_FLAG_READ)
    {
        DesiredAttributes = GENERIC_READ;
        CreationDisposition = OPEN_EXISTING;
    }
    else if(BitFlag & OS_CREATE_FILE_BIT_FLAG_WRITE)
    {
        DesiredAttributes = GENERIC_WRITE;
        CreationDisposition= CREATE_ALWAYS;
    }
    else
    {
        return NULL;
    }
    
    str16 PathW = UTF8_To_UTF16(Get_Base_Allocator(Core_Get_Thread_Context()->Scratch), Path);
    HANDLE Handle = CreateFileW(PathW.Str, DesiredAttributes, 0, NULL, CreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
    if(Handle == INVALID_HANDLE_VALUE) return NULL;
    
    win32_file* File = OS->FreeFiles;
    if(!File) File = Arena_Push_Struct(OS->Arena, win32_file);
    else SLL_Pop_Front(OS->FreeFiles);
    Zero_Struct(File, win32_file);
    
    File->BitFlags = BitFlag;
    File->Handle  = Handle;
    return (os_file*)File;
}

uint64_t OS_Get_File_Size(os_file* File)
{
    win32_file* OSFile = (win32_file*)File;
    LARGE_INTEGER Result;
    GetFileSizeEx(OSFile->Handle, &Result);
    return Result.QuadPart;
}

bool8_t OS_Read_File(os_file* File,  void* Data, uint32_t DataSize, uint64_t Offset)
{
    win32_file* OSFile = (win32_file*)File;
    if(!(OSFile->BitFlags & OS_CREATE_FILE_BIT_FLAG_READ))
        return false;
    
    OVERLAPPED* POverlappedOffset = NULL;
    OVERLAPPED OverlappedOffset;
    Zero_Struct(&OverlappedOffset, OVERLAPPED);
    if(Offset != OS_INVALID_FILE_OFFSET)
    {
        OverlappedOffset.Offset = (DWORD)(Offset & 0xFFFFFFFF);
        OverlappedOffset.OffsetHigh = (DWORD)((Offset >> 32) & 0xFFFFFFFF);
        POverlappedOffset = &OverlappedOffset;
    }
    
    DWORD BytesRead;
    if(!ReadFile(OSFile->Handle, Data, DataSize, &BytesRead, POverlappedOffset) || (BytesRead != DataSize))
        return false;
    
    return true;
}

bool8_t OS_Write_File(os_file* File, void* Data, uint32_t DataSize, uint64_t Offset)
{
    win32_file* OSFile = (win32_file*)File;
    if(!(OSFile->BitFlags & OS_CREATE_FILE_BIT_FLAG_WRITE))
        return false;
    
    OVERLAPPED* POverlappedOffset = NULL;
    OVERLAPPED OverlappedOffset;
    Zero_Struct(&OverlappedOffset, OVERLAPPED);
    if(Offset != OS_INVALID_FILE_OFFSET)
    {
        OverlappedOffset.Offset = (DWORD)(Offset & 0xFFFFFFFF);
        OverlappedOffset.OffsetHigh = (DWORD)((Offset >> 32) & 0xFFFFFFFF);
        POverlappedOffset = &OverlappedOffset;
    }
    
    DWORD BytesWritten;
    if(!WriteFile(OSFile->Handle, Data, DataSize, &BytesWritten, POverlappedOffset) || (BytesWritten != DataSize))
        return false;
    
    return true;
}

void OS_Delete_File(os_file* File)
{
    win32_runtime_os* OS = (win32_runtime_os*)OS_Get();
    if(!OS) return;
    
    win32_file* OSFile = (win32_file*)File;
    CloseHandle(OSFile->Handle);
    SLL_Push_Front(OS->FreeFiles, OSFile);
}