#ifndef UI_BOX_SHADER_H
#define UI_BOX_SHADER_H

struct ui_box_shader_info {
    dim2 InvRes;
    dim2 InvTexRes;
};

struct ui_box_shader_box {
    point2 DstP0 SEMANTIC(Position0);
    point2 DstP1 SEMANTIC(Position1);
    point2 SrcP0 SEMANTIC(Position2);
    point2 SrcP1 SEMANTIC(Position3);
    color4 Color SEMANTIC(Color0);
};

#endif