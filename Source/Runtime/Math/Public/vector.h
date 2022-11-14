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

v2 V2(float x, float y);
v2i V2i(int64_t x, int64_t y);
v3 V3(float x, float y, float z);
v3i V3i(int64_t x, int64_t y, int64_t z);

#endif