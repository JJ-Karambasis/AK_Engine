#ifndef UI_H
#define UI_H

struct ui;

#include "ui_common.h"
#include "ui_stack.h"

typedef u32 ui_box_flags;

enum {
    UI_BOX_FLAG_FIXED_WIDTH_BIT  = (1 << 0),
    UI_BOX_FLAG_FIXED_HEIGHT_BIT = (1 << 1),
    UI_BOX_FLAG_DRAW_TEXT        = (1 << 2),
    UI_BOX_FLAG_MOUSE_CLICKABLE  = (1 << 3)
};

struct ui_render_box {
    rect2                      ScreenRect;
    rect2                      UVRect;
    color4                     Color;
    dim2                       TextureDim;
    gdi_handle<gdi_bind_group> Texture;
};

struct ui_render_box_entry : ui_render_box {
    ui_render_box_entry* Next;
};

struct ui_text_glyph {
    u32   Codepoint;
    rect2 ScreenRect;
};

struct ui_text {
    uptr           Count;
    ui_text_glyph* Glyphs;
    string         Text;
    ui_font        Font;
    dim2           Dim;
};

struct ui;
struct ui_box;

#define UI_CUSTOM_RENDER_FUNC_DEFINE(name) void name(ui* UI, ui_box* Box, im_renderer* Renderer, void* UserData)
typedef UI_CUSTOM_RENDER_FUNC_DEFINE(ui_custom_render_func);

struct ui_box {
    //Hash state
    ui_box* HashNext;
    ui_box* HashPrev;

    //Hierarchy state
    ui_box* FirstChild;
    ui_box* LastChild;
    ui_box* NextSibling;
    ui_box* PrevSibling;
    ui_box* Parent;
    u32     ChildCount;

    //Per build info
    ui_key                  Key;
    ui_box_flags            Flags;
    u64                     LastUsedBuildIndex;
    dim2                    FixedSize;
    point2                  FixedPosition;
    ui_size                 PrefSize[UI_AXIS2_COUNT];
    ui_axis2                ChildLayoutAxis;
    color4                  BackgroundColor;
    color4                  TextColor;
    rect2                   Rect;
    ui_font                 Font;
    ui_text                 Text;
    ui_text_alignment_flags TextAlignment;
    ui_custom_render_func*  CustomRenderFunc;
    void*                   RenderFuncUserData;
};

enum {
    UI_SIGNAL_FLAG_LEFT_PRESSED_BIT  = (1 << 0),
    UI_SIGNAL_FLAG_LEFT_RELEASED_BIT = (1 << 1),
    UI_SIGNAL_FLAG_LEFT_DOWN_BIT     = (1 << 2),
    UI_SIGNAL_FLAG_HOVERING_BIT      = (1 << 3),
    UI_SIGNAL_FLAG_DRAGGING_BIT      = (1 << 4)
};
typedef u32 ui_signal_flags;
struct ui_signal {
    ui_box*         Box;
    ui_signal_flags Flags;
};

#define UI_Pressed(signal)  ((signal).Flags & UI_SIGNAL_FLAG_LEFT_PRESSED_BIT)
#define UI_Released(signal) ((signal).Flags & UI_SIGNAL_FLAG_LEFT_RELEASED_BIT)
#define UI_Down(signal)     ((signal).Flags & UI_SIGNAL_FLAG_LEFT_DOWN_BIT)
#define UI_Hovering(signal) ((signal).Flags & UI_SIGNAL_FLAG_HOVERING_BIT)
#define UI_Dragging(signal) ((signal).Flags & UI_SIGNAL_FLAG_DRAGGING_BIT)

struct ui_create_info {
    allocator*                        Allocator;
    renderer*                         Renderer;
    glyph_cache*                      GlyphCache;
    gdi_handle<gdi_pipeline>          Pipeline;
    gdi_handle<gdi_bind_group_layout> GlobalLayout;
};

struct ui {
    //Main arena
    arena*  Arena;

    //Dependencies
    glyph_cache*          GlyphCache;

    //Window
    window* Window;

    //Rendering
    im_renderer*               Renderer;
    gdi_handle<gdi_pipeline>   Pipeline;
    gdi_handle<gdi_bind_group> GlobalBindGroup;
    gdi_handle<gdi_buffer>     GlobalBuffer;

    //Build arenas
    u64    BuildIndex;
    arena* BuildArenas[2];

    //Box cache
    ui_box* FirstFreeBox;
    hashmap<ui_key, ui_box*> BoxHashTable;

    //Build Hierarchy
    ui_box* Root;


    //Current box
    ui_box* CurrentBox;

    //Cached signal
    ui_signal CachedSignal;

    //Stacks
    ui_stack_list Stacks[UI_STACK_TYPE_COUNT];
};

//Creation and deletion API
ui*  UI_Create(const ui_create_info& CreateInfo);
void UI_Delete(ui* UI);

//Build API
void UI_Begin_Build(ui* UI, window* Window);
void UI_End_Build(ui* UI);

//Cache lookup
ui_box* UI_Box_From_Key(ui* UI, ui_key Key);
ui_box* UI_Current_Box(ui* UI);

//Box construction API
ui_box* UI_Build_Box_From_Key(ui* UI, ui_box_flags Flags, ui_key Key);
ui_box* UI_Build_Box_From_String(ui* UI, ui_box_flags Flags, string String);
ui_box* UI_Build_Box_From_StringF(ui* UI, ui_box_flags Flags, const char* Format, ...);

//Box set attachments API
void UI_Box_Attach_Custom_Render(ui_box* Box, ui_custom_render_func* RenderFunc, void* UserData);
void UI_Box_Attach_Display_Text(ui* UI, ui_box* Box, string Text);

//Box get attachments API
const ui_text* UI_Box_Get_Display_Text(ui_box* Box);
dim2           UI_Box_Get_Dim(ui_box* Box);

//Box interaction api
ui_signal UI_Signal_From_Box(ui* UI, ui_box* Box);
ui_signal UI_Current_Signal(ui* UI);

#define UI_Box_Get_Height(box) UI_Box_Get_Dim(box).height
#define UI_Box_Get_Width(box) UI_Box_Get_Dim(box).width

//UI push stack API
void UI_Push_Parent(ui* UI, ui_box* Box);
void UI_Push_Child_Layout_Axis(ui* UI, ui_axis2 Axis);
void UI_Push_Fixed_Width(ui* UI, f32 Width);
void UI_Push_Fixed_Height(ui* UI, f32 Height);
void UI_Push_Pref_Width(ui* UI, ui_size Size);
void UI_Push_Pref_Height(ui* UI, ui_size Size);
void UI_Push_Background_Color(ui* UI, color4 Color);
void UI_Push_Font(ui* UI, ui_font Font);
void UI_Push_Text_Color(ui* UI, color4 Color);
void UI_Push_Text_Alignment(ui* UI, ui_text_alignment_flags AlignmentFlags);

//UI pop stack API
void UI_Pop_Parent(ui* UI);
void UI_Pop_Child_Layout_Axis(ui* UI);
void UI_Pop_Fixed_Width(ui* UI);
void UI_Pop_Fixed_Height(ui* UI);
void UI_Pop_Pref_Width(ui* UI);
void UI_Pop_Pref_Height(ui* UI);
void UI_Pop_Background_Color(ui* UI);
void UI_Pop_Font(ui* UI);
void UI_Pop_Text_Color(ui* UI);
void UI_Pop_Text_Alignment(ui* UI);

//UI autopop api
void UI_Set_Next_Parent(ui* UI, ui_box* Box);
void UI_Set_Next_Child_Layout_Axis(ui* UI, ui_axis2 Axis);
void UI_Set_Next_Fixed_Width(ui* UI, f32 Width);
void UI_Set_Next_Fixed_Height(ui* UI, f32 Height);
void UI_Set_Next_Pref_Width(ui* UI, ui_size Size);
void UI_Set_Next_Pref_Height(ui* UI, ui_size Size);
void UI_Set_Next_Background_Color(ui* UI, color4 Color);
void UI_Set_Next_Font(ui* UI, ui_font Font);
void UI_Set_Next_Text_Color(ui* UI, color4 Color);
void UI_Set_Next_Text_Alignment(ui* UI, ui_text_alignment_flags AlignmentFlags);

//UI get most recent stack api
ui_stack_parent*            UI_Current_Parent(ui* UI);
ui_stack_child_layout_axis* UI_Current_Child_Layout(ui* UI);
ui_stack_fixed_width*       UI_Current_Fixed_Width(ui* UI);
ui_stack_fixed_height*      UI_Current_Fixed_Height(ui* UI);
ui_stack_pref_width*        UI_Current_Pref_Width(ui* UI);
ui_stack_pref_height*       UI_Current_Pref_Height(ui* UI);
ui_stack_background_color*  UI_Current_Background_Color(ui* UI);
ui_stack_font*              UI_Current_Font(ui* UI);    
ui_stack_text_color*        UI_Current_Text_Color(ui* UI); 
ui_stack_text_alignment*    UI_Current_Text_Alignment(ui* UI);            

#endif