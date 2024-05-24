internal bool Win32_File_Exists(const wchar_t* File) {
    DWORD Flags = GetFileAttributesW(File);
    return (Flags != INVALID_FILE_ATTRIBUTES) && 
           !(Flags & FILE_ATTRIBUTE_DIRECTORY);
}

internal bool Win32_Directory_Exists(const wchar_t* Directory) {
    DWORD Flags = GetFileAttributesW(Directory);
    return (Flags != INVALID_FILE_ATTRIBUTES) && 
           (Flags & FILE_ATTRIBUTE_DIRECTORY);
}

bool OS_File_Exists(string File) {
    scratch Scratch = Scratch_Get();
    wstring FileW(&Scratch, File);
    return Win32_File_Exists(FileW.Str);
}

bool OS_Directory_Exists(string Directory) {
    scratch Scratch = Scratch_Get();
    wstring DirectoryW(&Scratch, Directory);
    return Win32_Directory_Exists(DirectoryW.Str); 
}