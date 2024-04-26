#ifndef UI_BOX_SHADER_H
#define UI_BOX_SHADER_H

#define UI_BIND_GROUP_GLOBAL_INDEX 0
#define UI_BIND_GROUP_TEXTURE_INDEX 1
#define UI_BIND_GROUP_DYNAMIC_INDEX 2
#define UI_BIND_GROUP_COUNT 3

#define UI_TEXTURE_BIND_GROUP_TEXTURE_BINDING 0
#define UI_TEXTURE_BIND_GROUP_SAMPLER_BINDING 1
#define UI_TEXTURE_BIND_GROUP_BINDING_COUNT 2

struct ui_box_shader_global {
    vec2 InvRes;
    vec2 InvTexRes;
};

struct ui_box_shader_dynamic {
    vec2 DstP0;
    vec2 DstP1;
    vec2 SrcP0;
    vec2 SrcP1;
    vec4 Color;
};

#endif