#ifndef VECTOR_H
#define VECTOR_H

typedef union v2
{
    float Data[2];
    struct
    {
        float x;
        float y;
    };
} v2;

typedef union v2i
{
    int64_t Data[2];
    struct
    {
        int64_t x;
        int64_t y;
    };
} v2i;

typedef union v3
{
    float Data[3];
    struct
    {
        float x;
        float y;
        float z;
    };
} v3;

typedef union v3i
{
    int64_t Data[3];
    struct
    {
        int64_t x;
        int64_t y;
        int64_t z;
    };
} v3i;

typedef union v4
{
    float Data[4];
    struct
    {
        float x;
        float y;
        float z;
        float w;
    };
} v4;

typedef union v4i
{
    int64_t Data[4];
    struct
    {
        int64_t x;
        int64_t y;
        int64_t z;
        int64_t w;
    };
} v4i;

v2 V2(float x, float y);
v2i V2i(int64_t x, int64_t y);
v3 V3(float x, float y, float z);
v3i V3i(int64_t x, int64_t y, int64_t z);
v4 V4(float x, float y, float z, float w);
v4i V4i(int64_t x, int64_t y, int64_t z, int64_t w);

v2 V2_Mul_S(v2 V, float S);
v2 S_Div_V2(float S, v2 V);
v2 V2_Inv(v2 V);

#endif