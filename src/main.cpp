#if (defined WIN32) && (defined USE_D3D)
#include "Platform/Win32D3D11.h"
#endif

#include "Platform/GLFWApp.h"
#include "Object.h"
#include "RBMath/Inc/RBMath.h"

class AT
{
public:
	AT(int k) :key(k){}
	int key;
};

int main(int argc,char** argv)
{  
#if 0
	HashLink<AT> hash_link[2];
	std::vector<AT*> objs;
	for (int i = 0; i < 10000;++i)
	{
		objs.push_back(new AT(i));
	}
	for (int t = 0; t < 20; t++)
	{
		hash_link[0].insert(objs[t]);
	}
	for (int t = 19; t >= 0; t--)
	{
		hash_link[0].remove(objs[t]);
	}
	while (true)
	{
		int id = RBMath::get_rand_i(9999);
		int k = RBMath::get_rand_i(800);
		if (k > 400)
		{
			hash_link[0].insert(objs[id]);
			hash_link[0].remove(objs[id]);
		}
		else
		{
			hash_link[1].insert(objs[id]);
		}
		hash_link[1].remove(objs[id]);

	}
#endif
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
	g_app = &app;
#endif
	app.init();
	app.run();
	return 0;

}