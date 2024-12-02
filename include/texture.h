#ifndef texture_h_
#define texture_h_

typedef struct bigc_Texture
{
	unsigned int handle;
	unsigned char slotBoundTo;
} bigc_Texture;

#define BIGC_BAD_TEXTURE (bigc_Texture){0, 0}

//To be called after GL3W initialization and must be called before any Texture related call for them to work.
void bigc_texture_InitializeModule();

unsigned char bigc_texture_GetMaxTextureSlots();

bigc_Texture bigc_texture_LoadFromDisk(const char* filePath);

//Slot starts from 0
void bigc_texture_BindToSlot(bigc_Texture* textureToBind, unsigned char slot);

void bigc_texture_FreeFromGPU(bigc_Texture* texture);

#endif