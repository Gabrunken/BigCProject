#include <material.h>
#include <shaders.h>
#include <bigc.h>
#include <stdlib.h>

void bigc_material_UploadDataToShader(const bigc_Material* material)
{
	//There should be something, for each shader's uniform, that checks if the material has it's counterpart stored, otherwise don't call any shader uniform function.
	//For now, i'll just use the material without before calling any shader function, everything will pass through material calls.
	for(uint8_t uniformIndex = 0; uniformIndex < material->shaderReference->uniformsStored; uniformIndex++) 
	{
		switch(material->shaderReference->uniformDataTypes[uniformIndex])
		{
		case GL_FLOAT:
			bigc_shaders_SetFloat(material->shaderReference, material->shaderReference->uniformNames[uniformIndex], material->storedValues[uniformIndex][0]);
			break;

		case GL_INT:
			bigc_shaders_SetIntOrBool(material->shaderReference, material->shaderReference->uniformNames[uniformIndex], material->storedValues[uniformIndex][0]);
			break;

		case GL_BOOL:
			bigc_shaders_SetIntOrBool(material->shaderReference, material->shaderReference->uniformNames[uniformIndex], material->storedValues[uniformIndex][0]);
			break;

		case GL_FLOAT_VEC2:
			bigc_shaders_SetVec2(material->shaderReference, material->shaderReference->uniformNames[uniformIndex],
				(vec2){material->storedValues[uniformIndex][0], material->storedValues[uniformIndex][1]});
			break;

		case GL_FLOAT_VEC3:
			bigc_shaders_SetVec3(material->shaderReference, material->shaderReference->uniformNames[uniformIndex],
				(vec3){material->storedValues[uniformIndex][0], material->storedValues[uniformIndex][1], material->storedValues[uniformIndex][2]});
			break;

		case GL_FLOAT_VEC4:
			bigc_shaders_SetVec4(material->shaderReference, material->shaderReference->uniformNames[uniformIndex], material->storedValues[uniformIndex]);
			break;

		case GL_FLOAT_MAT4:
			bigc_shaders_SetMat4(material->shaderReference, material->shaderReference->uniformNames[uniformIndex], material->storedMatrices[uniformIndex]);
			break;

		default:
			break;
		}
	}
}

void bigc_material_UpdateIntOrBool(bigc_Material* material, const char* variableName, int value)
{
	//Check before if the uniform actually exists in the shader's hash-map
	for(unsigned char i = 0; i < material->shaderReference->uniformsStored; i++)
	{
		//Check if the uniforms match by name
		if(strcmp(material->shaderReference->uniformNames[i], variableName) == 0)
		{
			//Now check if they the material's stored value matches with by value
			if(material->storedValues[i][0] != value)
			{
				material->storedValues[i][0] = value;
				bigc_shaders_SetIntOrBool(material->shaderReference, variableName, value);
				return;
			}

			//Otherwise, don't update the uniform
		}
	}
}

void bigc_material_UpdateFloat(bigc_Material* material, const char* variableName, float value)
{
	//Check before if the uniform actually exists in the shader's hash-map
	for(unsigned char i = 0; i < material->shaderReference->uniformsStored; i++)
	{
		//Check if the uniforms match by name
		if(strcmp(material->shaderReference->uniformNames[i], variableName) == 0)
		{
			//Now check if they the material's stored value matches with by value
			if(material->storedValues[i][0] != value)
			{
				material->storedValues[i][0] = value;
				bigc_shaders_SetFloat(material->shaderReference, variableName, value);
				return;
			}

			//Otherwise, don't update the uniform
		}
	}
}

void bigc_material_UpdateVec3(bigc_Material* material, const char* variableName, vec3 value)
{
	//Check before if the uniform actually exists in the shader's hash-map
	for(unsigned char i = 0; i < material->shaderReference->uniformsStored; i++)
	{
		//Check if the uniforms match by name
		if(strcmp(material->shaderReference->uniformNames[i], variableName) == 0)
		{
			//Now check if they the material's stored value matches with by value
			if( material->storedValues[i][0] != value[0] ||
				material->storedValues[i][1] != value[1] ||
				material->storedValues[i][2] != value[2])
			{
				glmc_vec3_copy(value, (vec3){material->storedValues[i][0], material->storedValues[i][1], material->storedValues[i][2]});
				bigc_shaders_SetVec3(material->shaderReference, variableName, value);
				return;
			}

			//Otherwise, don't update the uniform
		}
	}
}

void bigc_material_UpdateVec4(bigc_Material* material, const char* variableName, vec4 value)
{	
	//Check before if the uniform actually exists in the shader's hash-map
	for(unsigned char i = 0; i < material->shaderReference->uniformsStored; i++)
	{
		//Check if the uniforms match by name
		if(strcmp(material->shaderReference->uniformNames[i], variableName) == 0)
		{
			//Now check if they the material's stored value matches with by value
			if( material->storedValues[i][0] != value[0] ||
				material->storedValues[i][1] != value[1] ||
				material->storedValues[i][2] != value[2] ||
				material->storedValues[i][3] != value[3])
			{

				glmc_vec4_copy(value, material->storedValues[i]);
				bigc_shaders_SetVec4(material->shaderReference, variableName, value);
				return;
			}

			//Otherwise, don't update the uniform
		}
	}
}

void bigc_material_UpdateMat4(bigc_Material* material, const char* variableName, mat4 value)
{
	//Check before if the uniform actually exists in the shader's hash-map
	for(unsigned char i = 0; i < material->shaderReference->uniformsStored; i++)
	{
		//Check if the uniforms match by name
		if(strcmp(material->shaderReference->uniformNames[i], variableName) == 0)
		{
			//Now check if they the material's stored value matches with by value
			if( material->storedMatrices[i][0][0] != value[0][0] ||
				material->storedMatrices[i][0][1] != value[0][1] ||
				material->storedMatrices[i][0][2] != value[0][2] ||
				material->storedMatrices[i][0][3] != value[0][3] ||

				material->storedMatrices[i][1][0] != value[1][0] ||
				material->storedMatrices[i][1][1] != value[1][1] ||
				material->storedMatrices[i][1][2] != value[1][2] ||
				material->storedMatrices[i][1][3] != value[1][3] ||

				material->storedMatrices[i][2][0] != value[2][0] ||
				material->storedMatrices[i][2][1] != value[2][1] ||
				material->storedMatrices[i][2][2] != value[2][2] ||
				material->storedMatrices[i][2][3] != value[2][3] ||

				material->storedMatrices[i][3][0] != value[3][0] ||
				material->storedMatrices[i][3][1] != value[3][1] ||
				material->storedMatrices[i][3][2] != value[3][2] ||
				material->storedMatrices[i][3][3] != value[3][3])
			{
				glmc_mat4_copy(value, material->storedMatrices[i]);
				bigc_shaders_SetMat4(material->shaderReference, variableName, value);
				return;
			}

			//Otherwise, don't update the uniform
		}
	}
}