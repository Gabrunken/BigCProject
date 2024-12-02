#include <application.h>
#include <bigc.h>

#include <input.h>
#include <math.h>

static bigc_ShaderProgram unlitTexturedShaderProgram, unlitSolidColorShaderProgram;
static bigc_Texture testTexture;
static bigc_Model sphereModel;
static bigc_Prop sphereProp;

float deltaTime;
float moveSpeed = 2.5f;

extern vec3 moveAmount;

extern mat4 bigc_modelMatrix;
extern mat4 bigc_projectionMatrix;
extern mat4 bigc_viewMatrix;

static bigc_Material sphereMaterial; 

extern GLFWwindow* bigc_mainWindow;

void DrawScene()
{
	bigc_draw_Prop(&sphereProp);
}

void Initialize()
{
	//Basically when the program ends (at the final "return" statement), calls the CleanUp function, which just cleans up everything and frees memory.
	atexit(CleanUp);

	if(bigc_Initialize("Application", WINDOW_WIDTH, WINDOW_HEIGHT) == BIGC_INIT_FAILED)
	{
		exit(BIGC_INIT_FAILED);
	}

	//Set GLFW callbacks
	glfwSetCursorPosCallback(bigc_mainWindow, CursorPositionCallback);
	glfwSetMouseButtonCallback(bigc_mainWindow, MouseButtonCallback);
	glfwSetKeyCallback(bigc_mainWindow, KeyboardCallback);

	sphereModel = bigc_model_LoadFromDiskAsSingleModel("./Resources/Models/sphere.gltf");

	unlitTexturedShaderProgram = bigc_shaders_LoadFromDisk("./Resources/Shaders/Vertex/unlit_textured.vertexshader",
												   "./Resources/Shaders/Fragment/unlit_textured.fragmentshader");

	unlitSolidColorShaderProgram = bigc_shaders_LoadFromDisk("./Resources/Shaders/Vertex/unlit_solidcolor.vertexshader",
												   "./Resources/Shaders/Fragment/unlit_solidcolor.fragmentshader");

	//bigc_Texture loading is to be fixed
	//testTexture = bigc_texture_LoadFromDisk("./Resources/Images/test.jpg");

	bigc_camera_MoveByAmount((vec3){0.0f, 0.0f, -4.0f});
	bigc_camera_SetPerspectiveProjection(60, WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
	
	sphereMaterial.shaderReference = &unlitSolidColorShaderProgram;

	bigc_material_UpdateVec4(&sphereMaterial, "color", (vec4){0.0f, 1.0f, 0.5f, 1.0f});

	bigc_prop_Initialize(&sphereProp, &sphereModel, &sphereMaterial);

	bigc_draw_SetDrawCallback(DrawScene);
	bigc_draw_SetClearFlags(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Loop()
{
	float lastTime = glfwGetTime();
	
	while(!glfwWindowShouldClose(bigc_mainWindow))
	{
		bigc_prop_SetPosition(&sphereProp, (vec3){0.0f, sin(glfwGetTime()), 0.0f});

		//Multiply move amount with deltaTime
		vec3 fixedMoveAmount;
		glmc_vec3_scale(moveAmount, deltaTime, fixedMoveAmount);

		//Translate camera
		bigc_camera_MoveByAmount(fixedMoveAmount);

		bigc_draw_DrawCallback();

		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;
	}
}

void CleanUp()
{
	glfwTerminate();

	//Free everything from gpu
	bigc_shaders_FreeFromGPU(&unlitTexturedShaderProgram);
	bigc_shaders_FreeFromGPU(&unlitSolidColorShaderProgram);
	bigc_model_FreeFromGPU(&sphereModel);
	//bigc_texture_FreeFromGPU(&testTexture);

	#ifdef DEBUG
	printf("\nClean up done.\nPress ENTER to terminate the process.");
	getchar();
	#endif
}