#ifndef UI_BOX_SHADER_H
#define UI_BOX_SHADER_H

#define UI_BIND_GROUP_GLOBAL_INDEX 0
#define UI_BIND_GROUP_DYNAMIC_INDEX 1
#define UI_BIND_GROUP_COUNT 2

#define UI_TEXTURE_BIND_GROUP_TEXTURE_BINDING 0
#define UI_TEXTURE_BIND_GROUP_SAMPLER_BINDING 1
#define UI_TEXTURE_BIND_GROUP_BINDING_COUNT 2

struct ui_box_shader_global {
    vec2 InvRes;
};

struct ui_box_shader_dynamic {
    vec2 P1;
    vec2 P2;
    vec4 Color;
};

#endif