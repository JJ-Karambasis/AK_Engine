os_file_enumerator* OS_File_Enumerator_Create(allocator* Allocator)
{
    arena* Arena = Arena_Create(Allocator, Mega(1));
    os_file_enumerator* Enumerator = Arena_Push_Struct(Arena, os_file_enumerator);
    Enumerator->Arena = Arena;
    return Enumerator;
}

os_file_enumerator_entry* OS_File_Enumerator_Begin(os_file_enumerator* Enumerator, str8 Directory)
{
    arena* Scratch = Core_Get_Thread_Context()->Scratch;
    
    str16 DirectoryW;
    if(Str8_Ends_With_Char(Directory, OS_FILE_DELIMTER_CHAR))
    {
        DirectoryW = UTF8_To_UTF16(Get_Base_Allocator(Scratch), Str8_Concat(Get_Base_Allocator(Scratch), Directory, Str8_Lit("*")));
    }
    else
    {
        DirectoryW = UTF8_To_UTF16(Get_Base_Allocator(Scratch), Str8_Concat(Get_Base_Allocator(Scratch), Directory, Str8_Lit(Glue(OS_FILE_DELIMTER, "*"))));
    }
    
    WIN32_FIND_DATAW FindData;
    HANDLE File = FindFirstFileW(DirectoryW.Str, &FindData);
    if(File != INVALID_HANDLE_VALUE)
    {
        Enumerator->Marker = Arena_Get_Marker(Enumerator->Arena);
        
        if(Str8_Ends_With_Char(Directory, OS_FILE_DELIMTER_CHAR))
            Enumerator->Directory = Str8_Copy(Get_Base_Allocator(Enumerator->Arena), Directory);
        else
            Enumerator->Directory = Str8_Concat(Get_Base_Allocator(Enumerator->Arena), Directory, Str8_Lit(OS_FILE_DELIMTER));
        
        Enumerator->Handle = File;
        
        os_file_enumerator_entry* Entry = Arena_Push_Struct(Enumerator->Arena, os_file_enumerator_entry);
        Entry->Filename = UTF16_To_UTF8(Get_Base_Allocator(Enumerator->Arena), Str16_Null_Term(FindData.cFileName));
        Entry->Path = Str8_Concat(Get_Base_Allocator(Enumerator->Arena), Enumerator->Directory, Entry->Filename);  
        return Entry;
    }
    
    return NULL;
}

os_file_enumerator_entry* OS_File_Enumerator_Next(os_file_enumerator* Enumerator)
{
    arena* Scratch = Core_Get_Thread_Context()->Scratch;
    
    WIN32_FIND_DATAW FindData;
    if(FindNextFileW(Enumerator->Handle, &FindData))
    {
        os_file_enumerator_entry* Entry = Arena_Push_Struct(Enumerator->Arena, os_file_enumerator_entry);
        Entry->Filename = UTF16_To_UTF8(Get_Base_Allocator(Enumerator->Arena), Str16_Null_Term(FindData.cFileName));
        Entry->Path = Str8_Concat(Get_Base_Allocator(Enumerator->Arena), Enumerator->Directory, Entry->Filename);  
        return Entry;
    }
    return NULL;
}

void OS_File_Enumerator_End(os_file_enumerator* Enumerator)
{
    FindClose(Enumerator->Handle);
    Arena_Set_Marker(Enumerator->Arena, &Enumerator->Marker);
}

void OS_File_Enumerator_Delete(os_file_enumerator* Enumerator)
{
    Arena_Delete(Enumerator->Arena);
}