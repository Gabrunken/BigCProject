#ifndef ubo_h_
#define ubo_h_
#include <gl3w.h>

typedef GLint bigc_UBO;

bigc_UBO bigc_ubo_Create(uint8_t bindingSlot, uint32_t uboSize, uint8_t changeOverTime); 

void bigc_ubo_InitializeModule();
//these only work for std140 layout
void bigc_ubo_LoadData(bigc_UBO targetUBO, const void* data, uint32_t dataSize);
void bigc_ubo_LoadSubData(bigc_UBO targetUBO, const void* data, uint32_t offset, uint32_t dataSize);
void bigc_ubo_FreeFromGPU(bigc_UBO* ubo);

#endif