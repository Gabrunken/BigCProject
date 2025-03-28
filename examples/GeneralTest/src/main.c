#include <application.h>

int main()
{
	Initialize();

	Loop();

	//Automatically calls the function set by atexit(), which is the clean up function.
	return 0;
}