#ifndef model_h_
#define model_h_

#include <gl3w.h>

#define BIGC_MAX_VBO_PER_MODEL 8

//Maybe i'll have to rewrite these comments...
typedef struct bigc_VertexDataLayout
{
	unsigned char attributes; //number of attributes
	unsigned char attributeComponents[8]; //number of components for each attribute
	GLenum attributeTypes[8]; //Works with GL_FLOAT, GL_UNSIGNED_INT, ecc...
	unsigned int bytesToNextValues[8]; //Bytes to the next value (not component) of the same attribute type, for each attribute. AKA stride
	unsigned int bytesToNextAttributes[7]; //Bytes to the start of the next different attribute, for each attribute (ignoring the first considering it is at the start of the data array)
} bigc_VertexDataLayout;

typedef struct bigc_Model
{
	GLuint vertexArrayHandle;
	GLuint vertexBufferHandles[BIGC_MAX_VBO_PER_MODEL];
	GLuint indexBufferHandle;
	GLuint indicesCount;
	uint8_t attributes;
} bigc_Model;

#define BIGC_BAD_MODEL (bigc_Model){0, 0, 0, 0}

#define BIGC_VERTEX_POSITION 0x00000001
#define BIGC_VERTEX_UV 		 BIGC_VERTEX_POSITION<<1
#define BIGC_VERTEX_NORMAL   BIGC_VERTEX_POSITION<<2

bigc_Model bigc_model_SendToGPU(const void* vertexData, unsigned int bytesOfVertices,
					   const bigc_VertexDataLayout* vertexDataLayout,
					   const unsigned int* indexData, unsigned int bytesOfIndices);

void bigc_model_PushInstancedAttribute(bigc_Model* model, const void* data, uint32_t dataSize, uint8_t attributeComponents, GLenum componentType, uint32_t offsetToValue);

bigc_Model bigc_model_LoadOBJFromDisk(const char* filePath);

void bigc_model_FreeFromGPU(bigc_Model* model);

#endif