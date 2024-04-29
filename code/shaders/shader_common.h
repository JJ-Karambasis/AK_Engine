#ifndef SHADER_COMMON_H
#define SHADER_COMMON_H

#ifndef __cplusplus
#define u32 uint
#define b32 int
#define f32 float
#define vec2 float2
#define vec3 float3
#define vec4 float4
#define matrix3 float3x3
#define matrix4 float4x4
#define matrix4_affine float4x3
#define PI 3.14159265359f
#define INV_PI 0.31830988618f
#define SPACE_(value) space ## value
#define SPACE(value) SPACE_(value)
#define BUFFER_(value) b ## value
#define BUFFER(value) BUFFER_(value)
#define TEXTURE_(value) t ## value
#define TEXTURE(value) TEXTURE_(value)
#define SAMPLER_(value) s ## value
#define SAMPLER(value) SAMPLER_(value)
#define SEMANTIC(name) : name
#else
#define SEMANTIC(name)
#endif

#endif