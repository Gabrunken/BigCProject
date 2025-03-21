#include <camera.h>
#include <transform.h>
#include <ubo.h>

extern mat4 bigc_viewMatrix;
extern mat4 bigc_projectionMatrix;

static bigc_Transform transform;

extern bigc_UBO bigc_viewProjectionUBO;

void bigc_camera_SetPerspectiveProjection(unsigned char FOV, float screenRatio, float nearPlane, float farPlane)
{
	glmc_perspective(glm_rad(FOV), //FOV
					 screenRatio, //aspect ratio
					 nearPlane, //near plane
					 farPlane, //far plane
					 bigc_projectionMatrix //dest
					 );
}

void bigc_camera_SetPosition(vec3 newPosition)
{
	glmc_vec3_copy(newPosition, transform.position);
}

void bigc_camera_MoveByAmount(vec3 moveAmount)
{
	glmc_vec3_add(transform.position, moveAmount, transform.position);
}

void bigc_camera_SetRotation(vec3 newRotation)
{
	glmc_vec3_copy(newRotation, transform.rotation);
}

void bigc_camera_RotateByAmount(vec3 rotateAmount)
{
	glmc_vec3_add(transform.rotation, rotateAmount, transform.rotation);
}

void bigc_camera_LoadViewProjectionMatrices()
{
	glmc_mat4_identity(bigc_viewMatrix);
	glmc_translate(bigc_viewMatrix, transform.position);
	glmc_rotate_x(bigc_viewMatrix, transform.rotation[0], bigc_viewMatrix);
	glmc_rotate_y(bigc_viewMatrix, transform.rotation[1], bigc_viewMatrix);
	glmc_rotate_z(bigc_viewMatrix, transform.rotation[2], bigc_viewMatrix);

	mat4 data[2];
	memcpy(data, bigc_projectionMatrix, sizeof(mat4));
	memcpy(data+1, bigc_viewMatrix, sizeof(mat4));

	//Upload to UBO
	bigc_ubo_LoadData(bigc_viewProjectionUBO, (void*)data, sizeof(mat4)*2);
}

const bigc_Transform* bigc_camera_GetTransform()
{
	return &transform;
}