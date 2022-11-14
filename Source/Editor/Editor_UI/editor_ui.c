editor_ui* EditorUI_Init(editor* Editor)
{
    arena* EditorUIArena = Arena_Create(Get_Base_Allocator(Editor->Arena), Mega(2));
    editor_ui* Result = Arena_Push_Struct(EditorUIArena, editor_ui);
    Result->Arena = EditorUIArena;
    
    EditorUI_Set(Result);
    Result->MainWindow = EditorUI_Create_Window(1920, 1080, Str8_Lit("AK Engine"), 0);
    Result->CmdBuffer = GPU_Allocate_Cmd_Buffers(Editor->DeviceGPU, 1)[0];
    
    arena* Scratch = Core_Get_Thread_Context()->Scratch;
    
    str8 FontPath = Str8_Concat(Get_Base_Allocator(Scratch), Editor->DataPath, Str8_Lit("Cousine-Regular.ttf"));
    if(!OS_Read_Entire_File(&Result->MainFontBuffer, Get_Base_Allocator(Scratch), FontPath))
        return NULL;
    
    Result->GlyphGenerator = FT_Glyph_Generator_Create(Get_Base_Allocator(Result->Arena));
    Result->GlyphCache = Glyph_Cache_Create(Get_Base_Allocator(Result->Arena), Result->GlyphGenerator);
    return Result;
}

editor_window* EditorUI_Create_Window(uint32_t Width, uint32_t Height, str8 WindowTitle, uint64_t WindowFlags)
{
    editor_ui* UI = EditorUI_Get();
    
    editor_window* Window = UI->FreeWindows;
    if(!Window) Window = Arena_Push_Struct(UI->Arena, editor_window);
    else Window = SLL_Pop_Front(UI->FreeWindows);
    Zero_Struct(Window, editor_window);
    
    gpu_device_context*  DeviceGPU      = Editor_Get()->DeviceGPU;
    //gpu_display_manager* DisplayManager = GPU_Get_Display_Manager(DeviceGPU);
    
    Window->Window = OS_Create_Window(Width, Height, WindowTitle, WindowFlags, Window);
#ifdef OS_WIN32
    //Window->Display = GPU_Create_Display(DisplayManager, ((win32_window*)Window->Window)->Handle);
#else
#error Not Implemented
#endif
    return Window;
}

void EditorUI_Delete_Window(editor_window* Window)
{
    editor_ui* UI = EditorUI_Get();
    gpu_device_context*  DeviceGPU      = Editor_Get()->DeviceGPU;
    //gpu_display_manager* DisplayManager = GPU_Get_Display_Manager(DeviceGPU);
    
    //GPU_Delete_Display(DisplayManager, Window->Display);
    OS_Delete_Window(Window->Window);
    SLL_Push_Front(UI->FreeWindows, Window);
}

void EditorUI_Shutdown()
{
    //TODO(JJ): Handle case
}

void EditorUI_Update(double dt)
{
    editor_ui* UI = EditorUI_Get();
    
    static font_face* DEBUGFontFace;
    if(!DEBUGFontFace)
    {
        DEBUGFontFace = Glyph_Generator_Create_Font_Face(UI->GlyphGenerator, UI->MainFontBuffer.Ptr, UI->MainFontBuffer.Size, 16);
    }
    
    glyph* Glyph = Glyph_Cache_Get(UI->GlyphCache, DEBUGFontFace, 'b');
    Glyph_Cache_Generate(UI->GlyphCache);
    
    GPU_Cmd_Buffer_Reset(UI->CmdBuffer);
    gpu_cmd_ui_pass* UIPass = GPU_Cmd_Buffer_Begin_UI_Pass(UI->CmdBuffer);
    GPU_Cmd_UI_Pass_Draw_Rectangle(UIPass, V2(0.0f, 0.0f), V2(100.0f, 100.0f), V3(0.0f, 1.0f, 0.0f), 1.0f);
    
    GPU_Dispatch_Cmds(Editor_Get()->DeviceGPU, &UI->CmdBuffer, 1);
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