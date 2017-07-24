#if (defined WIN32) && (defined USE_D3D)
#include "Platform/Win32D3D11.h"
#endif

#include "Platform/GLFWApp.h"

int main(int argc,char** argv)
{
	//can change app according command args 
	//if defined USE_D3D on windows
	//linux noly glfw.
#ifdef USE_D3D
	//if argv[1] -d3d11
	//Win32D3DApp app;
	//else
	//GLFWApp app;
#else
	GLFWApp app;
#endif
	app.init();
	app.run();
	return 0;

}