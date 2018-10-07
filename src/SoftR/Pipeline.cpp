#include "Pipeline.h"
#include "SimGPU.h"
#include "../Input.h"

#include "../Debug.h"


void SrPipeline::draw(const SrBufferVertex & vertex_buffer, SrBufferIndex & index_buffer, int num_tri)
{
	if (Input::get_key_up(WIP_O))
	{
		_stage_ia->set_wireframe(false);
	}
	if (Input::get_key_up(WIP_P))
	{
		_stage_ia->set_wireframe(true);
	}
#define SORT_FIRST
#ifdef SORT_FIRST
	_profler.set_begin();
	//must change buffer write position
	//index_buffer.shuffle();
	//_stage_ia->proccess_sort_first(vertex_buffer, index_buffer, _triangles, num_tri, &s1);
	_stage_ia->proccess_sort_everywhere(vertex_buffer, index_buffer, _triangles, num_tri, &s1);
	_profler.set_end("WAIT1");
#else
	rmt_BeginCPUSample(PipLine);
	//_backup_index = &index_buffer;
	//_backup_vertex = &vertex_buffer;
	

	_profler.set_begin();
	rmt_BeginCPUSample(IA);
	_stage_ia->proccess(vertex_buffer, index_buffer, _triangles, num_tri);
	rmt_EndCPUSample();//IA

	_profler.set_end("IA"); _profler.set_begin();
	rmt_BeginCPUSample(VS);
	_stage_vs->proccess(_triangles,num_tri);
	rmt_EndCPUSample();//VS
	
	_profler.set_end("VS"); _profler.set_begin();
	rmt_BeginCPUSample(GS);
	_stage_gs->proccess();
	rmt_EndCPUSample();//GS

	_profler.set_end("GS"); _profler.set_begin();
	rmt_BeginCPUSample(NEAR_FAR_CULL);
	_rasterizer->near_far_cull(_triangles, _triangles_near_far_cull,num_tri);
	rmt_EndCPUSample();//NEAR_FAR_CULL
	_profler.set_end("NFC"); _profler.set_begin();
	rmt_BeginCPUSample(BACK_CULL);
	_rasterizer->back_cull(_triangles_near_far_cull, _triangles_back_cull);
	rmt_EndCPUSample();//BACK_CULL
	_profler.set_end("BC"); _profler.set_begin();
	rmt_BeginCPUSample(CLIP);
	_rasterizer->clip(_triangles_back_cull, _triangles_clip);
	rmt_EndCPUSample();//CLIP

	_profler.set_end("CLIP"); _profler.set_begin();
	rmt_BeginCPUSample(TS);
	_rasterizer->triangle_setup(_triangles_clip);
	rmt_EndCPUSample();//TS
	_profler.set_end("TS"); 
#endif

	_profler.set_begin();
	rmt_BeginCPUSample(thread_wait);
	s1.wait();
	rmt_EndCPUSample();//thread wait
	_profler.set_end("WAIT");

	rmt_EndCPUSample();//Pipline

	_profler.set_begin();
	_profler.out_put_after_time(20);
	_rasterizer->set_last_ts_time(_profler.get_time());
	s1.release();
	_clear();
	printf("total frag shaded:%f\n",(float)_rasterizer->get_total_shade_frag()/(_rasterizer->get_width()*_rasterizer->get_height()));
	_rasterizer->clear_total(); _profler.set_end("END");
}
float lum(const RBColorf& c)
{
	return c.r*0.299 + 0.587*c.g + c.b*0.114;
}
void SrPipeline::_show_buffer(int index,RBD3D11Texture2D* out_tex)
{
	out_tex->write_data(reinterpret_cast<RBColor32*>(&_main_buffer.get_buffer()[0]), _rasterizer->get_width(), _rasterizer->get_height());
	return;
	/*
	if (index == 0)
	{
		for (int i = 0; i < _color_buffer.h; ++i)
		{
			for (int j = 0; j < _color_buffer.w; ++j)
			{
				RBColorf bc =  _color_buffer.get_data(j, i);
				RBColorf color = bc;
				float sample_n = 100;
				float f = float(1) / float(sample_n);
				RBVector2 d = (RBVector2(0.5, 0.5) - RBVector2(j,i))*f;
				RBVector2 c = RBVector2(j, i);
				for (int i = 0; i<sample_n; ++i)
				{
					RBColorf sc = _color_buffer.get_data(c.x, c.y);
					if (lum(sc)>0.8)
						color = color + sc;

					c += d;
				}
				color *= f;

				color = color*0.7f + bc;
				RBColor32 rc;
				color.clamp_to_0_1();
				rc.r = color.r * 255;
				rc.g = color.g * 255;
				rc.b = color.b * 255;
				//rc.a = color.a * 255;

				_color_buffers[0]->set_data(j, i, rc);
			}
		}
		out_tex->write_data(reinterpret_cast<RBColor32*>(&_color_buffers[0]->get_buffer()[0]), _rasterizer->get_width(), _rasterizer->get_height());
	}
	*/
}


void SrPipeline::_clear_SSBuffer()
{
	//if(_color_buffer)
	//memcpy(&_color_buffer->get_buffer()[0], &_o_color_buffer.get_buffer()[0], _color_buffer->get_size());
	//memcpy(&_depth_buffer.get_buffer()[0], &_o_depth_buffer.get_buffer()[0], _depth_buffer.size);
	//memcpy(&_main_buffer.get_buffer()[0], &_o_main_buffer.get_buffer()[0], _main_buffer.size);
}

static const int thread_n = std::thread::hardware_concurrency();
SrPipeline::SrPipeline():s1(thread_n)
{
	_bfidx = 0;

	_profler.init();
	_profler.set_output_inter(10);

	SrTriangle::Init();

	_backup_index = 0;
	_backup_vertex = 0;
	_stage_ia = new SrStageIA();
	
	_stage_vs = new SrStageVS();
	_stage_gs = new SrStageGS();
	SrSimGPU::mem_list_init();
	_rasterizer = new SrRasterizer(&s1);
	int w = RW;
	int h = RH;
	_rasterizer->set_viewport_shape(w, h);
	_stage_ia->gpu = &s1;
	_stage_ia->init();
	_depth_buffer.init(w,h);
	_color_buffer = nullptr;
	_main_buffer.init(w,h);
	s1.set_stage_vs(_stage_vs);

	_o_depth_buffer.init(w, h);
	_o_color_buffer.init(w, h);
	_o_main_buffer.init(w, h);

	_o_main_buffer.set_vals(RBColor32(0,0,0,255));
	_o_depth_buffer.set_vals(1.1f);
	_o_color_buffer.set_vals(RBColorf::black);

	_clear_SSBuffer();

	_rasterizer->set_depth_buffer(&_depth_buffer);
	_rasterizer->set_color_buffer(_color_buffer);
	_rasterizer->set_main_buffer(&_main_buffer);


}

SrPipeline::~SrPipeline()
{
	delete _rasterizer;
	delete _stage_gs;
	delete _stage_vs;
	delete _stage_ia;

#ifndef POOL
	SrTriangle::Deinit();
#endif
	SrSimGPU::mem_list_deinit();

}

void SrPipeline::swap(f32 frame)
{
	_show_buffer(_bfidx, _out_tex);
}

void SrPipeline::clear_depth()
{
	//memcpy(&_color_buffer->get_buffer()[0], &_o_color_buffer.get_buffer()[0], _color_buffer->get_size());
	memcpy(&_depth_buffer.get_buffer()[0], &_o_depth_buffer.get_buffer()[0], _depth_buffer.size);
	//memcpy(&_main_buffer.get_buffer()[0], &_o_main_buffer.get_buffer()[0], _main_buffer.size);
}

void SrPipeline::clear_render_target()
{
	if(_color_buffer)
		memcpy(&_color_buffer->get_buffer()[0], &_o_color_buffer.get_buffer()[0], _o_color_buffer.size);
	//else
	
	memcpy(&_main_buffer.get_buffer()[0], &_o_main_buffer.get_buffer()[0], _main_buffer.size);
}

void SrPipeline::_clear()
{	
	//SrTriangle::print_mem();
#ifdef POOL
	for (auto& tri : _triangles)
	{
		delete tri;
		tri  = nullptr;
	}
#else
	SrTriangle::ReleaseAll();
#endif


	_triangles.clear();
	_triangles_near_far_cull.clear();
	_triangles_back_cull.clear();
	_triangles_clip.clear();
	_triangles_fragments.clear();
	
	_stage_ia->clear();

}