#pragma once
#include "StageBase.h"
#include <vector>
#include "InnerData.h"

class SrStageOM : public SrStageBase
{
public:
	SrStageOM();
	~SrStageOM();

	void proccess(std::vector<SrFragment*>& _triangles_fragments, SrSSBuffer<RBColorf>& color, SrSSBuffer<float>& depth);
	void proccess(VertexP3N3T2& px, class SrTexture2D* color, SrSSBuffer<RBColor32>* bk, SrSSBuffer<float>& depth);
private:

};

