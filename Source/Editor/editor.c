#include "editor.h"

static gpu_init*       GPU_Init;
static gpu_shutdown*   GPU_Shutdown;
static gpu_reload*     GPU_Reload;
static gpu_set_device* GPU_Set_Device;

int main(int ArgumentCount, char** Arguments)
{
    if(!Editor_Init()) return 1;
    Editor_Update();
    Editor_Shutdown();
    return 0;
}

editor* Editor_Init()
{
    core* Core = Core_Init();
    arena* EditorArena = Arena_Create(OS_Get_Allocator(), Mega(32));
    editor* Editor = Arena_Push_Struct(EditorArena, editor);
    
    Editor->Arena    = EditorArena;
    Editor->RootPath = OS_Get_Application_Path(Editor->Arena);
    Editor->DataPath = Str8_Concat(Get_Base_Allocator(Editor->Arena), Editor->RootPath, Str8_Lit(Glue("Data", OS_FILE_DELIMTER)));
    Editor->Core     = Core;
    
    os_library* GPULibrary = OS_Load_Library(Str8_Lit("GPU"));
    if(!GPULibrary) return NULL;
    
    GPU_Init = (gpu_init*)OS_Get_Symbol(GPULibrary, StrC_Lit("GPU_Init"));
    GPU_Shutdown = (gpu_shutdown*)OS_Get_Symbol(GPULibrary, StrC_Lit("GPU_Shutdown"));
    GPU_Reload = (gpu_reload*)OS_Get_Symbol(GPULibrary, StrC_Lit("GPU_Reload"));
    GPU_Set_Device = (gpu_set_device*)OS_Get_Symbol(GPULibrary, StrC_Lit("GPU_Set_Device"));
    
    Editor->ContextGPU = GPU_Init(Core);
    Editor->DeviceGPU = GPU_Set_Device(Editor->ContextGPU, Editor->ContextGPU->DeviceList.Devices[0]);
    
    Editor_Set(Editor);
    
    arena* Scratch = Core_Get_Thread_Context()->Scratch;
    
    Editor->MainWindow = Editor_Create_Window(1920, 1080, Str8_Lit("AK Engine"), 0);
    str8 FontPath = Str8_Concat(Get_Base_Allocator(Scratch), Editor->DataPath, Str8_Lit("Cousine-Regular.ttf"));
    if(!OS_Read_Entire_File(&Editor->MainFontBuffer, Get_Base_Allocator(Scratch), FontPath))
        return NULL;
    
    Editor->GlyphGenerator = FT_Glyph_Generator_Create(Get_Base_Allocator(Editor->Arena));
    Editor->GlyphCache = Glyph_Cache_Create(Get_Base_Allocator(Editor->Arena), Editor->GlyphGenerator, 
                                            GPU_Get_Resource_Manager(Editor->DeviceGPU));
    
    return Editor;
}

void Editor_Update()
{
    editor* Editor = Editor_Get();
    
    gpu_device_context* DeviceGPU = Editor->DeviceGPU;
    gpu_display_manager* DisplayManager = GPU_Get_Display_Manager(DeviceGPU);
    gpu_resource_manager* ResourceManager = GPU_Get_Resource_Manager(DeviceGPU);
    gpu_cmd_buffer* CmdBuffer = GPU_Allocate_Cmd_Buffer(DeviceGPU);
    
    v2i WindowDim = Editor->MainWindow->Window->Dim;
    gpu_texture2D* RenderTarget = GPU_Resource_Manager_Create_Texture2D(ResourceManager, Safe_S64_U32(WindowDim.x), Safe_S64_U32(WindowDim.y), GPU_TEXTURE_FORMAT_R8G8B8A8_UNORM, GPU_TEXTURE_USAGE_RENDER_TARGET|GPU_TEXTURE_USAGE_SAMPLED);
    
    gpu_framebuffer_create_info FramebufferCreateInfo;
    Zero_Struct(&FramebufferCreateInfo, gpu_framebuffer_create_info);
    FramebufferCreateInfo.Dim = WindowDim;
    FramebufferCreateInfo.ColorAttachmentCount = 1;
    FramebufferCreateInfo.ColorAttachments = &RenderTarget;
    
    gpu_framebuffer* ColorFramebuffer = GPU_Resource_Manager_Create_Framebuffer(ResourceManager, &FramebufferCreateInfo);
    
    gpu_sampler_create_info SamplerCreateInfo;
    Zero_Struct(&SamplerCreateInfo, gpu_sampler_create_info);
    SamplerCreateInfo.MinFilter = GPU_SAMPLER_FILTER_LINEAR;
    SamplerCreateInfo.MagFilter = GPU_SAMPLER_FILTER_LINEAR;
    SamplerCreateInfo.AddressModeU = GPU_SAMPLER_ADDRESS_MODE_CLAMP;
    SamplerCreateInfo.AddressModeV = GPU_SAMPLER_ADDRESS_MODE_CLAMP;
    
    gpu_sampler* LinearSampler = GPU_Resource_Manager_Create_Sampler(ResourceManager, &SamplerCreateInfo);
    
    bool32_t IsLooping = true;
    
    uint64_t StartClock = OS_QPC();
    while(IsLooping)
    {
        double dt = OS_High_Res_Elapsed_Time(StartClock, OS_QPC());
        StartClock = OS_QPC();
        
        OS_Poll_Events();
        
        const os_event* Event = OS_Get_Next_Event();
        while(Event)
        {
            switch(Event->Type)
            {
                case OS_EVENT_TYPE_WINDOW_CLOSED:
                {
                    if(Event->Window == Editor->MainWindow->Window)
                        IsLooping = false;
                    
                    editor_window* Window = (editor_window*)Event->Window->UserData;
                    Editor_Delete_Window(Window);
                } break;
                
                case OS_EVENT_TYPE_WINDOW_RESIZED:
                {
                    editor_window* Window = (editor_window*)Event->Window->UserData;
                    WindowDim = Event->Window->Dim;
                    GPU_Display_Resize(Window->Display, Safe_S64_U32(WindowDim.x), Safe_S64_U32(WindowDim.y));
                    GPU_Resource_Manager_Delete_Framebuffer(ResourceManager, ColorFramebuffer);
                    GPU_Resource_Manager_Delete_Texture2D(ResourceManager, RenderTarget);
                    
                    RenderTarget = GPU_Resource_Manager_Create_Texture2D(ResourceManager, Safe_S64_U32(WindowDim.x), Safe_S64_U32(WindowDim.y), GPU_TEXTURE_FORMAT_R8G8B8A8_UNORM, GPU_TEXTURE_USAGE_RENDER_TARGET);
                    
                    Zero_Struct(&FramebufferCreateInfo, gpu_framebuffer_create_info);
                    FramebufferCreateInfo.Dim = WindowDim;
                    FramebufferCreateInfo.ColorAttachmentCount = 1;
                    FramebufferCreateInfo.ColorAttachments = &RenderTarget;
                    
                    ColorFramebuffer = GPU_Resource_Manager_Create_Framebuffer(ResourceManager, &FramebufferCreateInfo);
                } break;
            }
            
            Event = OS_Get_Next_Event();
        }
        
        GPU_Cmd_Buffer_Reset(CmdBuffer);
        
        static font_face* DEBUGFontFace;
        if(!DEBUGFontFace)
        {
            DEBUGFontFace = Glyph_Generator_Create_Font_Face(Editor->GlyphGenerator, Editor->MainFontBuffer.Ptr, Editor->MainFontBuffer.Size, 64);
        }
        
        gpu_display* MainDisplay = Editor->MainWindow->Display;
        
        glyph* Glyph = Glyph_Cache_Get(Editor->GlyphCache, DEBUGFontFace, 'b');
        Glyph_Cache_Generate(Editor->GlyphCache, CmdBuffer);
        
        gpu_color_clear_attachment ColorClears[1];
        Memory_Clear(ColorClears, sizeof(ColorClears));
        
        ColorClears[0].Float[0] = 1.0f;
        ColorClears[0].Float[3] = 1.0f;
        
        gpu_ui_pass_begin_info BeginInfo;
        Zero_Struct(&BeginInfo, gpu_ui_pass_begin_info);
        BeginInfo.FramebufferInfo.Clear.ColorCount  = Array_Count(ColorClears);
        BeginInfo.FramebufferInfo.Clear.ColorClears = ColorClears;
        BeginInfo.FramebufferInfo.Framebuffer = ColorFramebuffer;
        
        gpu_ui_pass* UIPass = GPU_Cmd_Buffer_Begin_UI_Pass(CmdBuffer, &BeginInfo);
        GPU_UI_Pass_Draw_Rectangle(UIPass, V2(0.0f, 0.0f), V2(100.0f, 100.0f), V4(0.0f, 0.0f, 1.0f, 1.0f));
        GPU_UI_Pass_Draw_Rectangle(UIPass, V2(200.0f, 200.0f), V2(400.0f, 400.0f), V4(0.0f, 1.0f, 1.0f, 1.0f));
        
        gpu_texture_unit TextureUnit;
        TextureUnit.Texture = Glyph->Texture;
        TextureUnit.Sampler = LinearSampler;
        
        v2 Min = V2(300.0f, 300.0f);
        GPU_UI_Pass_Draw_Texture_Rectangle(UIPass, Min, V2_Add_V2(Min, V2((float)Glyph->Width, (float)Glyph->Height)), TextureUnit);
        
        GPU_Cmd_Copy_Texture_To_Display(CmdBuffer, MainDisplay, 0, 0, RenderTarget, 0, 0, Safe_S64_U32(WindowDim.x), Safe_S64_U32(WindowDim.y));
        
        GPU_Dispatch_Cmds(DeviceGPU, &CmdBuffer, 1);
        
        GPU_Present_Displays(DisplayManager, &MainDisplay, 1);
        
        Arena_Clear(Core_Get_Thread_Context()->Scratch, MEMORY_NO_CLEAR);
    }
}

void Editor_Shutdown()
{
    //TODO(JJ): Handle case
}

editor_window* Editor_Create_Window(uint32_t Width, uint32_t Height, str8 WindowTitle, uint64_t WindowFlags)
{
    editor* Editor = Editor_Get();
    
    editor_window* Window = Editor->FreeWindows;
    if(!Window) Window = Arena_Push_Struct(Editor->Arena, editor_window);
    else Window = SLL_Pop_Front(Editor->FreeWindows);
    Zero_Struct(Window, editor_window);
    
    gpu_device_context*  DeviceGPU = Editor->DeviceGPU;
    gpu_display_manager* DisplayManager = GPU_Get_Display_Manager(DeviceGPU);
    
    Window->Window = OS_Create_Window(Width, Height, WindowTitle, WindowFlags, Window);
#ifdef OS_WIN32
    Window->Display = GPU_Create_Display(DisplayManager, ((win32_window*)Window->Window)->Handle);
#else
#error Not Implemented
#endif
    return Window;
}

void Editor_Delete_Window(editor_window* Window)
{
    editor* Editor = Editor_Get();
    gpu_device_context*  DeviceGPU = Editor->DeviceGPU;
    gpu_display_manager* DisplayManager = GPU_Get_Display_Manager(DeviceGPU);
    
    GPU_Delete_Display(DisplayManager, Window->Display);
    OS_Delete_Window(Window->Window);
    SLL_Push_Front(Editor->FreeWindows, Window);
}

global editor* G_Editor;
void Editor_Set(editor* Editor)
{
    G_Editor = Editor;
    if(Editor)
    {
        Core_Set(Editor->Core);
    }
    else
    {
        Core_Set(NULL);
    }
}

editor* Editor_Get()
{
    return G_Editor;
}

#include <Core/core.c>
#include <Glyphs/glyphs.c>
#include <GPU/gpu.c>