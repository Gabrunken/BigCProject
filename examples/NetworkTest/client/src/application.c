#include <application.h>
#include <bigc.h>
#include <pthread.h>
#include <input.h>
#include <math.h>

static bigc_ShaderProgram litSolidColorShaderProgram;
static bigc_Model clientModel;
static bigc_Prop clientProp;

float deltaTime;
float moveSpeed = 2.5f;

extern vec3 moveAmount;

extern mat4 bigc_modelMatrix;
extern mat4 bigc_projectionMatrix;
extern mat4 bigc_viewMatrix;

bigc_Material clientMaterial; 

extern GLFWwindow* bigc_mainWindow;

static bigc_UBO lightsUBO;

struct DirectionalLight
{
	_Alignas(16) vec3 direction;
	_Alignas(16) vec3 color;
	_Alignas(4) float ambientStrength;
	_Alignas(4) float specularStrength;
};

void DrawScene()
{
	bigc_draw_Prop(&clientProp);
}

uint8_t disconnect = 0;
pthread_t clientCallbackThread;

void* ClientCallback(void* args)
{
	clientCallbackThread = pthread_self();
	SOCKET serverSocket = *(SOCKET*)args;

	while(!disconnect)
	{
		char serverData[1000] = {};

		if(bigc_network_CheckIncomingData(serverSocket))
		{
			int recvResult = recv(serverSocket, serverData, sizeof(serverData), 0);
			if(recvResult == 0)
			{
				//The server disconnected
				#ifdef DEBUG
				BIGC_LOG_NOTE("The server disconnected.");
				#endif
				disconnect = BIGC_TRUE;
			}

			else if(recvResult == SOCKET_ERROR)
			{
				#ifdef DEBUG
				BIGC_LOG_ERROR("Failed to receive data from the server or it closed without telling.");
				#endif
				disconnect = BIGC_TRUE;
			}

			else
			{
				//Here the server sent data
			}
		}
	}

	shutdown(serverSocket, SD_BOTH); //Telling the server we are going to close the connection with him
	#ifdef DEBUG
	BIGC_LOG_NOTE("Closing the server's socket.");
	#endif
	closesocket(serverSocket);
	free(args);
	return NULL;
}

void Initialize()
{
	//Basically when the program ends (at the final "return" statement), calls the CleanUp function, which just cleans up everything and frees memory.
	atexit(CleanUp);

	if(bigc_Initialize("Application", WINDOW_WIDTH, WINDOW_HEIGHT) == BIGC_INIT_FAILED)
	{
		exit(BIGC_INIT_FAILED);
	}

	glfwSwapInterval(0); //Disable VSYNC

	//Set GLFW callbacks
	glfwSetCursorPosCallback(bigc_mainWindow, CursorPositionCallback);
	glfwSetMouseButtonCallback(bigc_mainWindow, MouseButtonCallback);
	glfwSetKeyCallback(bigc_mainWindow, KeyboardCallback);

	clientModel = bigc_model_LoadOBJFromDisk("./Resources/Models/client.obj");

	litSolidColorShaderProgram = bigc_shaders_LoadFromDisk("./Resources/Shaders/Vertex/lit_solidcolor.vertexshader",
												   "./Resources/Shaders/Fragment/lit_solidcolor.fragmentshader");

	bigc_camera_MoveByAmount((vec3){0.0f, 0.0f, -4.0f});
	bigc_camera_SetPerspectiveProjection(60, WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);
	
	clientMaterial.shaderReference = &litSolidColorShaderProgram;

	bigc_material_UpdateIntOrBool(&clientMaterial, "material.shininess", 32);
	bigc_material_UpdateVec4(&clientMaterial, "material.color", (vec4){0.9f, 0.9f, 0.9f, 1.0f});

	struct DirectionalLight sun =
	{
		{-1.0f, -1.0f, -1.0f},
		{1.0f, 0.95f, 0.85f},
		0.35f,
		1.0f
	};

	int directionalLightsCount = 1;
	vec3 skyColor = {0.15f, 0.575f, 0.925f};

	lightsUBO = bigc_ubo_Create(1, sizeof(struct DirectionalLight) * 16 + 4 + 16, TRUE); //4 is the directionalLightCount, 16 is the skyColor
	bigc_ubo_LoadData(lightsUBO, skyColor, sizeof(vec3));
	bigc_ubo_LoadSubData(lightsUBO, &sun, sizeof(vec4), sizeof(struct DirectionalLight));
	bigc_ubo_LoadSubData(lightsUBO, &directionalLightsCount, sizeof(struct DirectionalLight) * 16 + sizeof(vec4), 4);

	bigc_prop_Initialize(&clientProp, &clientModel, &clientMaterial);

	bigc_draw_SetDrawCallback(DrawScene);
	bigc_draw_SetClearFlags(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(skyColor[0], skyColor[1], skyColor[2], 1.0f);

	bigc_network_TryConnectToServer(ClientCallback, "192.168.1.10", 42000, BIGC_TCP);
}


void Loop()
{
	float lastTime = glfwGetTime();
	
	while(!glfwWindowShouldClose(bigc_mainWindow))
	{
		//Multiply move amount with deltaTime
		vec3 fixedMoveAmount;
		glmc_vec3_scale(moveAmount, deltaTime, fixedMoveAmount);

		//Translate camera
		bigc_camera_MoveByAmount(fixedMoveAmount);
		bigc_material_UpdateVec3(&clientMaterial, "cameraPos", bigc_camera_GetTransform()->position);

		bigc_draw_DrawCallback();

		float currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		char fpsString[10];
		memset(fpsString, 0, 10);
		sprintf(fpsString, "%f", 1/deltaTime);
		glfwSetWindowTitle(bigc_mainWindow, fpsString);
	}

	disconnect = BIGC_TRUE;
	pthread_join(clientCallbackThread, NULL);
}

void CleanUp()
{
	bigc_Terminate(); //Mandatory

	//Free everything from gpu
	bigc_shaders_FreeFromGPU(&litSolidColorShaderProgram);
	bigc_model_FreeFromGPU(&clientModel);
	bigc_ubo_FreeFromGPU(&lightsUBO);

	#ifdef DEBUG
	printf("\nClean up done.\nPress ENTER to terminate the process.");
	getchar();
	#endif
}