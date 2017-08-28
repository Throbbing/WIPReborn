#pragma once
#include "VertexFormat.h"
#include "StageBase.h"
#include "BufferIndex.h"
#include "BufferConstant.h"
#include "BufferVertex.h"
#include "BufferUnorder.h"
#include "InnerData.h"

class SrStageIA : public SrStageBase
{
public:
	SrStageIA();
	~SrStageIA();
	void input(const SrBufferVertex& vertex_buffer,const SrBufferIndex& index_buffer);
	void output(SrBufferVertex& vertex_buffer,SrBufferIndex& index_buffer);
	void proccess(const SrBufferVertex& vertex_buffer, const SrBufferIndex& index_buffer, std::vector<SrTriangle*>& triangles,int tri_n);
private:

};

