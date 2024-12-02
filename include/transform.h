#ifndef transform_h
#define transform_h

#include <cglm/call.h>

typedef struct bigc_Transform
{
	vec3 position;
	vec3 rotation;
	vec3 scale;
} bigc_Transform;

#endif