#ifndef ADI_H
#define ADI_H

#include <core.h>

struct adi_device {
    string Name;
};

adi*         ADI_Create(const adi_create_info& CreateInfo);
void         ADI_Delete(adi* ADI);
u32          ADI_Get_Device_Count(adi* ADI);
void         ADI_Get_Device(adi* ADI, adi_device* Device, u32 DeviceIndex);
void         ADI_Get_Default_Device(adi* ADI, adi_device* Device);
adi_context* ADI_Create_Context(adi* ADI, const adi_context_create_info& CreateInfo);

void ADI_Context_Delete(adi_context* Context);
bool ADI_Context_Start_Audio_Thread(adi_context* Context);
void ADI_Context_Stop_Audio_Thread(adi_context* Context);

#endif