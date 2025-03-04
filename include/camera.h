#ifndef camera_h_
#define camera_h_
#include <cglm/call.h>
#include <transform.h>

void bigc_camera_SetPerspectiveProjection(unsigned char FOV, float screenRatio, float nearPlane, float farPlane);

void bigc_camera_SetPosition(vec3 newPosition);

void bigc_camera_MoveByAmount(vec3 moveAmount);

void bigc_camera_SetRotation(vec3 newRotation);

void bigc_camera_RotateByAmount(vec3 rotateAmount);

void bigc_camera_LoadViewProjectionMatrices(); //Load the view and projection matrices into the UBO, once per frame before drawing, so that they are shared among every shader that needs them

const bigc_Transform* bigc_camera_GetTransform();

#endif