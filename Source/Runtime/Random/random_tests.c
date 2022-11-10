UTEST(random, Random)
{
    for(uint64_t Seed = 0; Seed < 64; Seed++)
    {
        random32 Random32 = Random32_Init();
        for(uint64_t Index = 0; Index < 1000; Index++)
        {
            float UNorm = Random32_UNorm(&Random32);
            float SNorm = Random32_SNorm(&Random32);
            
            ASSERT_TRUE(UNorm >=  0.0f && UNorm <= 1.0f);
            ASSERT_TRUE(SNorm >= -1.0f && SNorm <= 1.0f);
            
            float F0 = Random32_SNorm(&Random32);
            float F1 = Random32_SNorm(&Random32);
            
            float MinV = Min(F0, F1);
            float MaxV = Max(F0, F1);
            float V = Random32_FBetween(&Random32, MinV, MaxV);
            
            ASSERT_TRUE(V >= MinV && V <= MaxV);
            
            int32_t I0 = Random32_Signed(&Random32);
            int32_t I1 = Random32_Signed(&Random32);                
            int32_t MinI = Min(I0, I1);
            int32_t MaxI = Max(I0, I1);
            int32_t I = Random32_SBetween(&Random32, MinI, MaxI);
            
            ASSERT_TRUE(I >= MinI && I <= MaxI);
        }
        
        random64 Random64 = Random64_Init();
        for(uint64_t Index = 0; Index < 1000; Index++)
        {
            double UNorm = Random64_UNorm(&Random64);
            double SNorm = Random64_SNorm(&Random64);
            
            ASSERT_TRUE(UNorm >=  0.0 && UNorm <= 1.0);
            ASSERT_TRUE(SNorm >= -1.0 && SNorm <= 1.0);
            
            double F0 = Random64_SNorm(&Random64);
            double F1 = Random64_SNorm(&Random64);
            
            double MinV = Min(F0, F1);
            double MaxV = Max(F0, F1);
            double V = Random64_FBetween(&Random64, MinV, MaxV);
            
            ASSERT_TRUE(V >= MinV && V <= MaxV);
            
            int64_t I0 = Random64_Signed(&Random64);
            int64_t I1 = Random64_Signed(&Random64);
            
            int64_t MinI = Min(I0, I1);
            int64_t MaxI = Max(I0, I1);
            int64_t I = Random64_SBetween(&Random64, MinI, MaxI);
            
            ASSERT_TRUE(I >= MinI && I <= MaxI);
        }
    }
}