#pragma once
#include "ShaderBase.h"
#include "InnerData.h"

class SrShaderPixel : public SrShaderBase
{
public:
	SrShaderPixel();
	~SrShaderPixel();

	void proccess(SrFragment& fragments)
	{
		for (auto& px : fragments.frag)
		{
			RBColorf c = shade(px);
			px.position.w = c.a;
			px.normal.x = c.r;
			px.normal.y = c.g;
			px.normal.z = c.b;
		}
	}

	virtual RBColorf shade(VertexP3N3T2& vert_lerp) = 0;
private:

};

