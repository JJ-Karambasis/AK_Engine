#ifndef UI_H
#define UI_H

struct ui;

#include "ui_common.h"
#include "ui_stack.h"
#include "ui_renderer.h"

typedef u32 ui_box_flags;

enum {
    UI_BOX_FLAG_FIXED_WIDTH_BIT  = (1 << 0),
    UI_BOX_FLAG_FIXED_HEIGHT_BIT = (1 << 1),
    UI_BOX_FLAG_DRAW_TEXT        = (1 << 2)
};

struct ui_render_box {
    rect2                      ScreenRect;
    rect2                      UVRect;
    vec4                       Color;
    vec2                       TextureDim;
    gdi_handle<gdi_bind_group> Texture;
};

struct ui_render_box_entry : ui_render_box {
    ui_render_box_entry* Next;
};

struct ui_text_glyph {
    rect2 ScreenRect;
    rect2 AtlasRect;
};

struct ui_text {
    uptr           Count;
    ui_text_glyph* Glyphs;
    string         Text;
};

struct ui;
struct ui_box;

#define UI_CUSTOM_RENDER_FUNC_DEFINE(name) void name(ui* UI, ui_box* Box, void* UserData)
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
    ui_key                 Key;
    ui_box_flags           Flags;
    u64                    LastUsedBuildIndex;
    vec2                   FixedSize;
    vec2                   FixedPosition;
    ui_size                PrefSize[UI_AXIS2_COUNT];
    ui_axis2               ChildLayoutAxis;
    vec4                   BackgroundColor;
    rect2                  Rect;
    font_id                FontID;
    ui_text                Text;
    ui_custom_render_func* CustomRenderFunc;
    void*                  RenderFuncUserData;
};

struct ui_create_info {
    allocator*   Allocator;
    glyph_cache* GlyphCache;
    renderer*    Renderer;
    ui_pipeline* Pipeline;
};

struct ui {
    //Main arena
    arena*  Arena;

    //Dependencies
    glyph_cache* GlyphCache;
    ui_renderer  Renderer;

    //Build arenas
    u64    BuildIndex;
    arena* BuildArenas[2];

    //Box cache
    ui_box* FirstFreeBox;
    hashmap<ui_key, ui_box*> BoxHashTable;

    //Build Hierarchy
    ui_box* Root;

    //Rendering
    u32                  RenderBoxCount;
    ui_render_box_entry* FirstRenderBox;
    ui_render_box_entry* LastRenderBox;
    ui_render_box_entry* CurrentRenderBox;

    //Stacks
    ui_stack_list Stacks[UI_STACK_TYPE_COUNT];
};

//Creation and deletion API
ui*  UI_Create(const ui_create_info& CreateInfo);
void UI_Delete(ui* UI);

//Build API
void UI_Begin_Build(ui* UI, window_handle Window);
void UI_End_Build(ui* UI);

//Cache lookup
ui_box* UI_Box_From_Key(ui* UI, ui_key Key);

//Box construction API
ui_box* UI_Build_Box_From_Key(ui* UI, ui_box_flags Flags, ui_key Key);
ui_box* UI_Build_Box_From_String(ui* UI, ui_box_flags Flags, string String);
ui_box* UI_Build_Box_From_StringF(ui* UI, ui_box_flags Flags, const char* Format, ...);

//Box set attachments API
void UI_Box_Attach_Custom_Render(ui_box* Box, ui_custom_render_func* RenderFunc, void* UserData);
void UI_Box_Attach_Display_Text(ui_box* Box, string Text);

//Box get attachments API
const ui_text* UI_Box_Get_Display_Text(ui_box* Box);

//Rendering API
ui_render_box* UI_Begin_Render_Box(ui* UI);
void           UI_End_Render_Box(ui* UI);

//UI push stack API
void UI_Push_Parent(ui* UI, ui_box* Box);
void UI_Push_Child_Layout_Axis(ui* UI, ui_axis2 Axis);
void UI_Push_Fixed_Width(ui* UI, f32 Width);
void UI_Push_Fixed_Height(ui* UI, f32 Height);
void UI_Push_Pref_Width(ui* UI, ui_size Size);
void UI_Push_Pref_Height(ui* UI, ui_size Size);
void UI_Push_Background_Color(ui* UI, vec4 Color);

//UI pop stack API
void UI_Pop_Parent(ui* UI);
void UI_Pop_Child_Layout_Axis(ui* UI);
void UI_Pop_Fixed_Width(ui* UI);
void UI_Pop_Fixed_Height(ui* UI);
void UI_Pop_Pref_Width(ui* UI);
void UI_Pop_Pref_Height(ui* UI);
void UI_Pop_Background_Color(ui* UI);

//UI autopop api
void UI_Set_Next_Parent(ui* UI, ui_box* Box);
void UI_Set_Next_Child_Layout_Axis(ui* UI, ui_axis2 Axis);
void UI_Set_Next_Fixed_Width(ui* UI, f32 Width);
void UI_Set_Next_Fixed_Height(ui* UI, f32 Height);
void UI_Set_Next_Pref_Width(ui* UI, ui_size Size);
void UI_Set_Next_Pref_Height(ui* UI, ui_size Size);
void UI_Set_Next_Background_Color(ui* UI, vec4 Color);

//UI get most recent stack api
ui_stack_parent*            UI_Current_Parent(ui* UI);
ui_stack_child_layout_axis* UI_Current_Child_Layout(ui* UI);
ui_stack_fixed_width*       UI_Current_Fixed_Width(ui* UI);
ui_stack_fixed_height*      UI_Current_Fixed_Height(ui* UI);
ui_stack_pref_width*        UI_Current_Pref_Width(ui* UI);
ui_stack_pref_height*       UI_Current_Pref_Height(ui* UI);
ui_stack_background_color*  UI_Current_Background_Color(ui* UI);


#endif