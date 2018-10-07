#pragma once

#include "StageIA.h"
#include "StageVS.h"
#include "StageGS.h"
#include "Rasterizer.h"
#include <vector>
#include "InnerData.h"
#include "../D3D11Texture2D.h"
#include "../Profiler.h"
#include "SimGPU.h"

class SrPipeline
{
public:
	SrPipeline();
	~SrPipeline();

	void draw(const SrBufferVertex & vertex_buffer, SrBufferIndex & index_buffer,int num_tri);

	void set_vs(SrShaderVertex* vs)
	{
		_stage_vs->set_vertex_shader(vs);
	}

	void set_ps(SrShaderPixel* ps)
	{
		_rasterizer->set_ps(ps);
	}

	void set_render_target(SrTexture2D& rt)
	{
		_rasterizer->set_color_buffer(&rt);
	}

	void set_render_target(SrSSBuffer<RBColor32>& rt)
	{
		_rasterizer->set_main_buffer(&rt);
	}


	inline void show_buffer_index(int index) { _bfidx = index; }

	//inner use
	void set_out_tex(RBD3D11Texture2D* out_tex)
	{
		_out_tex = out_tex;
	}
	void swap(f32 frame = 60);
	void clear()
	{
		//批量修改内存，下列函数非常耗时
		_clear_SSBuffer();
	}
	SrTexture2D* get_back_color_buffer()
	{
		return _color_buffer;
	}
	SrSSBuffer<RBColor32>* get_main_buffer()
	{
		return &_main_buffer;
	}
	void clear_depth();
	void clear_render_target();
private:

	Profiler _profler;

	void _clear();
	void _clear_SSBuffer();

	void _show_buffer(int index,RBD3D11Texture2D* out_tex);

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
	SrTexture2D* _color_buffer;
	SrSSBuffer<RBColor32> _main_buffer;

	SrSSBuffer<float> _depth_buffer;

	//origin
	SrSSBuffer<RBColorf> _o_color_buffer;
	SrSSBuffer<RBColor32> _o_main_buffer;

	SrSSBuffer<float> _o_depth_buffer;

	RBD3D11Texture2D* _out_tex;

	//threads
	SrSimGPU s1;

	int _bfidx;

};
