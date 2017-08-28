#include "Rasterizer.h"
#include "../RBMath/Inc/RBMathBase.h"
#include <math.h>
#include "SimGPU.h"


SrRasterizer::SrRasterizer()
{
	_prof.init();

	_viewport_h = _viewport_w = 0;
	_stage_ps = new SrStagePS();
	_stage_om = new SrStageOM();
	_total_frag = 0;
}

SrRasterizer::SrRasterizer(SrSimGPU * gpu)
{
	_prof.init();

	_viewport_h = _viewport_w = 0;
	_stage_ps = new SrStagePS();
	_stage_om = new SrStageOM();

	for (int i = 0; i < thread_num; ++i)
	{
		gpu->set_stage(i, _stage_ps, _stage_om);
	}

	_gpu = gpu;
	_total_frag = 0;
}

SrRasterizer::~SrRasterizer()
{
	delete _stage_om;
	delete _stage_ps;
}

void SrRasterizer::set_viewport_shape(float w, float h)
{
	_viewport_w = w;
	_viewport_h = h;
}

void SrRasterizer::near_far_cull(std::vector<SrTriangle*> _triangles, std::vector<SrTriangle*>& _triangles_near_far_cull,int n)
{
	//clip space
	/*
	we use 
	-w<=x<=w
	-w<=y<=w
	0<=z<=w
	0<=w
	to clip triangles
	so we need not care dividing zero.
	*/
	/*
	for (auto tri : _triangles)
	{
		if (tri->v[0].position.w <0||
			tri->v[1].position.w <0||
			tri->v[2].position.w <0)
		{
			continue;
		}

		//这里依然只做近远面裁剪，其他四周的裁剪在光栅化的时候以fragment为单位做
		//注意近面裁剪是一定要做的，因为不做近面会出现错误，其他的做不做和效率有关，但是最好是在triangle阶段做


	}
	*/


	//xyz除以w，但是保留w为z
			for(auto tri : _triangles)
			{
		float inv_w = 1.f / tri->v[0].position.w;
		float x1 = tri->v[0].position.x * inv_w;
		float y1 = tri->v[0].position.y * inv_w;
		float z1 = tri->v[0].position.z * inv_w;

		inv_w = 1.f / tri->v[1].position.w;
		float x2 = tri->v[1].position.x * inv_w;
		float y2 = tri->v[1].position.y * inv_w;
		float z2 = tri->v[1].position.z * inv_w;

		inv_w = 1.f / tri->v[2].position.w;
		float x3 = tri->v[2].position.x * inv_w;
		float y3 = tri->v[2].position.y * inv_w;
		float z3 = tri->v[2].position.z * inv_w;
		
		float far_z = 1;
		
		tri->v[0].position.x = x1;
		tri->v[0].position.y = y1;
		tri->v[0].position.z = z1;

		tri->v[1].position.x = x2;
		tri->v[1].position.y = y2;
		tri->v[1].position.z = z2;

		tri->v[2].position.x = x3;
		tri->v[2].position.y = y3;
		tri->v[2].position.z = z3;
		

		if((z1>far_z&&z2>far_z&&z3>far_z)||(z1<0&&z2<0&&z3<0))
		{
			continue;
		}

		if ( z1 >= 0 && z2 >= 0 && z3 >= 0)
		{
			_triangles_near_far_cull.push_back(tri);
			continue;
		}

		//处理所有近裁剪面相交的三角形,保绕序
		
		if (z1 < 0 && z2>0 && z3 > 0)
		{
			float t = -z1 / (z2 - z1);
			VertexP3N3T2 newv1;
			newv1.position = _lerp_position(tri->v[0].position,tri->v[1].position,t);
			newv1.normal = _lerp_normal(tri->v[0].normal,tri->v[1].normal,t);
			newv1.text_coord = _lerp_uv(tri->v[0].text_coord,tri->v[0].position.w,tri->v[1].text_coord,tri->v[1].position.w,t);


			t = -z1 / (z3 - z1);
			VertexP3N3T2 newv2;
			newv2.position = _lerp_position(tri->v[0].position, tri->v[2].position, t);
			newv2.normal = _lerp_normal(tri->v[0].normal, tri->v[2].normal, t);
			newv2.text_coord = _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[2].text_coord, tri->v[2].position.w, t);
			SrTriangle* addtri = new SrTriangle(newv1,tri->v[2],newv2);
			_triangles_near_far_cull.push_back(addtri);

			tri->v[0] = newv1;
			_triangles_near_far_cull.push_back(tri);
		}
		if (z1 > 0 && z2 < 0 && z3>0)
		{
			float t = -z2 / (z3 - z2);
			VertexP3N3T2 newv1;
			newv1.position = _lerp_position(tri->v[1].position, tri->v[2].position, t);
			newv1.normal = _lerp_normal(tri->v[1].normal, tri->v[2].normal, t);
			newv1.text_coord = _lerp_uv(tri->v[1].text_coord, tri->v[1].position.w, tri->v[2].text_coord, tri->v[2].position.w, t);



			t = -z2 / (z1 - z2);
			VertexP3N3T2 newv2;
			newv2.position = _lerp_position(tri->v[1].position, tri->v[0].position, t);
			newv2.normal = _lerp_normal(tri->v[1].normal, tri->v[0].normal, t);
			newv2.text_coord = _lerp_uv(tri->v[1].text_coord, tri->v[1].position.w, tri->v[0].text_coord, tri->v[0].position.w, t);
			SrTriangle* addtri = new SrTriangle(newv1, tri->v[0], newv2);
			_triangles_near_far_cull.push_back(addtri);
			

			tri->v[1] = newv1;
			_triangles_near_far_cull.push_back(tri);
			
		}
		if (z1 > 0 && z2 > 0 && z3 < 0)
		{
			float t = -z3 / (z1 - z3);
			VertexP3N3T2 newv1;
			newv1.position = _lerp_position(tri->v[2].position, tri->v[0].position, t);
			newv1.normal = _lerp_normal(tri->v[2].normal, tri->v[0].normal, t);
			newv1.text_coord = _lerp_uv(tri->v[2].text_coord, tri->v[2].position.w, tri->v[0].text_coord, tri->v[0].position.w, t);
		
			t = -z3 / (z2 - z3);
			VertexP3N3T2 newv2;
			newv2.position = _lerp_position(tri->v[2].position, tri->v[1].position, t);
			newv2.normal = _lerp_normal(tri->v[2].normal, tri->v[1].normal, t);
			newv2.text_coord = _lerp_uv(tri->v[2].text_coord, tri->v[2].position.w, tri->v[1].text_coord, tri->v[1].position.w, t);
			SrTriangle* addtri = new SrTriangle(newv1, tri->v[1], newv2);
			_triangles_near_far_cull.push_back(addtri);

			tri->v[2] = newv1;
			_triangles_near_far_cull.push_back(tri);
		}


		if (z1>0 && z2 < 0 && z3 < 0)
		{
			float t = -z3 / (z1 - z3);
			VertexP3N3T2 newv1;
			newv1.position = _lerp_position(tri->v[2].position, tri->v[0].position, t);
			newv1.normal = _lerp_normal(tri->v[2].normal, tri->v[0].normal, t);
			newv1.text_coord = _lerp_uv(tri->v[2].text_coord, tri->v[2].position.w, tri->v[0].text_coord, tri->v[0].position.w, t);
			
			t = -z2 / (z1 - z2);
			VertexP3N3T2 newv2;
			newv2.position = _lerp_position(tri->v[1].position, tri->v[0].position, t);
			newv2.normal = _lerp_normal(tri->v[1].normal, tri->v[0].normal, t);
			newv2.text_coord = _lerp_uv(tri->v[1].text_coord, tri->v[1].position.w, tri->v[0].text_coord, tri->v[0].position.w, t);
			tri->v[2] = newv1;
			tri->v[1] = newv2;
			_triangles_near_far_cull.push_back(tri);
		}
		if (z1 < 0 && z2 < 0 && z3 > 0)
		{
			float t = -z2 / (z3 - z2);
			VertexP3N3T2 newv1;
			newv1.position = _lerp_position(tri->v[1].position, tri->v[2].position, t);
			newv1.normal = _lerp_normal(tri->v[1].normal, tri->v[2].normal, t);
			newv1.text_coord = _lerp_uv(tri->v[1].text_coord, tri->v[1].position.w, tri->v[2].text_coord, tri->v[2].position.w, t);
			
			t = -z1 / (z3 - z1);
			VertexP3N3T2 newv2;
			newv2.position = _lerp_position(tri->v[0].position, tri->v[2].position, t);
			newv2.normal = _lerp_normal(tri->v[0].normal, tri->v[2].normal, t);
			newv2.text_coord = _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[2].text_coord, tri->v[2].position.w, t);
			tri->v[1] = newv1;
			tri->v[0] = newv2;
			_triangles_near_far_cull.push_back(tri);
		}
		if (z1 < 0 && z2 > 0 && z3 < 0)
		{
			float t = -z1 / (z2 - z1);
			VertexP3N3T2 newv1;
			newv1.position = _lerp_position(tri->v[0].position, tri->v[1].position, t);
			newv1.normal = _lerp_normal(tri->v[0].normal, tri->v[1].normal, t);
			newv1.text_coord = _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[1].text_coord, tri->v[1].position.w, t);
			
			t = -z3 / (z2 - z3);
			VertexP3N3T2 newv2;
			newv2.position = _lerp_position(tri->v[2].position, tri->v[1].position, t);
			newv2.normal = _lerp_normal(tri->v[2].normal, tri->v[1].normal, t);
			newv2.text_coord = _lerp_uv(tri->v[2].text_coord, tri->v[2].position.w, tri->v[1].text_coord, tri->v[1].position.w, t);
			tri->v[0] = newv1;
			tri->v[2] = newv2;
			_triangles_near_far_cull.push_back(tri);
		}
		
	}
	
}

void SrRasterizer::back_cull(std::vector<SrTriangle*> _triangles, std::vector<SrTriangle*>& _triangles_back_cull)
{
	for (auto tri : _triangles)
	{
		RBVector4 v1 = tri->v[1].position - tri->v[0].position;
		RBVector4 v2 = tri->v[2].position - tri->v[0].position;
		RBVector4 cr = v1 ^ v2;
		RBVector4 z(0,0,1);
		float res = RBVector4::dot3(cr, z);
		if (res<0)
		{
			_triangles_back_cull.push_back(tri);
		}
		else
		{
			//_triangles_back_cull.push_back(tri);

			continue;
		}
	}
}

void SrRasterizer::clip(std::vector<SrTriangle*> _triangles, std::vector<SrTriangle*>& _triangles_clip)
{
	for (auto tri : _triangles)
	{
		_triangles_clip.push_back(tri);
		//裁剪
	}
}

void SrRasterizer::triangle_setup(std::vector<SrTriangle*> _triangles, std::vector<SrFragment*>& _triangles_fragments)
{
	//_prof.set_begin();
	for (auto tri : _triangles)
	{
		//把坐标转换到视口
		tri->v[0].position.x = tri->v[0].position.x*0.5*_viewport_w;
		tri->v[0].position.y = tri->v[0].position.y*0.5*_viewport_h;
		tri->v[1].position.x = tri->v[1].position.x*0.5*_viewport_w;
		tri->v[1].position.y = tri->v[1].position.y*0.5*_viewport_h;
		tri->v[2].position.x = tri->v[2].position.x*0.5*_viewport_w;
		tri->v[2].position.y = tri->v[2].position.y*0.5*_viewport_h;

		//检测三角形是否退化为直线
		
		if(RBMath::is_nearly_equal(tri->v[0].position.y, tri->v[1].position.y) && RBMath::is_nearly_equal(tri->v[1].position.y, tri->v[2].position.y)||
			RBMath::is_nearly_equal(tri->v[0].position.x, tri->v[1].position.x) && RBMath::is_nearly_equal(tri->v[1].position.x, tri->v[2].position.x))
			continue;
			
		/*
		//整形化
		tri->v[0].position.x = int (tri->v[0].position.x+0.5);
		tri->v[0].position.y = int (tri->v[0].position.y+0.5);
		tri->v[1].position.x = int (tri->v[1].position.x+0.5);
		tri->v[1].position.y = int (tri->v[1].position.y+0.5);
		tri->v[2].position.x = int (tri->v[2].position.x+0.5);
		tri->v[2].position.y = int (tri->v[2].position.y+0.5);

		//检测三角形是否退化为直线
		if (RBMath::is_nearly_equal(tri->v[0].position.y, tri->v[1].position.y) && RBMath::is_nearly_equal(tri->v[1].position.y, tri->v[2].position.y) ||
			RBMath::is_nearly_equal(tri->v[0].position.x, tri->v[1].position.x) && RBMath::is_nearly_equal(tri->v[1].position.x, tri->v[2].position.x))
			continue;
		*/

		/*
		float inv_w = 1.f / tri->v[0].position.w;
		float x1 = tri->v[0].position.x * inv_w*_viewport_w*0.5;
		float y1 = tri->v[0].position.y * inv_w*_viewport_h*0.5;
		float z1 = tri->v[0].position.z * inv_w;

		inv_w = 1.f / tri->v[1].position.w;
		float x2 = tri->v[1].position.x * inv_w*_viewport_w*0.5;
		float y2 = tri->v[1].position.y * inv_w*_viewport_h*0.5;
		float z2 = tri->v[1].position.z * inv_w;

		inv_w = 1.f / tri->v[2].position.w;
		float x3 = tri->v[2].position.x * inv_w*_viewport_w*0.5;
		float y3 = tri->v[2].position.y * inv_w*_viewport_h*0.5;
		float z3 = tri->v[2].position.z * inv_w;
		*/
		
		float ym, xm, zm,
			yu, xu, zu,
			yd, xd, zd;
		
		_sort_y(tri);

		/*
		if (RBMath::is_nearly_equal(tri->v[0].position.y, tri->v[1].position.y) )
		{
			//左右排序
			if (tri->v[0].position.x > tri->v[1].position.x)
			{
				VertexP3N3T2 temp = tri->v[1];
				tri->v[1] = tri->v[0];
				tri->v[0] = temp;
			}
			_set_bottom_tri(tri, _triangles_fragments);
		}
		else if (RBMath::is_nearly_equal(tri->v[1].position.y, tri->v[2].position.y))
		{
			//左右排序
			if (tri->v[1].position.x > tri->v[2].position.x)
			{
				VertexP3N3T2 temp = tri->v[1];
				tri->v[1] = tri->v[2];
				tri->v[2] = temp;
			}
			_set_top_tri(tri, _triangles_fragments);
		}
		else
		*/
		{
		
			//_set_tri(tri, _triangles_fragments);
			//_new_set_tri(tri, _triangles_fragments);
			_new_set_tri2(tri, _triangles_fragments);
		}

	}
	//_prof.set_end(0);
	//_prof.out_put_after_time(20);
}

void SrRasterizer::shade(std::vector<SrFragment*>& _triangles_fragments)
{
	//_stage_ps->proccess(_triangles_fragments);
}


void SrRasterizer::merge(std::vector<SrFragment*>& _triangles_fragments, SrSSBuffer<RBColor32>& color, SrSSBuffer<float>& depth)
{
	_stage_om->proccess(_triangles_fragments, color, depth);
}

void SrRasterizer::_sort_y(SrTriangle* tri)
{
	//<
	if (tri->v[0].position.y > tri->v[1].position.y)
	{
		VertexP3N3T2 temp = tri->v[0];
		tri->v[0] = tri->v[1];
		tri->v[1] = temp;
	}
	if (tri->v[0].position.y > tri->v[2].position.y)
	{
		VertexP3N3T2 temp = tri->v[0];
		tri->v[0] = tri->v[2];
		tri->v[2] = temp;
	}
	if (tri->v[1].position.y > tri->v[2].position.y)
	{
		VertexP3N3T2 temp = tri->v[1];
		tri->v[1] = tri->v[2];
		tri->v[2] = temp;
	}

	/*
	if ((y1 > y2&&y2<y3) || (y2>y3&&y2 < y1))
	{
		ym = y2;
		xm = x2;
		zm = z2;

		if (y1 > y2)
		{
			yu = y1; yd = y3;
			xu = x1; xd = x3;
			zu = z1; zd = z3;
		}
		else
		{
			yu = y3; yd = y1;
			xu = x3; xd = x1;
			zu = z3; zd = z1;
		}
	}
	if ((y2 < y1&&y1<y3) || (y1>y3&&y1 < y2))
	{
		ym = y1;
		xm = x1;
		zm = z1;
		if (y1 < y2)
		{
			yu = y2; yd = y3;
			xu = x2; xd = x3;
			zu = z2; zd = z3;
		}
		else
		{
			yu = y3; yd = y1;
			xu = x3; xd = x1;
			zu = z3; zd = z1;
		}
	}
	if ((y3 > y1&&y3<y2) || (y3>y2&&y3 < y1))
	{
		ym = y3;
		xm = x3;
		zm = z3;
		if (y3 < y1)
		{
			yu = y1; yd = y2;
			xu = x1; xd = x2;
			zu = z1; zd = z2;
		}
		else
		{
			yu = y2; yd = y1;
			xu = x2; xd = x1;
			zu = z2; zd = z1;
		}
	}
	*/
}

void SrRasterizer::_set_tri(SrTriangle* tri, std::vector<SrFragment*>& _triangles_fragments)
{

	//!!!!!!
	//切出来的新顶点在ceil了之后不一定在同一条线上，所以导致计算出来的数据差距很大。

	//float new_x = tri->v[2].normal.x + (tri->v[2].position.y - tri->v[1].position.y) / (tri->v[2].position.y - tri->v[0].position.y)*(tri->v[0].position.x - tri->v[2].position.x);
	VertexP3N3T2 new_v;
	float t = (tri->v[1].position.y - tri->v[0].position.y) / (tri->v[2].position.y - tri->v[0].position.y);
	new_v.position = _lerp_position(tri->v[0].position, tri->v[2].position,t);
	new_v.normal = _lerp_normal(tri->v[0].normal, tri->v[2].normal, t);
	new_v.text_coord = _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[2].text_coord, tri->v[2].position.w, t);



	SrTriangle up_tri(VertexP3N3T2(tri->v[0]), VertexP3N3T2(tri->v[1]), new_v);
	//左右排序<
	if (up_tri.v[1].position.x > up_tri.v[2].position.x)
	{
		VertexP3N3T2 temp = up_tri.v[1];
		up_tri.v[1] = up_tri.v[2];
		up_tri.v[2] = temp;
	}
	SrTriangle down_tri(new_v,VertexP3N3T2(tri->v[1]), VertexP3N3T2(tri->v[2]));
	//左右排序<
	if (down_tri.v[0].position.x > down_tri.v[1].position.x)
	{
		VertexP3N3T2 temp = down_tri.v[1];
		down_tri.v[1] = down_tri.v[0];
		down_tri.v[0] = temp;
	}

	_set_bottom_tri(&down_tri,_triangles_fragments);
	_set_top_tri(&up_tri,_triangles_fragments);
}

void SrRasterizer::_set_top_tri(SrTriangle* tri, std::vector<SrFragment*>& _triangles_fragments)
{
	float dx_left = (tri->v[1].position.x - tri->v[0].position.x) / (tri->v[1].position.y - tri->v[0].position.y);
	float dx_right = (tri->v[2].position.x - tri->v[0].position.x) / (tri->v[2].position.y - tri->v[0].position.y);
	//char a[64];
	//sprintf(a, "================(%f %f)\n", dx_left, dx_right);
	//RBlog(a);
	float x0 = tri->v[0].position.x;
	int y0 = ceil(tri->v[0].position.y - 0.5);
	//int x1 = ceil(tri->v[1].position.x - 0.5);
	//int y1 = ceil(tri->v[1].position.y - 0.5);
	//int x2 = ceil(tri->v[2].position.x - 0.5);
	int y2 = ceil(tri->v[2].position.y - 0.5);

	/*
	tri->v[0].position.x = x0;
	tri->v[0].position.y = y0;
	tri->v[1].position.x = x1;
	tri->v[1].position.y = y1;
	tri->v[2].position.x = x2;
	tri->v[2].position.y = y2;
	*/

	//检测三角形是否退化为直线
	if (RBMath::is_nearly_equal(tri->v[0].position.y, tri->v[1].position.y) && RBMath::is_nearly_equal(tri->v[1].position.y, tri->v[2].position.y) ||
		RBMath::is_nearly_equal(tri->v[0].position.x, tri->v[1].position.x) && RBMath::is_nearly_equal(tri->v[1].position.x, tri->v[2].position.x))
		return;

	//dx_left = ((float)(x1 - x0)) / float(y1 - y0);
	//dx_right = ((float)(x2 - x0)) / float(y2 - y0);

	SrFragment* frg = new SrFragment();

	int loop_y = 0;
	float xs = x0, xe = x0;
	float loop_x = 0;
	float t;
	float dy = y2 - y0+1;
	for (loop_y = 0; loop_y < y2-y0; ++loop_y)
	{

		char a[64];
		sprintf(a, "%f %f\n", xs, xe);
		//RBlog(a);
		t = loop_y / dy;


		VertexP3N3T2 vs;
		vs.position = _lerp_position(tri->v[0].position, tri->v[1].position, t);
		vs.position.y = loop_y + y0;
		vs.position.x = xs;
		vs.normal = _lerp_normal(tri->v[0].normal, tri->v[1].normal, t);
		vs.text_coord = _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[1].text_coord, tri->v[1].position.w, t);

		VertexP3N3T2 ve;
		ve.position = _lerp_position(tri->v[0].position, tri->v[2].position, t);
		ve.position.y = loop_y + y0;
		ve.position.x = xe;
		ve.normal = _lerp_normal(tri->v[0].normal, tri->v[2].normal, t);
		ve.text_coord = _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[2].text_coord, tri->v[2].position.w, t);

		scan_line(vs, ve, frg);

		xs += dx_left;
		xe += dx_right;

	}
	//RBlog("tri top\n");
	_triangles_fragments.push_back(frg);
}

void SrRasterizer::_set_bottom_tri(SrTriangle* tri, std::vector<SrFragment*>& _triangles_fragments)
{
	//issue
	float dx_left = (tri->v[2].position.x - tri->v[0].position.x) / (tri->v[2].position.y - tri->v[0].position.y);
	float dx_right = (tri->v[2].position.x - tri->v[1].position.x) / (tri->v[2].position.y - tri->v[1].position.y);

	/*
	char a[64];
	sprintf(a, "================(%f %f)\n", dx_left, dx_right);
	RBlog(a);
	*/
	float x0 = tri->v[0].position.x;
	int y0 = ceil(tri->v[0].position.y - 0.5);
	float x1 = tri->v[1].position.x;
	int y1 = ceil(tri->v[1].position.y - 0.5);
	float x2 = tri->v[2].position.x;
	int y2 = ceil(tri->v[2].position.y - 0.5);

	/*
	tri->v[0].position.x = x0;
	tri->v[0].position.y = y0;
	tri->v[1].position.x = x1;
	tri->v[1].position.y = y1;
	tri->v[2].position.x = x2;
	tri->v[2].position.y = y2;
	*/

	//dx_left = ((float)(x2 - x0)) / (y2 - y0);
	//dx_right = ((float)(x2 - x1)) / (y2 - y1);

	SrFragment* frg = new SrFragment();

	int loop_y = 0;
	float xs = x0, xe = x1;
	float loop_x = 0;
	float t;
	float dy = y2 - y0 +1;
	for (loop_y = 0;loop_y<y2-y0;++loop_y)
	{
		t = loop_y / dy;
		char a[64];
		sprintf(a, "%f %f\n", xs, xe);
		//RBlog(a);

		VertexP3N3T2 vs;
		vs.position = _lerp_position(tri->v[0].position,tri->v[2].position,t);
		vs.position.y = loop_y + y0;
		vs.position.x = xs;
		vs.normal = _lerp_normal(tri->v[0].normal,tri->v[2].normal,t);
		vs.text_coord = _lerp_uv(tri->v[0].text_coord,tri->v[0].position.w,tri->v[2].text_coord,tri->v[2].position.w,t);

		VertexP3N3T2 ve;
		ve.position = _lerp_position(tri->v[1].position, tri->v[2].position, t);
		ve.position.y = loop_y + y0;
		ve.position.x = xe;
		ve.normal = _lerp_normal(tri->v[1].normal, tri->v[2].normal, t);
		ve.text_coord = _lerp_uv(tri->v[1].text_coord, tri->v[1].position.w, tri->v[2].text_coord, tri->v[2].position.w, t);

		scan_line(vs, ve, frg);

		xs += dx_left;
		xe += dx_right;
	}
	//RBlog("tri bottom\n");
	_triangles_fragments.push_back(frg);

}

void SrRasterizer::_new_set_tri(SrTriangle* tri, std::vector<SrFragment*>& _triangles_fragments)
{
	//!!!性能issue
	SrFragment* frg = new SrFragment();
	/*
	//左右排序<
	if (tri->v[0].position.x > tri->v[1].position.x)
	{
	VertexP3N3T2 temp = tri->v[1];
	tri->v[1] = tri->v[0];
	tri->v[0] = temp;
	}
	*/
	//用于判断左右三角
	float tc = (tri->v[1].position.y - tri->v[0].position.y) / (tri->v[2].position.y - tri->v[0].position.y);
	float xc = tri->v[0].position.x*(1 - tc) + tri->v[2].position.x*tc;

	//0->2
	if (tri->v[1].position.x > xc)
	{
		//y中点在右边
		int i = 0;

		//y中点在右边
		float dx_left_02 = (tri->v[2].position.x - tri->v[0].position.x) / (tri->v[2].position.y - tri->v[0].position.y);

		float dx_right_12 = (tri->v[2].position.x - tri->v[1].position.x) / (tri->v[2].position.y - tri->v[1].position.y);
		float dx_right_01 = (tri->v[1].position.x - tri->v[0].position.x) / (tri->v[1].position.y - tri->v[0].position.y);

		float x0 = tri->v[0].position.x;
		int y0 = ceil(tri->v[0].position.y - 0.5);
		float x1 = tri->v[1].position.x;
		int y1 = ceil(tri->v[1].position.y - 0.5);
		float x2 = tri->v[2].position.x;
		int y2 = ceil(tri->v[2].position.y - 0.5);

		VertexP3N3T2 vs;
		VertexP3N3T2 ve;

		float tt = (y0 - tri->v[0].position.y) / (tri->v[2].position.y - tri->v[0].position.y);
		RBVector4 temps = _lerp_position(tri->v[0].position, tri->v[2].position, tt);

		tt = (y0 - tri->v[0].position.y) / (tri->v[1].position.y - tri->v[0].position.y);
		RBVector4 temps2 = _lerp_position(tri->v[0].position, tri->v[1].position, tt);

		//float cur_right_dx = dx_right_01;
		int loop_y = 0;
		float dy = y2 - y0 + 1;
		float dy1 = y1 - y0 + 1;
		float t;
		float xs = x0, xe = x0;
		for (loop_y = 0; loop_y < y1 - y0; ++loop_y)
		{
			t = loop_y / dy;
			if (y0 >= y1)
			{
				return;
			}
			i++;

			//vs.position = _lerp_position(tri->v[0].position, tri->v[2].position, t);
			vs.position = _lerp_position(temps, tri->v[2].position, t);
			vs.position.y = loop_y + y0;
			vs.position.x = xs;
			vs.normal = _lerp_normal(tri->v[0].normal, tri->v[2].normal, t);
			vs.text_coord = _lerp_uv(tri->v[0].text_coord, temps.w, tri->v[2].text_coord, tri->v[2].position.w, t);

			t = loop_y / dy1;
			//ve.position = _lerp_position(tri->v[0].position, tri->v[1].position, t);
			ve.position = _lerp_position(temps2, tri->v[1].position, t);
			ve.position.y = loop_y + y0;
			ve.position.x = xe;
			ve.normal = _lerp_normal(tri->v[0].normal, tri->v[1].normal, t);
			ve.text_coord = _lerp_uv(tri->v[0].text_coord, temps2.w, tri->v[1].text_coord, tri->v[1].position.w, t);

			scan_line(vs, ve, frg);

			xs += dx_left_02;
			xe += dx_right_01;
		}

		dy = y2 - y0 + 1;
		VertexP3N3T2 vnew;
		vs.position = _lerp_position(tri->v[0].position, tri->v[2].position, (loop_y) / dy);
		vs.position = _lerp_position(temps, tri->v[2].position, (loop_y) / dy);
		vs.position.y = y1;
		vs.position.x = xs;
		vs.normal = _lerp_normal(tri->v[0].normal, tri->v[2].normal, (loop_y) / dy);
		vs.text_coord = _lerp_uv(tri->v[0].text_coord, temps.w, tri->v[2].text_coord, tri->v[2].position.w, (loop_y) / dy);
		vnew = vs;


		tt = (y1 - tri->v[1].position.y) / (tri->v[2].position.y - tri->v[1].position.y);
		temps2 = _lerp_position(tri->v[1].position, tri->v[2].position, tt);

		float dy_test = y2 - y0 + 1;
		float ttty = loop_y;

		dy = y2 - y1 + 1;
		xs = vnew.position.x;
		xe = x1;
		for (loop_y = 0; loop_y < y2 - y1; ++loop_y)
		{
			t = loop_y / (dy);
			if (y1 >= y2)
			{
				return;
			}
			/*
			vs.position = _lerp_position(vnew.position, tri->v[2].position, t);
			vs.position.y = loop_y + y1;
			vs.position.x = xs;
			vs.normal = _lerp_normal(vnew.normal, tri->v[2].normal, t);
			vs.text_coord = _lerp_uv(vnew.text_coord, vnew.position.w, tri->v[2].text_coord, tri->v[2].position.w, t);
			*/
			float tttt = ttty / dy_test;
			vs.position = _lerp_position(tri->v[0].position, tri->v[2].position, tttt);
			vs.position.y = loop_y + y1;
			vs.position.x = xs;
			vs.normal = _lerp_normal(tri->v[0].normal, tri->v[2].normal, tttt);
			vs.text_coord = _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[2].text_coord, tri->v[2].position.w, tttt);

			ttty++;

			ve.position = _lerp_position(temps2, tri->v[2].position, t);
			ve.position.y = loop_y + y1;
			ve.position.x = xe;
			ve.normal = _lerp_normal(tri->v[1].normal, tri->v[2].normal, t);
			ve.text_coord = _lerp_uv(tri->v[1].text_coord, temps2.w, tri->v[2].text_coord, tri->v[2].position.w, t);

			scan_line(vs, ve, frg);

			xs += dx_left_02;
			xe += dx_right_12;
			i++;
		}

		i = 0;

		
	}
	else 
	{
		//y中点在左边
		float dx_right_02 = (tri->v[2].position.x - tri->v[0].position.x) / (tri->v[2].position.y - tri->v[0].position.y);

		float dx_left_12 = (tri->v[2].position.x - tri->v[1].position.x) / (tri->v[2].position.y - tri->v[1].position.y);
		float dx_left_01 = (tri->v[1].position.x - tri->v[0].position.x) / (tri->v[1].position.y - tri->v[0].position.y);

		float x0 = tri->v[0].position.x;
		int y0 = ceil(tri->v[0].position.y - 0.5);
		float x1 = tri->v[1].position.x;
		int y1 = ceil(tri->v[1].position.y - 0.5);
		float x2 = tri->v[2].position.x;
		int y2 = ceil(tri->v[2].position.y - 0.5);

		VertexP3N3T2 vs;
		VertexP3N3T2 ve;

		//float cur_right_dx = dx_right_01;
		int loop_y = 0;
		float dy = y2 - y0 + 1;
		float dy1 = y1 - y0 + 1;
		float t;
		float xs = x0, xe = x0;
		for (loop_y = 0; loop_y < y1 - y0; ++loop_y)
		{
			t = loop_y / dy;
			if (y0 > y1)
			{
				return;
			}

			vs.position = _lerp_position(tri->v[0].position, tri->v[2].position, t);
			vs.position.y = loop_y + y0;
			vs.position.x = xs;
			vs.normal = _lerp_normal(tri->v[0].normal, tri->v[2].normal, t);
			vs.text_coord = _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[2].text_coord, tri->v[2].position.w, t);

			t = loop_y / dy1;

			ve.position = _lerp_position(tri->v[0].position, tri->v[1].position, t);
			ve.position.y = loop_y + y0;
			ve.position.x = xe;
			ve.normal = _lerp_normal(tri->v[0].normal, tri->v[1].normal, t);
			ve.text_coord = _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[1].text_coord, tri->v[1].position.w, t);

			scan_line(ve, vs, frg);

			xs += dx_right_02;
			xe += dx_left_01;
		}

		VertexP3N3T2 vnew;
		dy = y2 - y0 + 1;
		vs.position = _lerp_position(tri->v[0].position, tri->v[2].position, (loop_y) / dy);
		vs.position.y = y1;
		vs.position.x = xs;
		vs.normal = _lerp_normal(tri->v[0].normal, tri->v[2].normal, (loop_y ) / dy);
		vs.text_coord = _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[2].text_coord, tri->v[2].position.w, (loop_y ) / dy);
		vnew = vs;

		dy = y2 - y1 + 1;
		xs = vnew.position.x;
		xe = x1;
		for (loop_y = 0; loop_y < y2 - y1; ++loop_y)
		{
			if (y1>y2)
			{
				return;
			}
			t = loop_y / dy;

			vs.position = _lerp_position(vnew.position, tri->v[2].position, t);
			vs.position.y = loop_y + y1;
			vs.position.x = xs;
			vs.normal = _lerp_normal(vnew.normal, tri->v[2].normal, t);
			vs.text_coord = _lerp_uv(vnew.text_coord, vnew.position.w, tri->v[2].text_coord, tri->v[2].position.w, t);

			ve.position = _lerp_position(tri->v[1].position, tri->v[2].position, t);
			ve.position.y = loop_y + y1;
			ve.position.x = xe;
			ve.normal = _lerp_normal(tri->v[1].normal, tri->v[2].normal, t);
			ve.text_coord = _lerp_uv(tri->v[1].text_coord, tri->v[1].position.w, tri->v[2].text_coord, tri->v[2].position.w, t);

			scan_line(ve, vs, frg);

			xs += dx_right_02;
			xe += dx_left_12;

		}
	}

	_triangles_fragments.push_back(frg);
}

void SrRasterizer::_new_set_tri2(SrTriangle* tri, std::vector<SrFragment*>& _triangles_fragments)
{
	//!!!性能issue
	//SrFragment* frg = new SrFragment();
	//用于判断左右三角
	float tc = (tri->v[1].position.y - tri->v[0].position.y) / (tri->v[2].position.y - tri->v[0].position.y);
	float xc = tri->v[0].position.x*(1 - tc) + tri->v[2].position.x*tc;

	//0->2
	if (tri->v[1].position.x >= xc)
	{
		//y中点在右边
		float x0 = tri->v[0].position.x;
		int y0 = ceil(tri->v[0].position.y - 0.5);
		float x1 = tri->v[1].position.x;
		int y1 = ceil(tri->v[1].position.y - 0.5);
		float x2 = tri->v[2].position.x;
		int y2 = ceil(tri->v[2].position.y - 0.5);

		tri->v[0].position.y = y0;
		tri->v[1].position.y = y1;
		tri->v[2].position.y = y2;

		//检测三角形是否退化为直线
		if (RBMath::is_nearly_equal(tri->v[0].position.y, tri->v[1].position.y) && RBMath::is_nearly_equal(tri->v[1].position.y, tri->v[2].position.y) ||
			RBMath::is_nearly_equal(tri->v[0].position.x, tri->v[1].position.x) && RBMath::is_nearly_equal(tri->v[1].position.x, tri->v[2].position.x))
			return;

		float dx_left_02 = (tri->v[2].position.x - tri->v[0].position.x) / (tri->v[2].position.y - tri->v[0].position.y);

		float dx_right_12 = (tri->v[2].position.x - tri->v[1].position.x) / (tri->v[2].position.y - tri->v[1].position.y);
		float dx_right_01 = (tri->v[1].position.x - tri->v[0].position.x) / (tri->v[1].position.y - tri->v[0].position.y);

		VertexP3N3T2 ph;
		VertexP3N3T2 ph1;
		VertexP3N3T2 p11;
		VertexP3N3T2 p21;
		VertexP3N3T2 p22;

		float tempt = (tri->v[1].position.y - tri->v[0].position.y) / (tri->v[2].position.y - tri->v[0].position.y);
		ph.position = _lerp_position(tri->v[0].position, tri->v[2].position, tempt);
		ph.position.y = RBMath::round_f(ph.position.y);
		ph.normal = _lerp_normal(tri->v[0].normal, tri->v[2].normal, tempt);
		ph.text_coord = _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[2].text_coord, tri->v[2].position.w, tempt);

		tempt = (tri->v[1].position.y - 1 - tri->v[0].position.y) / (tri->v[2].position.y - tri->v[0].position.y);
		ph1.position = _lerp_position(tri->v[0].position, tri->v[2].position, tempt);
		ph1.position.y = RBMath::round_f(ph1.position.y);
		ph1.normal = _lerp_normal(tri->v[0].normal, tri->v[2].normal, tempt);
		ph1.text_coord = _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[2].text_coord, tri->v[2].position.w, tempt);

		tempt = (tri->v[1].position.y - 1 - tri->v[0].position.y) / (tri->v[1].position.y - tri->v[0].position.y);
		p11.position = _lerp_position(tri->v[0].position, tri->v[1].position, tempt);
		p11.position.y = RBMath::round_f(p11.position.y);
		p11.normal = _lerp_normal(tri->v[0].normal, tri->v[1].normal, tempt);
		p11.text_coord = _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[1].text_coord, tri->v[1].position.w, tempt);

		tempt = (tri->v[2].position.y - 1 - ph.position.y) / (y2 - ph.position.y);
		p21.position = _lerp_position(ph.position, tri->v[2].position, tempt);
		p21.position.y = RBMath::round_f(p21.position.y );
		p21.normal = _lerp_normal(ph.normal, tri->v[2].normal, tempt);
		p21.text_coord = _lerp_uv(ph.text_coord, ph.position.w, tri->v[2].text_coord, tri->v[2].position.w, tempt);

		p22.position = _lerp_position(tri->v[1].position, tri->v[2].position, tempt);
		p22.position.y = RBMath::round_f(p22.position.y );
		p22.normal = _lerp_normal(tri->v[1].normal, tri->v[2].normal, tempt);
		p22.text_coord = _lerp_uv(tri->v[1].text_coord, tri->v[1].position.w, tri->v[2].text_coord, tri->v[2].position.w, tempt);

		VertexP3N3T2 vs;
		VertexP3N3T2 ve;

		float t = 0.f;
		int loop_y = 0;
		float dyl = ph1.position.y - y0;
		float dyr = p11.position.y - y0;
		float xs = x0;
		float xe = x0;
		for (loop_y = y0; loop_y <= ph1.position.y; ++loop_y)
		{
			if (RBMath::is_nearly_equal(dyl, 0.f))
			{
				t = 0;
			}
			else
			{
				t = (loop_y - y0) / dyl;
			}
			
			vs.position = _lerp_position(tri->v[0].position, ph1.position, t);
			vs.position.y = loop_y;
			vs.position.x = xs;
			vs.normal = _lerp_normal(tri->v[0].normal, ph1.normal, t);
			vs.text_coord = _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, ph1.text_coord, ph1.position.w, t);

			//t = (loop_y - y0) / dyr;
			//ve.position = _lerp_position(tri->v[0].position, tri->v[1].position, t);
			ve.position = _lerp_position(tri->v[0].position, p11.position, t);
			ve.position.y = loop_y;
			ve.position.x = xe;
			ve.normal = _lerp_normal(tri->v[0].normal, p11.normal, t);
			ve.text_coord = _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, p11.text_coord, p11.position.w, t);

			if(scan_line(vs, ve))
				return;

			xs += dx_left_02;
			xe += dx_right_01;
		}

		dyl = y2 - 1 - ph.position.y;
		dyr = y2 - 1 - y1;
		xs = ph.position.x;
		xe = x1;
		for (loop_y = ph.position.y; loop_y < y2; ++loop_y)
		{
			if (RBMath::is_nearly_equal(dyl, 0.f))
			{
				t = 0;
			}
			else
			{
				t = (loop_y - ph.position.y) / dyl;
			}
			
			vs.position = _lerp_position(ph.position, p21.position, t);
			vs.position.y = loop_y;
			vs.position.x = xs;
			vs.normal = _lerp_normal(ph.normal, p21.normal, t);
			vs.text_coord = _lerp_uv(ph.text_coord, ph.position.w, p21.text_coord, p21.position.w, t);

			//t = (loop_y - ph.position.y) / dyr;
			//ve.position = _lerp_position(tri->v[0].position, tri->v[1].position, t);
			ve.position = _lerp_position(tri->v[1].position, p22.position, t);
			ve.position.y = loop_y;
			ve.position.x = xe;
			ve.normal = _lerp_normal(tri->v[1].normal, p22.normal, t);
			ve.text_coord = _lerp_uv(tri->v[1].text_coord, tri->v[1].position.w, p22.text_coord, p22.position.w, t);

			if(scan_line(vs, ve))
				return;

			xs += dx_left_02;
			xe += dx_right_12;
		}

		//RBlog("line\n");
	}
	else
	{
		//y中点在左边
		float x0 = tri->v[0].position.x;
		int y0 = ceil(tri->v[0].position.y - 0.5);
		float x1 = tri->v[1].position.x;
		int y1 = ceil(tri->v[1].position.y - 0.5);
		float x2 = tri->v[2].position.x;
		int y2 = ceil(tri->v[2].position.y - 0.5);

		tri->v[0].position.y = y0;
		tri->v[1].position.y = y1;
		tri->v[2].position.y = y2;

		//检测三角形是否退化为直线
		if (RBMath::is_nearly_equal(tri->v[0].position.y, tri->v[1].position.y) && RBMath::is_nearly_equal(tri->v[1].position.y, tri->v[2].position.y) ||
			RBMath::is_nearly_equal(tri->v[0].position.x, tri->v[1].position.x) && RBMath::is_nearly_equal(tri->v[1].position.x, tri->v[2].position.x))
			return;

		float dx_right_02 = (tri->v[2].position.x - tri->v[0].position.x) / (tri->v[2].position.y - tri->v[0].position.y);

		float dx_left_12 = (tri->v[2].position.x - tri->v[1].position.x) / (tri->v[2].position.y - tri->v[1].position.y);
		float dx_left_01 = (tri->v[1].position.x - tri->v[0].position.x) / (tri->v[1].position.y - tri->v[0].position.y);

		VertexP3N3T2 ph;
		VertexP3N3T2 ph1;
		VertexP3N3T2 p11;
		VertexP3N3T2 p21;
		VertexP3N3T2 p22;

		float tempt = (tri->v[1].position.y - tri->v[0].position.y) / (tri->v[2].position.y - tri->v[0].position.y);
		ph.position = _lerp_position(tri->v[0].position, tri->v[2].position, tempt);
		ph.position.y =  RBMath::round_f(ph.position.y );
		ph.normal = _lerp_normal(tri->v[0].normal, tri->v[2].normal, tempt);
		ph.text_coord = _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[2].text_coord, tri->v[2].position.w, tempt);

		tempt = (tri->v[1].position.y - 1 - tri->v[0].position.y) / (tri->v[2].position.y - tri->v[0].position.y);
		ph1.position = _lerp_position(tri->v[0].position, tri->v[2].position, tempt);
		ph1.position.y = RBMath::round_f(ph1.position.y);
		ph1.normal = _lerp_normal(tri->v[0].normal, tri->v[2].normal, tempt);
		ph1.text_coord = _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[2].text_coord, tri->v[2].position.w, tempt);

		tempt = (tri->v[1].position.y - 1 - tri->v[0].position.y) / (tri->v[1].position.y - tri->v[0].position.y);
		p11.position = _lerp_position(tri->v[0].position, tri->v[1].position, tempt);
		p11.position.y = RBMath::round_f(p11.position.y);
		p11.normal = _lerp_normal(tri->v[0].normal, tri->v[1].normal, tempt);
		p11.text_coord = _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[1].text_coord, tri->v[1].position.w, tempt);

		tempt = (tri->v[2].position.y - 1 - ph.position.y) / (y2 - ph.position.y);
		p21.position = _lerp_position(ph.position, tri->v[2].position, tempt);
		p21.position.y = RBMath::round_f(p21.position.y);
		p21.normal = _lerp_normal(ph.normal, tri->v[2].normal, tempt);
		p21.text_coord = _lerp_uv(ph.text_coord, ph.position.w, tri->v[2].text_coord, tri->v[2].position.w, tempt);

		p22.position = _lerp_position(tri->v[1].position, tri->v[2].position, tempt);
		p22.position.y = RBMath::round_f(p22.position.y);
		p22.normal = _lerp_normal(tri->v[1].normal, tri->v[2].normal, tempt);
		p22.text_coord = _lerp_uv(tri->v[1].text_coord, tri->v[1].position.w, tri->v[2].text_coord, tri->v[2].position.w, tempt);

		VertexP3N3T2 vs;
		VertexP3N3T2 ve;

		float t = 0.f;
		int loop_y = 0;
		float dyl = ph1.position.y - y0;
		float dyr = p11.position.y - y0;
		float xs = x0;
		float xe = x0;
		for (loop_y = y0; loop_y <= ph1.position.y; ++loop_y)
		{


			if (RBMath::is_nearly_equal(dyl, 0.f))
			{
				t = 0;
			}
			else
			{
				t = (loop_y - y0) / dyl;
			}
			
			vs.position = _lerp_position(tri->v[0].position, ph1.position, t);
			vs.position.y = loop_y;
			if (abs(xs - vs.position.x)>0.001)
			{
				xs += dx_right_02;
				xe += dx_left_01;
				continue;
			}
			vs.position.x = xs;
			vs.normal = _lerp_normal(tri->v[0].normal, ph1.normal, t);
			vs.text_coord = _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, ph1.text_coord, ph1.position.w, t);

			//t = (loop_y - y0) / dyr;
			//ve.position = _lerp_position(tri->v[0].position, tri->v[1].position, t);
			ve.position = _lerp_position(tri->v[0].position, p11.position, t);
			ve.position.y = loop_y;

			ve.position.x = xe;

			ve.normal = _lerp_normal(tri->v[0].normal, p11.normal, t);
			ve.text_coord = _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, p11.text_coord, p11.position.w, t);

			if(scan_line(ve, vs))
				return;

			xs += dx_right_02;
			xe += dx_left_01;
		}

		dyl = y2 - 1 - ph.position.y;
		dyr = y2 - 1 - y1;
		xs = ph.position.x;
		xe = x1;
		for (loop_y = ph.position.y; loop_y < y2; ++loop_y)
		{
			if (RBMath::is_nearly_equal(dyl, 0.f))
			{
				t = 0;
			}
			else
			{
				t = (loop_y - ph.position.y) / dyl;
			}
			vs.position = _lerp_position(ph.position, p21.position, t);
			vs.position.y = loop_y;
			vs.position.x = xs;
			vs.normal = _lerp_normal(ph.normal, p21.normal, t);
			vs.text_coord = _lerp_uv(ph.text_coord, ph.position.w, p21.text_coord, p21.position.w, t);

			//t = (loop_y - ph.position.y) / dyr;
			//ve.position = _lerp_position(tri->v[0].position, tri->v[1].position, t);
			ve.position = _lerp_position(tri->v[1].position, p22.position, t);
			ve.position.y = loop_y;
			ve.position.x = xe;
			ve.normal = _lerp_normal(tri->v[1].normal, p22.normal, t);
			ve.text_coord = _lerp_uv(tri->v[1].text_coord, tri->v[1].position.w, p22.text_coord, p22.position.w, t);

			if (scan_line(ve, vs))
				return;

			xs += dx_right_02;
			xe += dx_left_12;
		}
		//RBlog("line\n");
	}
	
	//_triangles_fragments.push_back(frg);

}


bool SrRasterizer::scan_line(VertexP3N3T2& sv, VertexP3N3T2& ev)
{
	sv.position.x = ceil(sv.position.x - 0.5);
	ev.position.x = ceil(ev.position.x - 0.5);
	//RBlog("line\n");
	if (!RBMath::is_nearly_equal(ev.position.y, sv.position.y))
		return false;

	float t = 0;
	float dx = ev.position.x - sv.position.x + 1;
	int loop_x;
	for (loop_x = 0; loop_x < ev.position.x - sv.position.x; ++loop_x)
	{
		t = loop_x / dx;

		VertexP3N3T2 v;
		//插值消耗了一半的时间 14 total
		//1~2
		v.position = _lerp_position(sv.position, ev.position, t);
		//2~3
		v.position.x = sv.position.x + loop_x;
		v.position.y = int(sv.position.y - 0.5);
		//~2
		v.normal = _lerp_normal(sv.normal, ev.normal, t);
		//~2
		v.text_coord = _lerp_uv(sv.text_coord, sv.position.w, ev.text_coord, ev.position.w, t);		
		
		
		//~7
		//_stage_ps->proccess(v);
		//~3
		//_stage_om->proccess(v, *_color_buffer, *_depth_buffer);
		

		//如果帧率太低直接跳过不处理

		//
		_total_frag++;

		
#define OP_
#ifdef OP
		if (_last_ts_time < 120)
		{
			_gpu->write_min(v);
			continue;
		}
		else
		{
			return true;
		}
#else
		_gpu->write_min(v);
#endif

		/*
		int s0 = _gpu->gauss_size(0);
		int s1 = _gpu->gauss_size(1);
		int s2 = _gpu->gauss_size(2);
		int s3 = _gpu->gauss_size(3);
		int index = s0 > s1 ? 1 : 0;
		int index1 = s2 > s3 ? 3 : 2;
		index = _gpu->gauss_size(index) > _gpu->gauss_size(index1) ? index1 : index;
		_gpu->write(index, v);
		*/
	}
	return false;
}

void SrRasterizer::scan_line(VertexP3N3T2& sv, VertexP3N3T2& ev, SrFragment* _triangle_fragment)
{
	//char a[64];
	//sprintf(a, "(%f %f)\n", sv.position.x, ev.position.x);
	//RBlog(a);

	sv.position.x = ceil(sv.position.x - 0.5);
	ev.position.x = ceil(ev.position.x - 0.5);
	


	
	//RBlog("line\n");
	if (!RBMath::is_nearly_equal(ev.position.y, sv.position.y))
		return;

	float t = 0;
	float dx = ev.position.x - sv.position.x + 1;
	int loop_x;
	for (loop_x = 0; loop_x < ev.position.x - sv.position.x; ++loop_x)
	{
		t = loop_x / dx;

		VertexP3N3T2 v;
		v.position = _lerp_position(sv.position, ev.position, t);
		v.position.x = sv.position.x + loop_x;
		v.position.y = int(sv.position.y - 0.5);
		v.normal = _lerp_normal(sv.normal, ev.normal, t);
		v.text_coord = _lerp_uv(sv.text_coord, sv.position.w, ev.text_coord, ev.position.w, t);

		_triangle_fragment->frag.push_back(v);

	}

}

RBVector4 SrRasterizer::_lerp_position(RBVector4 start, RBVector4 end, float t)
{
	RBVector4 v = (1 - t)*start + t*end;
	v.w = start.w * end.w / ((1 - t)*end.w + start.w*t);
	//v.w = start.w;
	return v;
}

RBVector3 SrRasterizer::_lerp_normal(RBVector3 start, RBVector3 end, float t)
{
	return start + t*(end - start);
}

RBVector2 SrRasterizer::_lerp_uv(RBVector2 start, float sz, RBVector2 end, float ez, float t)
{
	float factor = 1/(sz*(t) + (1-t)*ez);
	RBVector2 v = (ez*start*(1-t) + t*sz*end)*factor;
	//v = (1 - t)* start + t* end;
	return v;
}

