#pragma once

#include "StageIA.h"
#include "StageVS.h"
#include "StageGS.h"
#include "Rasterizer.h"
#include <vector>
#include "InnerData.h"
#include "../Profiler.h"
#include "SimGPU.h"

class SrPipeline
{
public:
	SrPipeline();
	~SrPipeline();

	void draw(const SrBufferVertex & vertex_buffer, const SrBufferIndex & index_buffer,int num_tri);

	void set_vs(SrShaderVertex* vs)
	{
		_stage_vs->set_vertex_shader(vs);
	}

	void set_ps(SrShaderPixel* ps)
	{
		_rasterizer->set_ps(ps);
	}

	inline void show_buffer_index(int index) { _bfidx = index; }

  /*
	//inner use
	void set_out_tex(RBD3D11Texture2D* out_tex)
	{
		_out_tex = out_tex;
	}
  */
	  
	void clear()
	{
		//批量修改内存，下列函数非常耗时
		//_color_buffer.set_vals(0.f);
		//_depth_buffer.set_vals(1.1f);
		_clear_SSBuffer();
	}

private:

	Profiler _profler;

	void _clear();
	void _clear_SSBuffer();

	//void _show_buffer(int index,RBD3D11Texture2D* out_tex);


	SrStageIA* _stage_ia;
	SrStageVS* _stage_vs;
	SrStageGS* _stage_gs;
	SrRasterizer* _rasterizer;

	std::vector<SrTriangle*> _triangles;
	std::vector<SrTriangle*> _triangles_near_far_cull;
	std::vector<SrTriangle*> _triangles_back_cull;
	std::vector<SrTriangle*> _triangles_clip;
	std::vector<SrFragment*> _triangles_fragments;
	const SrBufferVertex* _backup_vertex;
	const SrBufferIndex* _backup_index;
	SrSSBuffer<RBColor32> _color_buffer;
	SrSSBuffer<float> _depth_buffer;


	//thread
	std::vector<SrSSBuffer<RBColor32>* > _color_buffers;
	std::vector<SrSSBuffer<float>* > _depth_buffers;

	//useless
	SrSSBuffer<RBColor32> _co_color_buffer;
	SrSSBuffer<float> _co_depth_buffer;

	//origin
	SrSSBuffer<RBColor32> _o_color_buffer;
	SrSSBuffer<float> _o_depth_buffer;
	//RBD3D11Texture2D* _out_tex;

	//threads
	SrSimGPU s1;

	int _bfidx;

};
