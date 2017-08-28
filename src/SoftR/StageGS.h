#pragma once
#include "VertexFormat.h"
#include "StageBase.h"
#include "BufferIndex.h"
#include "BufferConstant.h"
#include "BufferVertex.h"
#include "BufferUnorder.h"

class SrStageGS : public SrStageBase
{
public:
	SrStageGS();
	~SrStageGS();
	void input(SrBufferVertex& vertex_buffer, SrBufferIndex& index_buffer);
	void output(SrBufferVertex& vertex_buffer, SrBufferIndex& index_buffer);
	void proccess()
	{

	}
private:

};

