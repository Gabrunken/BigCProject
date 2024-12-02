#include <draw.h>
#include <bigc.h>
#include <cglm/call.h>

mat4 bigc_modelMatrix;
mat4 bigc_viewMatrix;
mat4 bigc_projectionMatrix;
mat4 bigc_MVPMatrix;

//Make them global so we always know whats the currently bound object, with that we don't have to rebind it again.
static GLuint currentlyBoundVertexArray;
extern unsigned int bigc_currentlyBoundShader; //Found in shaders.c

static void (*DrawCallback)(void);
static int clearFlags;

extern GLFWwindow* bigc_mainWindow;

void bigc_draw_InitializeModule()
{
	//Set clear color to black
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	//Enable depth test
	glEnable(GL_DEPTH_TEST);
	//Enable face culling (back face by default)
	glEnable(GL_CULL_FACE);

	glmc_mat4_identity(bigc_modelMatrix);
	glmc_mat4_identity(bigc_viewMatrix);
	glmc_mat4_identity(bigc_projectionMatrix);
	glmc_mat4_identity(bigc_MVPMatrix);
}

void bigc_draw_SetDrawCallback(void (*callback)(void))
{
	if(callback == NULL)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("cannot pass a NULL pointer as the draw callback function");
		#endif
		return;
	}

	DrawCallback = callback;
}

void bigc_draw_SetClearFlags(int flags)
{
	clearFlags = flags;
}

void bigc_draw_Model(bigc_Model model, const bigc_Material* materialReference)
{
	//Bind the vertex array if not already bound
	if(currentlyBoundVertexArray != model.vertexArrayHandle && model.vertexArrayHandle != 0)
	{
		glBindVertexArray(model.vertexArrayHandle);
		currentlyBoundVertexArray = model.vertexArrayHandle;
	}

	//Bind the shader if not already bound
	if(bigc_currentlyBoundShader != materialReference->shaderReference->handle)
	{
		glUseProgram(materialReference->shaderReference->handle);
		bigc_currentlyBoundShader = materialReference->shaderReference->handle;
	}

	bigc_material_UploadDataToShader(materialReference);
	glDrawElements(GL_TRIANGLES, model.indicesCount, GL_UNSIGNED_INT, NULL);
}

void bigc_draw_Prop(const bigc_Prop* prop)
{
	//Bind the vertex array if not already bound
	if(currentlyBoundVertexArray != prop->modelReference->vertexArrayHandle && prop->modelReference->vertexArrayHandle != 0)
	{
		glBindVertexArray(prop->modelReference->vertexArrayHandle);
		currentlyBoundVertexArray = prop->modelReference->vertexArrayHandle;
	}

	//Bind the shader if not already bound
	if(bigc_currentlyBoundShader != prop->materialReference->shaderReference->handle)
	{
		glUseProgram(prop->materialReference->shaderReference->handle);
		bigc_currentlyBoundShader = prop->materialReference->shaderReference->handle;
	}

	bigc_prop_LoadToMVP(prop);
	bigc_material_UploadDataToShader(prop->materialReference);
	glDrawElements(GL_TRIANGLES, prop->modelReference->indicesCount, GL_UNSIGNED_INT, NULL);
}

void bigc_draw_DrawCallback()
{
	bigc_camera_LoadToViewMatrix();

	glClear(clearFlags);
	DrawCallback();
	glfwSwapBuffers(bigc_mainWindow);
	glfwPollEvents();
}