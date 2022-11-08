random32 Random32_Init_Seed(uint32_t Seed)
{
    random32 Result;
    Zero_Struct(&Result, random32);
    Result.Seed = Seed;
    return Result;
}

random32 Random32_Init()
{
    uint32_t Seed;
    OS_Get_Random_Seed(&Seed, sizeof(uint32_t));
    return Random32_Init_Seed(Seed);
}

uint32_t Random32_Unsigned(random32* Random)
{
    uint32_t Result = Random->Seed;
    Result ^= Result << 13;
    Result ^= Result >> 17;
    Result ^= Result << 5;
    Random->Seed = Result;
    return Result;
}

int32_t Random32_Signed(random32* Random)
{
    return (int32_t)Random32_Unsigned(Random);
}

float Random32_UNorm(random32* Random)
{
    float Divisor = 1.0f / (float)0xFFFFFFFF;
    float Result = Divisor*(float)Random32_Unsigned(Random);
    return Result;
}

float Random32_SNorm(random32* Random)
{
    float Result = 2.0f*Random32_UNorm(Random) - 1.0f;
    return Result;
}

int32_t Random32_SBetween(random32* Random, int32_t Min, int32_t Max)
{
    int32_t Denominator = ((Max+1) - Min);
    Assert(Denominator != 0);
    int32_t Result = Min + (int32_t)(Random32_Unsigned(Random) % Denominator);
    return Result;
}

float Random32_FBetween(random32* Random, float Min, float Max)
{
    float Result = Lerp_F32(Min, Random32_UNorm(Random), Max);
    return Result;
}

random64 Random64_Init_Seed(uint64_t Seed)
{
    random64 Result;
    Zero_Struct(&Result, random64);
    Result.Seed = Seed;
    return Result;
}

random64 Random64_Init()
{
    uint64_t Seed;
    OS_Get_Random_Seed(&Seed, sizeof(uint64_t));
    return Random64_Init_Seed(Seed);
}

uint64_t Random64_Unsigned(random64* Random)
{
    uint64_t Result = Random->Seed;
    Result ^= Result << 13;
    Result ^= Result >> 7;
    Result ^= Result << 17;
    Random->Seed = Result;
    return Result;
}

int64_t Random64_Signed(random64* Random)
{
    return (int64_t)Random64_Unsigned(Random);
}

double Random64_UNorm(random64* Random)
{
    double Divisor = 1.0 / (double)0xFFFFFFFFFFFFFFFF;
    double Result = Divisor*(double)Random64_Unsigned(Random);
    return Result;
}

double Random64_SNorm(random64* Random)
{
    double Result = 2.0*Random64_UNorm(Random) - 1.0;
    return Result;
}

int64_t Random64_SBetween(random64* Random, int64_t Min, int64_t Max)
{
    int64_t Result = Min + (int64_t)(Random64_Unsigned(Random) % ((Max+1) - Min));
    return Result;
}

double Random64_FBetween(random64* Random, double Min, double Max)
{
    double Result = Lerp_F64(Min, Random64_UNorm(Random), Max);
    return Result;
}