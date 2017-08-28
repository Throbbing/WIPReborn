#pragma once
#include "..\\RBMath\\Inc\\Vector2.h"
#include "..\\RBMath\\Inc\\Vector3.h"
#include "..\\RBMath\\Inc\\Colorf.h"


struct VertexP3N3C4
{
	RBVector4 position;
	RBVector3 normal;
	RBColorf color;
};

struct VertexP3N3T2
{
	VertexP3N3T2() : finish(false)
	{

	}
	RBVector4 position;
	RBVector3 normal;
	RBVector2 text_coord;
	//用于多线程终结信号	
	bool finish;
};