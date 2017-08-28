#pragma once
#include "VertexFormat.h"
#include "StageBase.h"
#include "BufferIndex.h"
#include "BufferConstant.h"
#include "BufferVertex.h"
#include "BufferUnorder.h"
#include "InnerData.h"
#include "ShaderPixel.h"

class SrStagePS : public SrStageBase
{
public:
	SrStagePS();
	~SrStagePS();
	void input(SrBufferVertex& vertex_buffer,SrBufferIndex& index_buffer);
	void output();
	//void proccess(std::vector<SrFragment*>& _triangles_fragments);
	void proccess(VertexP3N3T2& px);
	void set_pixel_shader(SrShaderPixel* ps)
	{
		_ps = ps;
	}

private:
	SrShaderPixel* _ps;
};

