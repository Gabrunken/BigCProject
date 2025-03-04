#ifndef material_h
#define material_h

#include <shaders.h>
#include <cglm/call.h>

typedef struct bigc_Material
{
	bigc_ShaderProgram* shaderReference;
	vec4 storedValues[BIGC_MAX_UNIFORM_COUNT]; //It is vec4 because it is the biggest material uniform i expect to be present in a shader (not counting matrices),
										  //so it can be used for any other data type, which is smaller (float, int, vec3...)
	mat4 storedMatrices[BIGC_MAX_UNIFORM_COUNT];
} bigc_Material;

void bigc_material_UploadDataToShader(const bigc_Material* material);
void bigc_material_UpdateIntOrBool(bigc_Material* material, const char* variableName, int value);
void bigc_material_UpdateFloat(bigc_Material* material, const char* variableName, float value);
void bigc_material_UpdateVec3(bigc_Material* material, const char* variableName, vec3 value);
void bigc_material_UpdateVec4(bigc_Material* material, const char* variableName, vec4 value);
void bigc_material_UpdateMat4(bigc_Material* material, const char* variableName, mat4 value);
#endif