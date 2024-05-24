#ifndef UI_BOX_SHADER_H
#define UI_BOX_SHADER_H

struct ui_global_data {
    matrix4 Projection;
};

struct ui_vertex {
    point2 P  SEMANTIC(Position0);
    point2 UV SEMANTIC(TexCoord0);
    color4 C  SEMANTIC(Color0);
};

#endif