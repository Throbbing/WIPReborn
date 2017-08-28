#include "StagePS.h"
//SrShaderPixel*SrStagePS::_ps = nullptr;
/*
void SrStagePS::proccess(std::vector<SrFragment*>& _triangles_fragments)
{
	if (!_ps) return;
	for (auto frg : _triangles_fragments)
	{
		_ps->proccess(*frg);
	}	
}
*/
void SrStagePS::proccess(VertexP3N3T2& px)
{
	if (!_ps) return;
	//RBColorf  c = RBColorf::cyan;
	RBColorf c = _ps->shade(px);
	px.position.w = c.a;
	px.normal.x = c.r;
	px.normal.y = c.g;
	px.normal.z = c.b;
}

SrStagePS::SrStagePS()
{
	_ps = nullptr;
}

SrStagePS::~SrStagePS()
{
}