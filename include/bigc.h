#ifndef bigc_h_
#define bigc_h_

#define BIGC_FALSE 0
#define BIGC_TRUE (!BIGC_FALSE)

#define BIGC_INIT_FAILED -1
#define BIGC_INIT_SUCCESS (!BIGC_INIT_FAILED)

#ifdef DEBUG
#include <stdio.h>
#endif

//Has to be included before glfw3.h, or else it gives errors about stuff it doesn't know about. Don't ask why.
#include <gl3w.h>
#include <glfw3.h>
#include <draw.h>
#include <shaders.h>
#include <texture.h>
#include <material.h>
#include <model.h>
#include <prop.h>
#include <camera.h>
#include <transform.h>
#include <bitscalculations.h>
#include <ubo.h>

//If the program is in debug mode
#ifdef DEBUG

//use it to log fatal issues, an operation (maybe a function) failed its purpose or cannot go on
#define BIGC_LOG_ERROR(message)\
{\
	printf("BIGC ERROR: %s\n", message);\
}

//use it when something went wrong or might, altho what you were trying to do can still go on
#define BIGC_LOG_WARNING(message)\
{\
	printf("BIGC WARNING: %s\n", message);\
}

//simply print something to note, no issues have happened
#define BIGC_LOG_NOTE(message)\
{\
	printf("BIGC NOTE: %s\n", message);\
}

#endif

uint8_t bigc_Initialize(const char* windowTitle, unsigned short windowWidth, unsigned short windowHeight);
void bigc_Terminate();

#endif