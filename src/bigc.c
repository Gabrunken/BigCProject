#include <bigc.h>

GLFWwindow* bigc_mainWindow;

#ifdef DEBUG
static void GLAPIENTRY HandleOpenGLErrors(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam
)
{
	//Print error the message
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		   (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		    type, severity, message);
}
#endif

uint8_t bigc_Initialize(const char* windowTitle, unsigned short windowWidth, unsigned short windowHeight)
{
	if(!glfwInit())
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("failed to initialize glfw");
		#endif
		return BIGC_INIT_FAILED;
	}

	//Tell the opengl version to use, if it is not available, i believe it uses the most recent available version.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	//To use a profile that is NOT GLFW_OPENGL_ANY_PROFILE, we need an OpenGL version not below 3.2
	//So GLFW_CONTEXT_VERSION_* is needed.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	bigc_mainWindow = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);
	if(bigc_mainWindow == NULL)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("failed to create window");
		#endif
		return BIGC_INIT_FAILED;
	}

	glfwMakeContextCurrent(bigc_mainWindow);

	//GL3W has to be initialized before calling any OpenGL functions, even legacy.
	if(gl3wInit())
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("failed to initialize gl3w");
		#endif
		return BIGC_INIT_FAILED;
	}

	#ifdef DEBUG
	//We need an OpenGL context and GL3W initialized to call glGetString()
	printf("OpenGL supported version: %s\n", glGetString(GL_VERSION));

	//Enable opengl debug messages to know what's wrong in case something went bad.
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(HandleOpenGLErrors, NULL);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
	#endif

	bigc_ubo_InitializeModule();//This must be initialized before the "draw" module
	bigc_draw_InitializeModule();
	bigc_texture_InitializeModule();

	if(bigc_network_InitializeModule() == BIGC_FALSE)
	{
		#ifdef DEBUG
		BIGC_LOG_ERROR("failed to initialize network module");
		#endif
		return BIGC_INIT_FAILED;
	}

	return BIGC_INIT_SUCCESS;
}

void bigc_Terminate()
{
	glfwTerminate();

	extern bigc_UBO bigc_viewProjectionUBO;
	bigc_ubo_FreeFromGPU(&bigc_viewProjectionUBO);
}