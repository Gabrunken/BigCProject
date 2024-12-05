#include <texture.h>
#include <gl3w.h>
#include <bigc.h>
#include <stb_image.h>

static unsigned char maxTextureSlots;
static unsigned char currentlyBoundSlot;

void bigc_texture_InitializeModule()
{
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (GLint*)&maxTextureSlots);
	//Tell stbi to flip images vertically, because opengl wants them in a certain order, which stbi does not have by default, but with this function it does.
	stbi_set_flip_vertically_on_load(BIGC_TRUE);
}

unsigned char bigc_texture_GetMaxTextureSlots()
{
	return maxTextureSlots;
}

bigc_Texture bigc_texture_LoadFromDisk(const char* filePath)
{
	//Load image from file
	int width, height;
	unsigned char* data = stbi_load(filePath, &width, &height, NULL, 4); //4 is the channels i request from stbi (RGBA)

	if(data == NULL)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("failed to load a file");
		printf("\tPath: %s\n", filePath);
		#endif
		return BIGC_BAD_TEXTURE;
	}

	//Generate texture object
	unsigned int textureHandle;
	glGenTextures(1, &textureHandle);
	glBindTexture(GL_TEXTURE_2D, textureHandle);

	//Set texture data to currently bound texture
	glTexImage2D(GL_TEXTURE_2D, 0,
		GL_RGBA, //in which format we want to store the texture (someone on discord said that making it 32 bit pixels is better for CPUs and GPUs, so i'll put every component, RGBA)
		width, height,
		0,
		GL_RGBA, //in which format the texture i loaded is (i always load an RGBA)
		GL_UNSIGNED_BYTE,
		data);

	//Let opengl generate mipmaps for this texture
	glGenerateMipmap(GL_TEXTURE_2D);

	//Free the image data in the CPU (it is now on the GPU)
	stbi_image_free(data);

	//Return a new Texture struct
	bigc_Texture texture = {textureHandle, 0};
	return texture;
}

void bigc_texture_BindToSlot(bigc_Texture* textureToBind, unsigned char slot)
{
	if(slot >= maxTextureSlots)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("you tried to bind to a texture slot which is to high, check out bigc_texture_GetMaxTextureSlots()");
		#endif
		return;
	}

	if(currentlyBoundSlot != slot)
	{
		//Bind the new texture slot
		glActiveTexture(GL_TEXTURE0 + slot);
		currentlyBoundSlot = slot;
	}

	//Bind the texture to the currently bound slot
	//Before check if the slot and the texture are not already bound together
	if(textureToBind->slotBoundTo != slot)
	{
		textureToBind->slotBoundTo = slot;
		glBindTexture(GL_TEXTURE_2D, textureToBind->handle);
	}
}

void bigc_texture_FreeFromGPU(bigc_Texture* texture)
{
	if(texture->handle != 0)
		glDeleteTextures(1, &texture->handle);
}