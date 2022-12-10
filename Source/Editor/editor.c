#include "editor.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

static gpu_init*       GPU_Init;
static gpu_shutdown*   GPU_Shutdown;
static gpu_reload*     GPU_Reload;
static gpu_set_device* GPU_Set_Device;

void Draw_Text(gpu_ui_pass* UIPass, gpu_sampler* Sampler, glyph_font* Font, glyph_cache* GlyphCache, str8 Text, v2 PixelP, uint32_t PixelHeight, v4 Color)
{
    arena* Scratch = Core_Get_Thread_Context()->Scratch;
    bidi BIDI = BIDI_Get_Parts(Scratch, Text);
    
    Text = BIDI_Replace_Text(&BIDI, Text, Scratch);
    
    v2 InitialP = PixelP;
    
    bidi_part_list* Parts = &BIDI.Parts;
    for(bidi_part* Part = Parts->First; Part; Part = Part->Next)
    {
        if(Part->Type == BIDI_PART_TYPE_TEXT)
        {
            uint32_t FirstCodepoint = UTF8_Read(Text.Str + Part->Offset, NULL);
            bidi_part_text* PartText = (bidi_part_text*)Part;
            
            shape_list Shapes = Text_Shaper_Shape(Font->Shaper, Get_Base_Allocator(Scratch), Text, Part->Offset, 
                                                  Part->Length, PartText->Direction, PartText->Script, StrC_Empty(), PixelHeight);
            
            for(uint64_t ShapeIndex = 0; ShapeIndex < Shapes.Count; ShapeIndex++)
            {
                shape* Shape = Shapes.Ptr + ShapeIndex;
                if(Shape->Codepoint)
                {
                    const glyph* Glyph = Glyph_Cache_Get_Glyph(GlyphCache, Font->Face, Shape->Codepoint, PixelHeight);
                    if(Glyph && Glyph->Texture.Texture)
                    {
                        const glyph_texture* Texture = &Glyph->Texture;
                        
                        gpu_texture_unit TextureUnit;  
                        TextureUnit.Texture = Texture->Texture;
                        TextureUnit.Sampler = Sampler;
                        
                        v2 P = V2_Add_V2(PixelP, V2((float)Shape->XOffset+Glyph->XBearing, -(float)(Shape->YOffset+Glyph->YBearing)));
                        GPU_UI_Pass_Draw_Rectangle(UIPass, P, V2_Add_V2(P, V2((float)Texture->Width, (float)Texture->Height)), TextureUnit, Color);
                    }
                    
                    PixelP.x += Shape->XAdvance;
                    PixelP.y += Shape->YAdvance;
                }
            }
        }
        else if(Part->Type == BIDI_PART_TYPE_NEWLINE)
        {
            //TODO(JJ): Yeah obviously this isn't how this should work at all
            PixelP.y += PixelHeight;
            PixelP.x = InitialP.x; 
        }
    }
}

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
    
    Editor->MainWindow = Editor_Create_Window(1280, 720, Str8_Lit("AK Engine"), 0);
    Editor->EngineConsole = Engine_Console_Create(Get_Base_Allocator(Editor->Arena));
    Editor->FontLoader = OS_Font_Loader_Create(Get_Base_Allocator(Editor->Arena));
    Editor->GlyphGenerator = FT_Glyph_Generator_Create(Get_Base_Allocator(Editor->Arena));
    Editor->GlyphCache = Glyph_Cache_Create(Get_Base_Allocator(Editor->Arena), GPU_Get_Resource_Manager(Editor->DeviceGPU), 1024);
    
    return Editor;
}

void Editor_Update()
{
    editor* Editor = Editor_Get();
    
    arena* Scratch = Core_Get_Thread_Context()->Scratch;
    
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
    
    static char* CharAt;
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
                
                case OS_EVENT_TYPE_KEY_PRESSED:
                {
                    OS_Debug_Log(Str8_Lit("Pressed %d\n"), Event->KeyPressed.Keycode);
                } break;
                
                case OS_EVENT_TYPE_KEY_RELEASED:
                {
                    OS_Debug_Log(Str8_Lit("Released %d\n"), Event->KeyPressed.Keycode);
                } break;
                
                case OS_EVENT_TYPE_TEXT_INPUT:
                {
                    uint32_t Codepoint = Event->TextInputUTF32;
                    OS_Debug_Log(Str8_Lit("Char %d\n"), Codepoint);
                } break;
            }
            
            Event = OS_Get_Next_Event();
        }
        
        GPU_Cmd_Buffer_Reset(CmdBuffer);
        
        gpu_display* MainDisplay = Editor->MainWindow->Display;
        
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
        
        GPU_Cmd_Copy_Texture_To_Display(CmdBuffer, MainDisplay, 0, 0, RenderTarget, 0, 0, Safe_S64_U32(WindowDim.x), Safe_S64_U32(WindowDim.y));
        
        Glyph_Cache_Generate(Editor->GlyphCache, CmdBuffer);
        
        GPU_Dispatch_Cmds(DeviceGPU, &CmdBuffer, 1);
        
        GPU_Present_Displays(DisplayManager, &MainDisplay, 1);
        
        Arena_Clear(Scratch, MEMORY_NO_CLEAR);
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
#include <Engine_Console/engine_console.c>
#include <Fonts/fonts.c>
#include <GPU/gpu.c>