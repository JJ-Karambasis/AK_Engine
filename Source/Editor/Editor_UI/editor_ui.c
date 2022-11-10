void* FreeType_Alloc(FT_Memory Memory, long Size)
{
    heap* Heap = (heap*)Memory->user;
    return Heap_Allocate(Heap, Size, MEMORY_NO_CLEAR);
}

void FreeType_Free(FT_Memory Memory, void* Block)
{
    heap* Heap = (heap*)Memory->user;
    Heap_Free(Heap, Block);
}

void* FreeType_Realloc(FT_Memory Memory, long CurrentSize, long NewSize, void* Block)
{
    if(CurrentSize == NewSize) return Block;
    heap* Heap = (heap*)Memory->user;
    void* NewMemory = Heap_Allocate(Heap, NewSize, MEMORY_NO_CLEAR);
    Memory_Copy(NewMemory, Block, Min(CurrentSize, NewSize));
    if(Block) Heap_Free(Heap, Block);
    return NewMemory;
}

FT_Library Init_Freetype(arena* Arena)
{
    FT_Memory Memory = Arena_Push_Struct(Arena, struct FT_MemoryRec_);
    Memory->alloc   = FreeType_Alloc;
    Memory->free    = FreeType_Free;
    Memory->realloc = FreeType_Realloc;
    Memory->user    = Heap_Create(Get_Base_Allocator(Arena), Kilo(512));
    
    FT_Library Library;
    FT_Error Error = FT_New_Library(Memory, &Library);
    if(Error != 0) return NULL;
    
    FT_Add_Default_Modules(Library);
    FT_Set_Default_Properties(Library);
    return Library;
}

editor_ui* EditorUI_Init(arena* Arena)
{
    arena* EditorUIArena = Arena_Create(Get_Base_Allocator(Arena), Mega(2));
    editor_ui* Result = Arena_Push_Struct(EditorUIArena, editor_ui);
    Result->Arena = EditorUIArena;
    Result->Library = Init_Freetype(Result->Arena);
    EditorUI_Set(Result);
    
    return Result;
}

void EditorUI_Shutdown()
{
    //TODO(JJ): Handle case
}

global editor_ui* G_EditorUI;
void  EditorUI_Set(editor_ui* UI)
{
    G_EditorUI = UI;
}

editor_ui* EditorUI_Get()
{
    return G_EditorUI;
}