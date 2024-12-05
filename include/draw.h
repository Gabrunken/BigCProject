#ifndef draw_h_
#define draw_h_

#include <material.h>
#include <model.h>
#include <prop.h>

void bigc_draw_InitializeModule();

void bigc_draw_SetDrawCallback(void (*callback)(void));
void bigc_draw_SetClearFlags(int flags);

void bigc_draw_Model(const bigc_Model* model, const bigc_Material* materialReference);
void bigc_draw_Prop(const bigc_Prop* prop);

void bigc_draw_DrawCallback();

#endif