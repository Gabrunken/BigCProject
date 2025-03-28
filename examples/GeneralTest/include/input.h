#ifndef input_h_
#define input_h_

//Has to be included before glfw3.h, or else it gives errors about stuff it doesn't know about. Don't ask why.
#include <gl3w.h>
#include <glfw3.h>

//Callbacks from GLFW
//Cursor position on window when cursor moved.
void CursorPositionCallback(GLFWwindow* window, double windowX, double windowY);
//Button is like left or right mouse button, action is mouse up or down, mods is... idk.
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
//key corresponds to GLFW key enums, like GLFW_KEY_F. action is like GLFW_PRESS, GLFW_RELEASE, scancode and mods... idk.
void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
#endif