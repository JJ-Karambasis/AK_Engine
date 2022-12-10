typedef HRESULT dwrite_create_factory(DWRITE_FACTORY_TYPE FactoryType, REFIID IID, IUnknown** Factory);

void Win32__Font_Loader_Delete_All_Fonts(win32_font_loader* Loader)
{
    for(os_font* Font = Loader->Fonts.First; Font; Font = Font->Next)
    {
        win32_font* OSFont = (win32_font*)Font;
        IDWriteFontFamily_Release(OSFont->FontFamily);
    }
}

os_font_loader* OS_Font_Loader_Create(allocator* Allocator)
{
    win32_runtime_os* OS = (win32_runtime_os*)OS_Get();
    win32_editor_os* EditorOS = (win32_editor_os*)OS->OS.EditorOS;
    
    dwrite_create_factory* DWriteCreateFactory = (dwrite_create_factory*)GetProcAddress(EditorOS->DWriteLibrary, "DWriteCreateFactory");
    if(!DWriteCreateFactory)
    {
        //TODO(JJ): Diagnostic and error logging
        return NULL;
    }
    
    IDWriteFactory* WriteFactory;
    if(FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, &IID_IDWriteFactory, (IUnknown**)&WriteFactory)))
    {
        //TODO(JJ): Diagnostic and error logging
        return NULL;
    }
    
    IDWriteFontCollection* FontCollection;
    if(FAILED(IDWriteFactory_GetSystemFontCollection(WriteFactory, &FontCollection, false)))
    {
        //TODO(JJ): Diagnostic and error logging
        return NULL;
    }
    
    arena* Arena = Arena_Create(Allocator, Mega(1));
    win32_font_loader* Loader = Arena_Push_Struct(Arena, win32_font_loader);
    Loader->Arena = Arena;
    Loader->Factory = WriteFactory;
    Loader->FontCollection = FontCollection;
    
    OS_Font_Loader_Reload((os_font_loader*)Loader);
    
    return (os_font_loader*)Loader;
}

void OS_Font_Loader_Reload(os_font_loader* _Loader)
{
    arena* Scratch = Core_Get_Thread_Context()->Scratch;
    
    win32_font_loader* Loader = (win32_font_loader*)_Loader;
    if(Loader->Marker.Arena)
    {
        Win32__Font_Loader_Delete_All_Fonts(Loader);
        Arena_Set_Marker(Loader->Arena, &Loader->Marker);
    }
    Loader->Marker = Arena_Get_Marker(Loader->Arena);
    
    uint32_t FontFamilyCount = IDWriteFontCollection_GetFontFamilyCount(Loader->FontCollection);
    for(uint32_t FontFamilyIndex = 0; FontFamilyIndex < FontFamilyCount; FontFamilyIndex++)
    {
        IDWriteFontFamily* FontFamily;
        if(SUCCEEDED(IDWriteFontCollection_GetFontFamily(Loader->FontCollection, FontFamilyIndex, &FontFamily)))
        {
            IDWriteLocalizedStrings* LocalizedString;
            if(SUCCEEDED(IDWriteFontFamily_GetFamilyNames(FontFamily, &LocalizedString)))
            {
                uint32_t Length;
                if(SUCCEEDED(IDWriteLocalizedStrings_GetStringLength(LocalizedString, 0, &Length)))
                {
                    uint16_t* Buffer = Arena_Push_Array(Scratch, uint16_t, Length+1);
                    Buffer[Length] = 0;
                    
                    if(SUCCEEDED(IDWriteLocalizedStrings_GetString(LocalizedString, 0, Buffer, Length+1)))
                    {
                        str8 FontFamilyName = UTF16_To_UTF8(Get_Base_Allocator(Loader->Arena), Str16(Buffer, Length));
                        
                        win32_font* Font = Arena_Push_Struct(Loader->Arena, win32_font);
                        Font->Base.Name = FontFamilyName;
                        Font->FontFamily = FontFamily;
                        
                        SLL_Push_Back(Loader->Fonts.First, Loader->Fonts.Last, &Font->Base);
                        Loader->Fonts.Count++;
                        
                        IDWriteLocalizedStrings_Release(LocalizedString);
                    }
                    else
                    {
                        IDWriteLocalizedStrings_Release(LocalizedString);
                        IDWriteFontFamily_Release(FontFamily);
                    }
                }
                else
                {
                    IDWriteLocalizedStrings_Release(LocalizedString);
                    IDWriteFontFamily_Release(FontFamily);
                }
            }
            else
            {
                IDWriteFontFamily_Release(FontFamily);
            }
        }
    }
}

os_font_list OS_Font_Loader_Get_All(os_font_loader* _Loader)
{
    win32_font_loader* Loader = (win32_font_loader*)_Loader;
    return Loader->Fonts;
}

os_font* OS_Font_Loader_Get_Font(os_font_loader* _Loader, str8 Name)
{
    win32_font_loader* Loader = (win32_font_loader*)_Loader;
    for(os_font* Font = Loader->Fonts.First; Font; Font = Font->Next)
    {
        if(Str8_Equal(Font->Name, Name))
            return Font;
    }
    
    return NULL;
}

buffer OS_Font_Loader_Load(os_font_loader* _Loader, os_font* _Font, allocator* Allocator)
{
    buffer Result;
    Zero_Struct(&Result, buffer);
    
    win32_font_loader* Loader = (win32_font_loader*)_Loader;
    win32_font* OSFont = (win32_font*)_Font;
    if(Loader && OSFont)
    {
        IDWriteFontFamily* FontFamily = OSFont->FontFamily;
        IDWriteFont* Font;
        if(SUCCEEDED(IDWriteFontFamily_GetFirstMatchingFont(FontFamily, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STRETCH_NORMAL, DWRITE_FONT_STYLE_NORMAL, &Font)))
        {
            IDWriteFontFace* FontFace;
            if(SUCCEEDED(IDWriteFont_CreateFontFace(Font, &FontFace)))
            {
                uint32_t NumberOfFiles;
                if(SUCCEEDED(IDWriteFontFace_GetFiles(FontFace, &NumberOfFiles, NULL)))
                {
                    IDWriteFontFile* Files;
                    if(SUCCEEDED(IDWriteFontFace_GetFiles(FontFace, &NumberOfFiles, &Files)))
                    {
                        const void*  ReferenceKey;
                        UINT32 ReferenceKeyLength;
                        
                        if(SUCCEEDED(IDWriteFontFile_GetReferenceKey(Files, &ReferenceKey, &ReferenceKeyLength)))
                        {
                            IDWriteFontFileLoader* DWLoader;
                            if(SUCCEEDED(IDWriteFontFile_GetLoader(Files, &DWLoader)))
                            {
                                IDWriteFontFileStream* Stream;
                                if(SUCCEEDED(IDWriteFontFileLoader_CreateStreamFromKey(DWLoader, ReferenceKey, ReferenceKeyLength, &Stream)))
                                {
                                    
                                    UINT64 FileSize;
                                    if(SUCCEEDED(IDWriteFontFileStream_GetFileSize(Stream, &FileSize)))
                                    {
                                        void* Data;
                                        void* AllocContext;
                                        if(SUCCEEDED(IDWriteFontFileStream_ReadFileFragment(Stream, &Data, 0, FileSize, &AllocContext)))
                                        {
                                            Result.Ptr = Allocate(Allocator, FileSize, MEMORY_NO_CLEAR);
                                            Result.Size = FileSize;
                                            Memory_Copy(Result.Ptr, Data, FileSize);
                                            IDWriteFontFileStream_ReleaseFileFragment(Stream, AllocContext);
                                        }
                                    }
                                    IDWriteFontFileStream_Release(Stream);
                                }
                            }
                        }
                        
                        IDWriteFontFile_Release(Files);
                    }
                }
                IDWriteFontFace_Release(FontFace);
            }
            IDWriteFont_Release(Font);
        }
    }
    
    return Result;
}

void OS_Font_Loader_Delete(os_font_loader* _Loader)
{
    if(_Loader)
    {
        win32_font_loader* Loader = (win32_font_loader*)_Loader;
        Win32__Font_Loader_Delete_All_Fonts(Loader);
        IDWriteFontCollection_Release(Loader->FontCollection);
        IDWriteFactory_Release(Loader->Factory);
        Arena_Delete(Loader->Arena);
    }
}