#ifndef UI_BOX_SHADER_H
#define UI_BOX_SHADER_H

struct ui_box_shader_info {
    vec2 InvRes;
    vec2 InvTexRes;
};

struct ui_box_shader_box {
    vec2 DstP0 SEMANTIC(Position0);
    vec2 DstP1 SEMANTIC(Position1);
    vec2 SrcP0 SEMANTIC(Position2);
    vec2 SrcP1 SEMANTIC(Position3);
    vec4 Color SEMANTIC(Color0);
};

#endif