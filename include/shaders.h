#ifndef shader_h_
#define shader_h_

#include <cglm/call.h>
#include <gl3w.h>

#define BIGC_MAX_UNIFORM_NAME_LENGTH 50
#define BIGC_MAX_UNIFORM_COUNT 20

typedef struct bigc_ShaderProgram
{
	uint16_t handle;
	char uniformNames[BIGC_MAX_UNIFORM_COUNT][BIGC_MAX_UNIFORM_NAME_LENGTH]; //Hash-map, 20 uniforms, 25 characters each MAX
	uint8_t uniformDataTypes[BIGC_MAX_UNIFORM_COUNT]; //data types in glsl code, vec3, vec2, float, bool, mat4...
	GLint uniformLocations[BIGC_MAX_UNIFORM_COUNT]; //Locations, matched up with names
	GLint uniformsStored; //Number of uniforms stored by this shader
} bigc_ShaderProgram;

#define BIGC_BAD_SHADER_PROGRAM (bigc_ShaderProgram){0, {}, {}, 0}

#define BIGC_MODEL_MATRIX_NAME "modelMatrix"

#define BIGC_GLSL_FLOAT 0
#define BIGC_GLSL_INT 1
#define BIGC_GLSL_VEC2 2
#define BIGC_GLSL_VEC3 3
#define BIGC_GLSL_VEC4 4
#define BIGC_GLSL_MAT4 5
#define BIGC_GLSL_ARRAY 6 //it is not made for matrix arrays
#define BIGC_GLSL_BOOL 7
#define BIGC_GLSL_STRUCT 8

bigc_ShaderProgram bigc_shaders_LoadFromDisk(const char* vertexShaderPath, const char* fragmentShaderPath);

void bigc_shaders_SetIntOrBool(const bigc_ShaderProgram* shader, const char* variableName, int value);
void bigc_shaders_SetFloat(const bigc_ShaderProgram* shader, const char* variableName, float value);
void bigc_shaders_SetVec2(const bigc_ShaderProgram* shader, const char* variableName, vec2 value);
void bigc_shaders_SetVec3(const bigc_ShaderProgram* shader, const char* variableName, vec3 value);
void bigc_shaders_SetVec4(const bigc_ShaderProgram* shader, const char* variableName, vec4 value);
void bigc_shaders_SetMat4(const bigc_ShaderProgram* shader, const char* matrixName, mat4 value);

//Deletes shader program and also the handle given inside the struct gets set to 0.
void bigc_shaders_FreeFromGPU(bigc_ShaderProgram* shaderPointer);

#endif