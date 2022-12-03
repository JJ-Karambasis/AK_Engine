resource_manager* Resource_Manager_Create(allocator* Allocator, str8 RootPath)
{
    arena* Arena = Arena_Create(Allocator, Mega(1));
    resource_manager* Result = Arena_Push_Struct(Arena, resource_manager);
    
    Result->Arena = Arena;
    
    arena* Scratch = Core_Get_Thread_Context()->Scratch;
    
    os_file_enumerator* DataEnumerator = OS_File_Enumerator_Create(Get_Base_Allocator(Scratch));
    for(os_file_enumerator_entry* Data = OS_File_Enumerator_Begin(DataEnumerator, RootPath); Data; 
        Data = OS_File_Enumerator_Next(DataEnumerator))
    {
        if(!Str8_Equal(Data->Filename, Str8_Lit(".")) &&
           !Str8_Equal(Data->Filename, Str8_Lit("..")))
        {
            if(OS_Directory_Exists(Data->Path))
            {
                os_file_enumerator* FileEnumerator = OS_File_Enumerator_Create(Get_Base_Allocator(Scratch));
                
                resource_type Type = (resource_type)-1;
                str8 TargetExt;
                Zero_Struct(&TargetExt, str8);
                
                if(Str8_Equal(Data->Filename, Str8_Lit("Fonts")))
                {
                    Type = RESOURCE_FONT;
                    TargetExt = Str8_Lit("ttf");
                }
                else if(Str8_Equal(Data->Filename, Str8_Lit("Languages")))
                {
                    Type = RESOURCE_LANGUAGE;
                    TargetExt = Str8_Lit("lang");
                }
                
                if(Type != (resource_type)-1)
                {
                    for(os_file_enumerator_entry* FontEntry = OS_File_Enumerator_Begin(FileEnumerator, Data->Path); FontEntry;
                        FontEntry = OS_File_Enumerator_Next(FileEnumerator))
                    {
                        if(!Str8_Equal(FontEntry->Filename, Str8_Lit(".")) &&
                           !Str8_Equal(FontEntry->Filename, Str8_Lit("..")))
                        {
                            uint64_t DotIndex = Str8_Find_Last_Char(FontEntry->Filename, '.');
                            str8 Ext = Str8_Substr(FontEntry->Filename, DotIndex+1, FontEntry->Filename.Length);
                            if(Str8_Equal(TargetExt, Ext))
                            {
                                uint64_t SlashIndex = Str8_Find_Last_Char(FontEntry->Filename, OS_FILE_DELIMTER_CHAR);
                                str8 Filename = Str8_Substr(FontEntry->Filename, SlashIndex+1, DotIndex);
                                
                                resource* Resource = Arena_Push_Struct(Arena, resource);
                                Resource->Name = Str8_Copy(Get_Base_Allocator(Arena), Filename);
                                Resource->Path = Str8_Copy(Get_Base_Allocator(Arena), FontEntry->Path);
                                if(!OS_Read_Entire_File(&Resource->Data, Get_Base_Allocator(Arena), Resource->Path))
                                {
                                    //TODO(JJ): Diagnostic and error logging
                                    return NULL;
                                }
                                
                                SLL_Push_Front(Result->Resources[Type], Resource);
                            }
                        }
                    }
                }
                
                OS_File_Enumerator_End(FileEnumerator);
                OS_File_Enumerator_Delete(FileEnumerator);
            }
        }
    }
    OS_File_Enumerator_End(DataEnumerator);
    OS_File_Enumerator_Delete(DataEnumerator);
    
    return Result;
}

resource* Resource_Manager_Get(resource_manager* Manager, resource_type Type, str8 Name)
{
    //TODO(JJ): Probably too slow once we get a lot of resources. Probably better to use a hashmap
    resource* Result = Manager->Resources[Type];
    while(Result && !Str8_Equal(Result->Name, Name)) Result = Result->Next;
    return Result;
}

void Resource_Manager_Delete(resource_manager* ResourceManager)
{
    Arena_Delete(ResourceManager->Arena);
}