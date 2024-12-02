#include <input.h>
#include <bigc.h>
#include <cglm/call.h>

extern mat4 bigc_viewMatrix;
extern float deltaTime;
extern float moveSpeed;
vec3 moveAmount;

void CursorPositionCallback(GLFWwindow* window, double windowX, double windowY)
{

}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	switch (button)
	{
		//Left click
		case GLFW_MOUSE_BUTTON_LEFT:
			break;

		//Right click
		case GLFW_MOUSE_BUTTON_RIGHT:
			break;

		default:
			break;
	}
}

void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	static vec3 moveInput;

	switch (key)
	{
	case GLFW_KEY_W:
		if (action == GLFW_PRESS)
		{
			moveInput[2] += 1.0f;
		}

		else if (action == GLFW_RELEASE)
		{
			moveInput[2] -= 1.0f;
		}

		break;

	case GLFW_KEY_S:
		if (action == GLFW_PRESS)
		{
			moveInput[2] += -1.0f;
		}

		else if (action == GLFW_RELEASE)
		{
			moveInput[2] -= -1.0f;
		}

		break;

	case GLFW_KEY_A:
		if (action == GLFW_PRESS)
		{
			moveInput[0] -= -1.0f;
		}

		else if (action == GLFW_RELEASE)
		{
			moveInput[0] += -1.0f;
		}

		break;

	case GLFW_KEY_D:
		if (action == GLFW_PRESS)
		{
			moveInput[0] -= 1.0f;
		}

		else if (action == GLFW_RELEASE)
		{
			moveInput[0] += 1.0f;
		}

		break;

	case GLFW_KEY_E:
		if (action == GLFW_PRESS)
		{
			moveInput[1] -= 1.0f;
		}

		else if (action == GLFW_RELEASE)
		{
			moveInput[1] += 1.0f;
		}

		break;

	case GLFW_KEY_Q:
		if (action == GLFW_PRESS)
		{
			moveInput[1] -= -1.0f;
		}

		else if (action == GLFW_RELEASE)
		{
			moveInput[1] += -1.0f;
		}

		break;

	case GLFW_KEY_F1:
		if (action == GLFW_PRESS)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;

	case GLFW_KEY_F2:
		if (action == GLFW_PRESS)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	}

	glmc_vec3_scale(moveInput, moveSpeed, moveAmount);
}