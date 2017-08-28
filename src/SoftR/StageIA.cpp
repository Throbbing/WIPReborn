#include "StageIA.h"


inline void SrStageIA::input(const SrBufferVertex & vertex_buffer, const SrBufferIndex & index_buffer)
{

}

void SrStageIA::output(SrBufferVertex & vertex_buffer, SrBufferIndex & index_buffer)
{

}


void SrStageIA::proccess(const SrBufferVertex& vertex_buffer, const SrBufferIndex& index_buffer, std::vector<SrTriangle*>& triangles,int tri_n)
{

	int index = 0;
	for (int i = 0; i < tri_n;i++)
	{
		/*
		triangles[i]->v[0] = *vertex_buffer[index_buffer[index]];
		triangles[i]->v[1] = *vertex_buffer[index_buffer[index+1]];
		triangles[i]->v[2] = *vertex_buffer[index_buffer[index+2]];
		*/
		
		triangles.push_back(new SrTriangle
			(
			*vertex_buffer[index_buffer[index]],
			*vertex_buffer[index_buffer[index+1]],
			*vertex_buffer[index_buffer[index+2]])
			);
			
		index += 3;
	}
}

SrStageIA::SrStageIA()
{
}

SrStageIA::~SrStageIA()
{
}

