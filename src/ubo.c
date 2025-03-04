#include <ubo.h>
#include <stdio.h>
#include <bigc.h>

static uint32_t maxUBOSize; //In bytes

void bigc_ubo_InitializeModule()
{
	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUBOSize);
}

bigc_UBO bigc_ubo_Create(uint8_t bindingSlot, uint32_t uboSize, uint8_t changeOverTime)
{
	if(uboSize > maxUBOSize)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("cannot create a uniform buffer object with that size");
		printf("\tSize: %llu\n\tMax size: %llu", uboSize, maxUBOSize);
		#endif

		return 0;
	}

	bigc_UBO ubo;
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);

	glBufferData(GL_UNIFORM_BUFFER, uboSize, NULL, changeOverTime ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);

	glBindBufferBase(GL_UNIFORM_BUFFER, bindingSlot, ubo);

	return ubo;
}

void bigc_ubo_LoadData(bigc_UBO targetUBO, const void* data, uint32_t dataSize)
{
	if(targetUBO == 0)
	{
		#ifdef DEBUG
		BIGC_LOG_WARNING("cannot load data to a null uniform buffer object");
		#endif

		return;
	}

	glBindBuffer(GL_UNIFORM_BUFFER, targetUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, dataSize, data);
}

void bigc_ubo_LoadSubData(bigc_UBO targetUBO, const void* data, uint32_t offset, uint32_t dataSize)
{
	if(targetUBO == 0)
	{
		#ifdef DEBUG
		BIGC_LOG_WARNING("cannot load data to a null uniform buffer object");
		#endif

		return;
	}

	glBindBuffer(GL_UNIFORM_BUFFER, targetUBO);
	glBufferSubData(GL_UNIFORM_BUFFER, offset, dataSize, data);
}

void bigc_ubo_FreeFromGPU(bigc_UBO* ubo)
{
	if(ubo == NULL)
	{
		#ifdef DEBUG
		BIGC_LOG_WARNING("you passed a NULL pointer to delete a uniform buffer object");
		#endif

		return;
	}

	if(ubo == 0)
	{
		#ifdef DEBUG
		BIGC_LOG_WARNING("cannot delete a null uniform buffer object");
		#endif

		return;
	}

	glDeleteBuffers(1, ubo);
}