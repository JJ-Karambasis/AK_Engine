os_library* OS_Load_Library(str8 Name)
{
    arena* Scratch = Core_Get_Thread_Context()->Scratch;
    str8 LibraryName = Str8_Concat(Get_Base_Allocator(Scratch), Name, Str8_Lit(".dll"));
    str16 LibraryNameW = UTF8_To_UTF16(Get_Base_Allocator(Scratch), LibraryName);
    HMODULE Library = LoadLibraryW(LibraryNameW.Str);
    return (os_library*)Library;
}

void* OS_Get_Symbol(os_library* Library, strc SymbolName)
{
    return (void*)GetProcAddress((HMODULE)Library, SymbolName.Str);
}

void OS_Close_Library(os_library* Library)
{
    FreeLibrary((HMODULE)Library);
}