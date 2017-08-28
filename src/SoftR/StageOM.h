#pragma once
#include "StageBase.h"
#include <vector>
#include "InnerData.h"

class SrStageOM : public SrStageBase
{
public:
	SrStageOM();
	~SrStageOM();

	void proccess(std::vector<SrFragment*>& _triangles_fragments, SrSSBuffer<RBColor32>& color, SrSSBuffer<float>& depth);
	void proccess(VertexP3N3T2& px, SrSSBuffer<RBColor32>& color, SrSSBuffer<float>& depth);
private:

};

