#ifndef shader_h_
#define shader_h_

#include <cglm/call.h>

#define BIGC_MAX_UNIFORM_NAME_LENGTH 25
#define BIGC_MAX_UNIFORM_COUNT 20

typedef struct bigc_ShaderProgram
{
	unsigned int handle;
	char uniformNames[BIGC_MAX_UNIFORM_COUNT][BIGC_MAX_UNIFORM_NAME_LENGTH]; //Hash-map, 20 uniforms, 25 characters each MAX
	unsigned int uniformLocations[BIGC_MAX_UNIFORM_COUNT]; //Locations, matched up with names
	unsigned char uniformsStored; //Number of uniforms stored by this shader
} bigc_ShaderProgram;

#define BIGC_BAD_SHADER_PROGRAM (bigc_ShaderProgram){0, {}, {}, 0}

#define BIGC_MODEL_MATRIX_NAME "modelMatrix"
#define BIGC_VIEW_MATRIX_NAME "viewMatrix"
#define BIGC_PROJECTION_MATRIX_NAME "projectionMatrix"

bigc_ShaderProgram bigc_shaders_LoadFromDisk(const char* vertexShaderPath, const char* fragmentShaderPath);

void bigc_shaders_SetFloat(bigc_ShaderProgram* shader, const char* variableName, float value);
void bigc_shaders_SetVec3(bigc_ShaderProgram* shader, const char* variableName, vec3 value);
void bigc_shaders_SetVec4(bigc_ShaderProgram* shader, const char* variableName, vec4 value);
void bigc_shaders_SetMatrix(bigc_ShaderProgram* shader, const char* matrixName, mat4 value);

//Deletes shader program and also the handle given inside the struct gets set to 0.
void bigc_shaders_FreeFromGPU(bigc_ShaderProgram* shaderPointer);

#endif