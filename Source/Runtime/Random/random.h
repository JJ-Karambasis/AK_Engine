#ifndef RANDOM_H
#define RANDOM_H

typedef struct random32
{
    uint32_t Seed;
} random32;

typedef struct random64
{
    uint64_t Seed;
} random64;

random32 Random32_Init_Seed(uint32_t Seed);
random32 Random32_Init();
uint32_t Random32_Unsigned(random32* Random);
int32_t  Random32_Signed(random32* Random);
float    Random32_UNorm(random32* Random);
float    Random32_SNorm(random32* Random);
int32_t  Random32_SBetween(random32* Random, int32_t Min, int32_t Max);
float    Random32_FBetween(random32* Random, float Min, float Max);

random64 Random64_Init_Seed(uint64_t Seed);
random64 Random64_Init();
uint64_t Random64_Unsigned(random64* Random);
int64_t  Random64_Signed(random64* Random);
double   Random64_UNorm(random64* Random);
double   Random64_SNorm(random64* Random);
int64_t  Random64_SBetween(random64* Random, int64_t Min, int64_t Max);
double   Random64_FBetween(random64* Random, double Min, double Max);

#endif
