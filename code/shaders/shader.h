#ifndef SHADER_H
#define SHADER_H

#define SHADER_VIEW_DATA_BIND_GROUP_INDEX 0
#define SHADER_DRAW_DATA_BIND_GROUP_INDEX 1
#define SHADER_TEXTURE_BIND_GROUP_INDEX 2  

struct view_data {
    matrix4 ViewProjection;
};

struct draw_data {
    matrix4_affine Model;
    vec4           Color;
};

#endif