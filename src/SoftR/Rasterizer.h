#pragma once
#include "StagePS.h"
#include "StageOM.h"
#include "InnerData.h"
#include <vector>
#include "..\\RBMath\\Inc\\Color32.h"
#include "..\\RBMath\\Inc\\AABB.h"
#include "../Profiler.h"

class SrSimGPU;
class SrRasterizer
{
public:
	SrRasterizer();
	SrRasterizer(SrSimGPU* gpu);
	~SrRasterizer();

	void set_viewport_shape(float w,float h);

	int near_far_cull(SrTriangle* _triangles, SrTriangle* _triangles_near_far_cull, int n);
	//顺时针
	int back_cull(SrTriangle* _triangles, SrTriangle* _triangles_back_cull,int n);

	int clip_besidenp(SrTriangle* _triangles, SrTriangle* _triangles_clip,int n);

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
	inline void set_main_buffer(SrSSBuffer<RBColor32>* buf)
	{
		_main_buffer = buf;
	}
	inline void set_depth_buffer(SrSSBuffer<float>* buf)
	{
		_depth_buffer = buf;
	}

	inline void set_color_buffer(SrTexture2D* buf)
	{
		_main_buffer = nullptr;
		_color_buffer = buf;
	}
	inline void clear_total() { _total_frag = 0; }
	inline int get_total_shade_frag() { return _total_frag; }
	inline void set_last_ts_time(float t) { _last_ts_time = t; }

	void trangle_setup_gpu_tiled(SrTriangle* tri, const RBAABB2D& quad, bool wireframe = false);

private:
	SrStagePS* _stage_ps;
	SrStageOM* _stage_om;

	float _last_ts_time;

	float _viewport_w;
	float _viewport_h;

	SrSSBuffer<RBColor32>* _main_buffer;
	SrTexture2D* _color_buffer;
	SrSSBuffer<float>* _depth_buffer;

	//排序后0~2是y增加顺序
	void _sort_y(SrTriangle* tri);
	void _new_set_tri2_tiled(SrTriangle* tri, const RBAABB2D& quad);
	//插值函数

	//位置线插,w按照1/z插值
	inline void _lerp_position(const RBVector4& start,const RBVector4& end,float t, RBVector4& v)
	{
		v = (1 - t)*start + t*end;
		//for perspective
		v.w = start.w * end.w / ((1 - t)*end.w + start.w*t);
	}
	//法线线插
	inline void _lerp_normal(const RBVector3& start,const RBVector3& end,float t, RBVector3& v)
	{
		v = start + t*(end - start);
	}
	//uv按照1/z线插
	inline void _lerp_uv(const RBVector2& start,float sz,const RBVector2& end,float ez,float t, RBVector2& v)
	{
		float factor = 1 / (sz*(t)+(1 - t)*ez);
		v = (ez*start*(1 - t) + t*sz*end)*factor;
	}
	//位置线插
	inline void _lerp_position_nopersp(const RBVector4& start, const RBVector4& end, float t,RBVector4& v)
	{
		v = (1 - t)*start + t*end;
		v.w = ((1 - t)*start.w + end.w*t);
	}
	//uv线插
	inline void _lerp_uv_nopersp(const RBVector2& start, const RBVector2& end, float t,RBVector2& v)
	{
		v = (1 - t)* start + t* end;
	}
	inline void _lerp_position_onlyzw(const RBVector4& start, const RBVector4& end, float t,RBVector4& v)
	{
		v.z = (1 - t)*start.z + t*end.z;
		v.w = start.w * end.w / ((1 - t)*end.w + start.w*t);
	}

	//如果跳过返回true
	bool scan_line_tiled(VertexP3N3T2& sv, VertexP3N3T2& ev, const RBAABB2D& quad);
	Profiler _prof;
public:
	SrSimGPU* _gpu;
private:
	atomic<int> _total_frag;

	bool _discard_invisible_tiled(const VertexP3N3T2& data, const RBAABB2D& quad);
	bool sss;
};

