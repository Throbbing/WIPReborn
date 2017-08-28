#pragma once
#include "VertexFormat.h"
#include "StageBase.h"
#include "BufferIndex.h"
#include "BufferConstant.h"
#include "BufferVertex.h"
#include "BufferUnorder.h"
#include "ShaderVertex.h"

class SrStageVS : public SrStageBase
{
public:
	SrStageVS();
	~SrStageVS();
	void input(SrBufferVertex& vertex_buffer, SrBufferIndex& index_buffer);
	void output(SrBufferVertex& vertex_buffer);
	void proccess(std::vector<SrTriangle*>& triangles,int n)
	{
		if (!_vs) return;
		for (auto i : triangles)
			_vs->proccess(*i);
	}
	void set_vertex_shader(SrShaderVertex* vs)
	{
		_vs = vs;
	}
private:
	SrShaderVertex* _vs;
};

