#pragma once
#include "VertexFormat.h"
#include "StageBase.h"
#include "BufferIndex.h"
#include "BufferConstant.h"
#include "BufferVertex.h"
#include "BufferUnorder.h"
#include "InnerData.h"
#include "../RBMath/Inc/AABB.h"

class SrStageIA : public SrStageBase
{
public:
	SrStageIA();
	~SrStageIA();
	void init();
	void input(const SrBufferVertex& vertex_buffer,const SrBufferIndex& index_buffer);
	void output(SrBufferVertex& vertex_buffer,SrBufferIndex& index_buffer);
	void proccess(const SrBufferVertex& vertex_buffer, const SrBufferIndex& index_buffer, std::vector<SrTriangle*>& triangles,int tri_n);
	void proccess_sort_everywhere(const SrBufferVertex& vertex_buffer, const SrBufferIndex& index_buffer, std::vector<SrTriangle*>& triangles, int tri_n, class SrSimGPU* gpu
	);
	void set_wireframe(bool w)
	{
		wire = w;
	}
	void clear();
	SrSimGPU* gpu;
private:
	bool wire = false;;
	static const uint tn = 8;
	static const uint tntn = tn*tn;
	std::vector<SrTriangle*> vs[tntn];
	RBAABB2D quads[tntn];
	
	
};

