#ifndef application_h_
#define application_h_

#include <stdlib.h>
#include <stdio.h>

#define WINDOW_WIDTH 1280.0f //just basic 16:9 HD
#define WINDOW_HEIGHT 720.0f

void DrawScene();

//Initialize program
void Initialize();

//Program's main loop
void Loop();

//Clean up program
//Should not be called manually, since it is called automatically when the process terminates.
void CleanUp();
#endif