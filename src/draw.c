#include <draw.h>
#include <bigc.h>
#include <cglm/call.h>

mat4 bigc_modelMatrix;
mat4 bigc_viewMatrix;
mat4 bigc_projectionMatrix;

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

void bigc_draw_Model(const bigc_Model* model, const bigc_Material* materialReference)
{
	//Check first of all if the passed model is valid or not
	if(model->vertexArrayHandle == 0)
	{
		#ifdef DEBUG
		BIGC_LOG_WARNING("cannot draw an invalid model");
		#endif
		return;
	}

	//Bind the vertex array if not already bound
	if(currentlyBoundVertexArray != model->vertexArrayHandle)
	{
		glBindVertexArray(model->vertexArrayHandle);
		currentlyBoundVertexArray = model->vertexArrayHandle;
	}

	//Check if the passed shader is valid or not
	if(materialReference->shaderReference->handle == 0)
	{
		#ifdef DEBUG
		BIGC_LOG_WARNING("cannot draw with an invalid shader");
		#endif

		//TODO: prolly just bind a standard shader
		return;
	}

	//Bind the shader if not already bound
	if(bigc_currentlyBoundShader != materialReference->shaderReference->handle)
	{
		glUseProgram(materialReference->shaderReference->handle);
		bigc_currentlyBoundShader = materialReference->shaderReference->handle;
	}

	bigc_material_UploadDataToShader(materialReference);
	glDrawElements(GL_TRIANGLES, model->indicesCount, GL_UNSIGNED_INT, NULL);
}

void bigc_draw_Prop(const bigc_Prop* prop)
{
	//Check first of all if the passed prop is valid or not
	if(prop->modelReference->vertexArrayHandle == 0)
	{
		#ifdef DEBUG
		BIGC_LOG_WARNING("cannot draw a prop with an invalid model reference");
		#endif
		return;
	}

	//Bind the vertex array if not already bound
	if(currentlyBoundVertexArray != prop->modelReference->vertexArrayHandle)
	{
		glBindVertexArray(prop->modelReference->vertexArrayHandle);
		currentlyBoundVertexArray = prop->modelReference->vertexArrayHandle;
	}

	//Check if the passed shader is valid or not
	if(prop->materialReference->shaderReference->handle == 0)
	{
		#ifdef DEBUG
		BIGC_LOG_WARNING("cannot draw with an invalid shader");
		#endif

		//TODO: prolly just bind a standard shader
		return;
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
	if(DrawCallback == NULL)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("before calling the draw function you must first set a draw callback");
		#endif
		return;
	}

	bigc_camera_LoadToViewMatrix();

	glClear(clearFlags);
	DrawCallback();
	glfwSwapBuffers(bigc_mainWindow);
	glfwPollEvents();
}