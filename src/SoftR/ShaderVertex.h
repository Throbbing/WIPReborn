#pragma once
#include "ShaderBase.h"
#include "InnerData.h"

class SrShaderVertex : public SrShaderBase
{
public:
	SrShaderVertex();
	~SrShaderVertex();


	void proccess(SrTriangle& triangle)
	{
		shade(triangle.v[0]);
		shade(triangle.v[1]);
		shade(triangle.v[2]);
	}

	virtual void shade(VertexP3N3T2& v) = 0;
private:

};

