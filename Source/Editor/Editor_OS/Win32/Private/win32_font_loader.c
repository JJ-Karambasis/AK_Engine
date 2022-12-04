os_font_loader* OS_Font_Loader_Create(allocator* Allocator)
{
    arena* Arena = Arena_Create(Allocator, Mega(1));
    win32_font_loader* Loader = Arena_Push_Struct(Arena, win32_font_loader);
    Loader->Arena = Arena;
    
    return (os_font_loader*)Loader;
}

os_font_list    OS_Font_Loader_Get_All(os_font_loader* Loader);
os_font*        OS_Font_Loader_Get_Font(os_font_loader* FontLoader, str8 Name);
buffer          OS_Font_Load_Data(os_font_loader* FontLoader, os_font* Font, allocator* Allocator);

void OS_Font_Loader_Delete(os_font_loader* _Loader)
{
    if(_Loader)
    {
        win32_font_loader* Loader = (win32_font_loader*)_Loader;
        Arena_Delete(Loader->Arena);
    }
}