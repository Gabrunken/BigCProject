#include <prop.h>

extern mat4 bigc_modelMatrix;
extern mat4 bigc_viewMatrix;
extern mat4 bigc_projectionMatrix;

void bigc_prop_Initialize(bigc_Prop* prop, const bigc_Model* model, const bigc_Material* material)
{
	prop->transform.scale[0] =
	prop->transform.scale[1] =
	prop->transform.scale[2] = 1.0f;

	prop->transform.position[0] =
	prop->transform.position[1] =
	prop->transform.position[2] = 0.0f;

	prop->transform.rotation[0] =
	prop->transform.rotation[1] =
	prop->transform.rotation[2] = 0.0f;

	prop->modelReference = model;
	prop->materialReference = material;
}

void bigc_prop_SetPosition(bigc_Prop* prop, vec3 newPosition)
{
	glmc_vec3_copy(newPosition, prop->transform.position);
}

void bigc_prop_SetRotation(bigc_Prop* prop, vec3 newRotation)
{
	glmc_vec3_copy(newRotation, prop->transform.rotation);
}

void bigc_prop_SetScale(bigc_Prop* prop, vec3 newScale)
{
	glmc_vec3_copy(newScale, prop->transform.scale);
}

void bigc_prop_Move(bigc_Prop* prop, vec3 movement)
{
	glmc_vec3_add(prop->transform.position, movement, prop->transform.position);
}

void bigc_prop_Rotate(bigc_Prop* prop, vec3 rotation)
{
	glmc_vec3_add(prop->transform.rotation, rotation, prop->transform.rotation);
}

void bigc_prop_Scale(bigc_Prop* prop, vec3 scale)
{
	glmc_vec3_add(prop->transform.scale, scale, prop->transform.scale);
}

void bigc_prop_LoadToMVP(bigc_Prop* prop)
{
	//make model matrix
	glmc_mat4_identity(bigc_modelMatrix);
	glmc_translate(bigc_modelMatrix, prop->transform.position);
	glmc_rotate_x(bigc_modelMatrix, prop->transform.rotation[0], bigc_modelMatrix);
	glmc_rotate_y(bigc_modelMatrix, prop->transform.rotation[1], bigc_modelMatrix);
	glmc_rotate_z(bigc_modelMatrix, prop->transform.rotation[2], bigc_modelMatrix);
	glmc_scale(bigc_modelMatrix, prop->transform.scale);
	
	//update the mvp in shader
	bigc_material_UpdateMatrix(prop->materialReference, BIGC_MODEL_MATRIX_NAME, bigc_modelMatrix);	
	bigc_material_UpdateMatrix(prop->materialReference, BIGC_VIEW_MATRIX_NAME, bigc_viewMatrix);	
	bigc_material_UpdateMatrix(prop->materialReference, BIGC_PROJECTION_MATRIX_NAME, bigc_projectionMatrix);	
}