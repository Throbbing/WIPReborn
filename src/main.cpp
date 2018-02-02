#if (defined WIN32) && (defined USE_D3D)
#include "Platform/Win32D3D11.h"
#endif
//#include "RefCountPtr.h"
#include "Platform/GLFWApp.h"
#include "Object.h"
#include "RBMath/Inc/RBMath.h"

class AT
{
public:
	AT(int k) :key(k){}
	int key;
};


#ifdef TEST




class TObj:public FRefCountedObject
{
public:
	TObj():a(1){}
	int a;
};

class TSp;
class TComp :public TObj
{
public:
	TSp* host;
};

class C1 :public TComp
{
public:
	C1() :b(0){}
	int b;
};

class C2 :public TComp
{
public:
	C2() :b(0){}
	int b;
};

class TSp :public TObj
{
public:
	~TSp()
	{
		printf("dec");
	}
	void set_comp(TRefCountPtr<TComp> cc)
	{
		c1[i++] = cc;
		cc->host = this;
	}
	TRefCountPtr<TComp> c1[3];
	int i=0;
};

template <class T>
T* get()
{
	T* p = new T();
	std::cout << "alloc:" << p << std::endl;
	return p;
}

int main()
{
	TRefCountPtr<TSp> a = get<TSp>();
	a->set_comp(get<C1>());
	a->set_comp(get<C2>());
	a->set_comp(get<C1>());

	
}

#else
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
#endif