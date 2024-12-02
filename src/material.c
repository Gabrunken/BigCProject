#include <material.h>
#include <bigc.h>
#include <stdlib.h>

void bigc_material_UploadDataToShader(const bigc_Material* material)
{
	//There should be something, for each shader's uniform, that checks if the material has it's counterpart stored, otherwise don't call any shader uniform function.
	//For now, i'll just use the material without before calling any shader function, everything will pass through material calls.
	for(unsigned char i = 0; i < material->shaderReference->uniformsStored; i++) 
	{
		switch(material->storedValuesDataTypes[i])
		{
		case BIGC_FLOAT_UNIFORM:
			bigc_shaders_SetFloat(material->shaderReference, material->shaderReference->uniformNames[i], material->storedValues[i][0]);
			break;

		case BIGC_INT_UNIFORM:
			break;

		case BIGC_BOOLEAN_UNIFORM:
			break;

		case BIGC_VECTOR2_UNIFORM:
			break;

		case BIGC_VECTOR3_UNIFORM:
			bigc_shaders_SetVec3(material->shaderReference, material->shaderReference->uniformNames[i],
				(vec3){material->storedValues[i][0], material->storedValues[i][1], material->storedValues[i][2]});
			break;

		case BIGC_VECTOR4_UNIFORM:
			bigc_shaders_SetVec4(material->shaderReference, material->shaderReference->uniformNames[i], material->storedValues[i]);
			break;

		case BIGC_MATRIX_UNIFORM:
			bigc_shaders_SetMatrix(material->shaderReference, material->shaderReference->uniformNames[i], material->storedMatrices[i]);
			break;

		default:
			break;
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
				material->storedValuesDataTypes[i] = BIGC_FLOAT_UNIFORM;
				bigc_shaders_SetFloat(material->shaderReference, variableName, value);
			}

			//Otherwise, don't update the uniform
		}
	}

	//if we are in the end, and it didn't find it, set it in the shader, so we create a new uniform in the shader
	material->storedValues[material->shaderReference->uniformsStored][0] = value;
	material->storedValuesDataTypes[material->shaderReference->uniformsStored] = BIGC_FLOAT_UNIFORM;
	bigc_shaders_SetFloat(material->shaderReference, variableName, value);
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
				material->storedValuesDataTypes[i] = BIGC_VECTOR3_UNIFORM;
				bigc_shaders_SetVec3(material->shaderReference, variableName, value);
			}

			//Otherwise, don't update the uniform
		}
	}

	//if we are in the end, and it didn't find it, set it in the shader, so we create a new uniform in the shader
	glmc_vec3_copy(value, (vec3){material->storedValues[material->shaderReference->uniformsStored][0],
		material->storedValues[material->shaderReference->uniformsStored][1],
		material->storedValues[material->shaderReference->uniformsStored][2]});
	material->storedValuesDataTypes[material->shaderReference->uniformsStored] = BIGC_VECTOR3_UNIFORM;
	bigc_shaders_SetVec3(material->shaderReference, variableName, value);
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
				material->storedValuesDataTypes[i] = BIGC_VECTOR4_UNIFORM;
				bigc_shaders_SetVec4(material->shaderReference, variableName, value);
				return;
			}

			//Otherwise, don't update the uniform
		}
	}

	//if we are in the end, and it didn't find it, set it in the shader, so we create a new uniform in the shader
	glmc_vec4_copy(value, material->storedValues[material->shaderReference->uniformsStored]);
	material->storedValuesDataTypes[material->shaderReference->uniformsStored] = BIGC_VECTOR4_UNIFORM;
	bigc_shaders_SetVec4(material->shaderReference, variableName, value);
}

void bigc_material_UpdateMatrix(bigc_Material* material, const char* variableName, mat4 value)
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
				material->storedValuesDataTypes[i] = BIGC_MATRIX_UNIFORM;
				bigc_shaders_SetMatrix(material->shaderReference, variableName, value);
				return;
			}

			//Otherwise, don't update the uniform
		}
	}

	//if we are in the end, and it didn't find it, set it in the shader, so we create a new uniform in the shader
	glmc_mat4_copy(value, material->storedMatrices[material->shaderReference->uniformsStored]);
	material->storedValuesDataTypes[material->shaderReference->uniformsStored] = BIGC_MATRIX_UNIFORM;
	bigc_shaders_SetMatrix(material->shaderReference, variableName, value);
}