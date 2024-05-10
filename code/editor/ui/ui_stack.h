#ifndef UI_STACK_H
#define UI_STACK_H

struct ui_box;

enum ui_stack_type {
    UI_STACK_TYPE_PARENT,
    UI_STACK_TYPE_CHILD_LAYOUT_AXIS,
    UI_STACK_TYPE_FIXED_WIDTH,
    UI_STACK_TYPE_FIXED_HEIGHT,
    UI_STACK_TYPE_PREF_WIDTH,
    UI_STACK_TYPE_PREF_HEIGHT,
    UI_STACK_TYPE_BACKGROUND_COLOR,
    UI_STACK_TYPE_FONT,
    UI_STACK_TYPE_TEXT_COLOR,
    UI_STACK_TYPE_TEXT_ALIGNMENT,
    UI_STACK_TYPE_COUNT
};

struct ui_stack_entry {
    ui_stack_entry* Next;
    ui_stack_entry* Prev;
};

struct ui_stack_list {
    ui_stack_entry* First;
    ui_stack_entry* Last;
    b32             AutoPop;
};

struct ui_stack_parent : ui_stack_entry {
    ui_box* Value;
};

struct ui_stack_child_layout_axis : ui_stack_entry {
    ui_axis2 Value;
};

struct ui_stack_fixed_width : ui_stack_entry {
    f32 Value;
};

struct ui_stack_fixed_height : ui_stack_entry {
    f32 Value;
};

struct ui_stack_pref_width : ui_stack_entry {
    ui_size Value;
};

struct ui_stack_pref_height : ui_stack_entry {
    ui_size Value;
};

struct ui_stack_background_color : ui_stack_entry {
    vec4 Value;
};

struct ui_stack_font : ui_stack_entry {
    ui_font Value;
};

struct ui_stack_text_color : ui_stack_entry {
    vec4 Value;
};

struct ui_stack_text_alignment : ui_stack_entry {
    ui_text_alignment_flags Value;
};

#endif