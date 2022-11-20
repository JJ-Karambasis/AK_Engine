#include "containers.h"
#include "containers.c"

UTEST(ArrayTest, Arrays)
{
    custom_allocator* Allocator = Allocate_Custom_Allocator();
    
    s32 Data[4] = {0, 1, 2, 3};
    s32_array Array = Array_Expand(Data);
    s32 E = Array_Get_Value_S32(&Array, 2);
    
    s32_array Array2 = Array_Allocate_S32(Get_Base_Allocator(Allocator), 4);
    
}