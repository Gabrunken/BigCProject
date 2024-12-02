#ifndef prop_h
#define prop_h

#include <cglm/call.h>
#include <material.h>
#include <model.h>
#include <transform.h>

typedef struct bigc_Prop
{
	bigc_Transform transform;

	const bigc_Model* modelReference;
	bigc_Material* materialReference;
} bigc_Prop;

void bigc_prop_Initialize(bigc_Prop* prop, const bigc_Model* model, const bigc_Material* material);

void bigc_prop_SetPosition(bigc_Prop* prop, vec3 newPosition);
void bigc_prop_SetRotation(bigc_Prop* prop, vec3 newRotation);
void bigc_prop_SetScale(bigc_Prop* prop, vec3 newScale);

void bigc_prop_Move(bigc_Prop* prop, vec3 movement);
void bigc_prop_Rotate(bigc_Prop* prop, vec3 rotation);
void bigc_prop_Scale(bigc_Prop* prop, vec3 scale);

//updating the only existing MVP with an instance of a prop
void bigc_prop_LoadToMVP(bigc_Prop* prop);

#endif