#pragma once
#include "StagePS.h"
#include "StageOM.h"
#include "InnerData.h"
#include <vector>
#include "..\\RBMath\\Inc\\Color32.h"
#include "../Profiler.h"

class SrSimGPU;
class SrRasterizer
{
public:
	SrRasterizer();
	SrRasterizer(SrSimGPU* gpu);
	~SrRasterizer();

	void set_viewport_shape(float w,float h);

	void near_far_cull(std::vector<SrTriangle*> _triangles, std::vector<SrTriangle*>& _triangles_near_far_cull,int n);
	//顺时针
	void back_cull(std::vector<SrTriangle*> _triangles, std::vector<SrTriangle*>& _triangles_back_cull);
	void clip(std::vector<SrTriangle*> _triangles, std::vector<SrTriangle*>& _triangles_clip);
	//为三角形生成片元，插值
	void triangle_setup(std::vector<SrTriangle*> _triangles, std::vector<SrFragment*>& _triangles_fragments);
	//ps
	void shade(std::vector<SrFragment*>& _triangles_fragments);
	void merge(std::vector<SrFragment*>& _triangles_fragments, SrSSBuffer<RBColor32>& color, SrSSBuffer<float>& depth);

	void set_ps(SrShaderPixel* ps)
	{
		_stage_ps->set_pixel_shader(ps);
	}

	int get_width()
	{
		return int(_viewport_w+0.5);
	}

	int get_height()
	{
		return int(_viewport_h+0.5);
	}

	void clear()
	{

	}

	inline void set_depth_buffer(SrSSBuffer<float>* buf)
	{
		_depth_buffer = buf;
	}

	inline void set_color_buffer(SrSSBuffer<RBColor32>* buf)
	{
		_color_buffer = buf;
	}
	inline void clear_total() { _total_frag = 0; }
	inline int get_total() { return _total_frag; }
	inline void set_last_ts_time(float t) { _last_ts_time = t; }
private:
	SrStagePS* _stage_ps;
	SrStageOM* _stage_om;

	float _last_ts_time;

	float _viewport_w;
	float _viewport_h;

	SrSSBuffer<RBColor32>* _color_buffer;
	SrSSBuffer<float>* _depth_buffer;

	//排序后0~2是y增加顺序
	void _sort_y(SrTriangle* tri);
	//setup任意三角形
	void _set_tri(SrTriangle* tri, std::vector<SrFragment*>& _triangles_fragments);
	//setup平顶三角形
	void _set_top_tri(SrTriangle* tri, std::vector<SrFragment*>& _triangles_fragments);
	//setup平底三角形
	void _set_bottom_tri(SrTriangle* tri, std::vector<SrFragment*>& _triangles_fragments);
	//setup任意三角形（直接）
	void _new_set_tri(SrTriangle* tri, std::vector<SrFragment*>& _triangles_fragments);
	//setup任意三角形（直接2）
	void _new_set_tri2(SrTriangle* tri, std::vector<SrFragment*>& _triangles_fragments);

	//插值函数

	//位置线插,w按照1/z插值
	RBVector4 _lerp_position(RBVector4 start,RBVector4 end,float t);
	//法线线插
	RBVector3 _lerp_normal(RBVector3 start,RBVector3 end,float t);
	//uv按照1/z线插
	RBVector2 _lerp_uv(RBVector2 start,float sz,RBVector2 end,float ez,float t);


	//线扫+插值
	void scan_line(VertexP3N3T2& sv, VertexP3N3T2& ev, SrFragment* _triangle_fragment);

	//如果跳过返回true
	bool scan_line(VertexP3N3T2& sv, VertexP3N3T2& ev);

	Profiler _prof;

	SrSimGPU* _gpu;

	int _total_frag;
};

