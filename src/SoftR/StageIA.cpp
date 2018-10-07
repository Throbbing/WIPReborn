#include "StageIA.h"
#include "StageGS.h"
#include "StageVS.h"
#include "StageOM.h"
#include "Rasterizer.h"
#include "SimGPU.h"

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
			vertex_buffer[index_buffer[index]],
			vertex_buffer[index_buffer[index+1]],
			vertex_buffer[index_buffer[index+2]])
			);
			
		index += 3;
	}
}



void SrStageIA::proccess_sort_everywhere(const SrBufferVertex & vertex_buffer,
	const SrBufferIndex & index_buffer, std::vector<SrTriangle*>& triangles, int tri_n, SrSimGPU* gpu)
{
	//on some situation, load is unbalanced 
	//when some many triangles are out of camera view 
	//because they generate no fragments so thread run out quickly
	int index = 0;
	int max_single_size = 5000;
	uint cur_size = 0;
	uint s = 0, e = 0;
	if (max_single_size > tri_n)
	{
		gpu->add_task_sort_everywhere(vertex_buffer, index_buffer, 0, (tri_n*3-1), wire);
	}
	else
	for (int i = 0; i < tri_n; i++)
	{
		if (cur_size == 0)
			s = index;

		cur_size++;
		index += 3;

		if (cur_size == max_single_size || (tri_n - i - 1<max_single_size))
		{
			e = index;
			gpu->add_task_sort_everywhere(vertex_buffer, index_buffer, s, e - 1,wire);
			cur_size = 0;
		}


	}

	gpu->wait();
	
	float _viewport_w = gpu->get_width();
	float _viewport_h = gpu->get_height();
	//
	for (int i = 0; i < gpu->thread_res_n.size(); ++i)
	{
		for (int j = 0; j < gpu->thread_res_n[i]; ++j)
		{
			SrTriangle* tri =  gpu->thread_res[i][j];
			int x0 = RBMath::clamp((int)(tri->v[0].position.x + 0.5f*_viewport_w), 0, (int)_viewport_w - 1);
			int y0 = RBMath::clamp((int)(tri->v[0].position.y + 0.5f*_viewport_h), 0, (int)_viewport_h - 1);
			int x1 = RBMath::clamp((int)(tri->v[1].position.x + 0.5f*_viewport_w), 0, (int)_viewport_w - 1);
			int y1 = RBMath::clamp((int)(tri->v[1].position.y + 0.5f*_viewport_h), 0, (int)_viewport_h - 1);
			int x2 = RBMath::clamp((int)(tri->v[2].position.x + 0.5f*_viewport_w), 0, (int)_viewport_w - 1);
			int y2 = RBMath::clamp((int)(tri->v[2].position.y + 0.5f*_viewport_h), 0, (int)_viewport_h - 1);
			RBAABB2D ab;
			RBVector2 off(0.5f*_viewport_w, 0.5f*_viewport_h);
			ab.include(tri->v[0].position+off);
			ab.include(tri->v[1].position+off);
			ab.include(tri->v[2].position+off);
			for (int i = 0; i < tntn; ++i)
			{
				
				if (quads[i].intersection(ab))
				{
					vs[i].push_back(tri);
				}
			}
		}
	}
	
	for (int i = 0; i < tntn; ++i)
	{
		if (vs[i].size()>0)
			gpu->add_task(vs[i], quads[i], wire);
	}
	

}

SrStageIA::SrStageIA()
{
	
}

void SrStageIA::init()
{
	float _viewport_w = gpu->get_width();
	float _viewport_h = gpu->get_height();
	//³ýÇå
	float iw = _viewport_w / tn;
	float ih = _viewport_h / tn;
	uint c = 0;
	for (uint i = 0; i<tn; i++)
	{
		for (uint j = 0; j<tn; j++)
		{
			float rw(j*iw + iw), rh(i*ih + ih);
			if (i == tn) rw = _viewport_w;
			if (j == tn) rh = _viewport_h;
			quads[c++] = RBAABB2D(RBVector2(j*iw, i*ih), RBVector2(rw-1, rh-1));
		}
	}
}

void SrStageIA::clear()
{
	for(int i=0;i<tntn;++i)
	{
		vs[i].swap(std::vector<SrTriangle*>());
		vs[i].clear();
	}
}

SrStageIA::~SrStageIA()
{
}

