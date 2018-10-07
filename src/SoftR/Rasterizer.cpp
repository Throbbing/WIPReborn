#include "Rasterizer.h"
#include "../RBMath/Inc/RBMathBase.h"
#include "../RBMath/Inc/AABB.h"
#include <math.h>
#include "..\\Uitilities.h"
#include "SimGPU.h"
#include "../Input.h"

SrRasterizer::SrRasterizer()
{
	_prof.init();

	_viewport_h = _viewport_w = 0;
	_stage_ps = new SrStagePS();
	_stage_om = new SrStageOM();
	_total_frag = 0;

	sss = false;
}

SrRasterizer::SrRasterizer(SrSimGPU * gpu)
{
	_prof.init();

	_viewport_h = _viewport_w = 0;
	_stage_ps = new SrStagePS();
	_stage_om = new SrStageOM();

	gpu->set_stage_ps(_stage_ps);
	gpu->set_stage_om(_stage_om);
	gpu->set_raster(this);

	_gpu = gpu;
	_total_frag = 0;

	sss = false;
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


int SrRasterizer::near_far_cull(SrTriangle* _triangles, SrTriangle* _triangles_near_far_cull, int n)
{
	int ret = 0;

	//it is clip-space before perspective dividing(/w)
	//clip triangles in clip-space is the most convenient way
	//because you must cull and clip the triangles whose some one vectex is <=0
	//and get a new vertex which is on the plane z=0
	//but there is some issue:
	//必须在 /w 之前的四维空间下进行裁剪，保证所有顶点的 w > 0，后面归一化 / w 时也才不会除零错误。
	//在clip空间进行裁剪的时候，要把所有顶点属性全部插值得到新的裁剪顶点，这个顶点因为在裁剪平面上
	//依然存在透视除法除以0的情况，一种做法是把插值参数w的值稍微往前偏移一点点，防止除以0
	//但是还有一个坑，浮点数误差，w = E，近似零的值了，要取的比较小，但又不能太小，
	//要搞到接近单精度浮点的 1.4E-45 之类的单精度极限值的话，归一化 /w 时，P3其他几个值就都他妈失真，接近无穷大了
	//所以处理起来略麻烦，早年的处理方法是只要有一个顶点在摄像机后面，就整个三角形全部不要，但是这对于大三角形会出现问题
	//另外也可以在近面裁剪之后，做透视除法，在ndc空间去做其他面的裁剪
	//但在线代硬件上不是这样，硬件一般只cull不裁剪，因为生成新的三角形会增加传递带宽，
	//而在硬件上带宽有限而在三角形setup的时候做fragment级别的提出却很快
	//但软件实现上再fragment上做剔除却并不是一个好主意，因为setup至少要插值位置，而软件实现的瓶颈有一部分就在于大量的fragment
	//注意，w是视口z，而z是透视z，两者原点不一样的
	//
	//xyz除以w，但是保留w为z
	for (int i = 0; i < n; ++i)
	{
		auto* tri = &(_triangles[i]);
		//note:divide w need abs in case -*-=+

		//w<=0的全部提出，跨越w=0的截断
		//-w<=x,y<=w
		//0<z<=w

		float x1 = tri->v[0].position.x;
		float y1 = tri->v[0].position.y;
		float z1 = tri->v[0].position.z;


		float x2 = tri->v[1].position.x;
		float y2 = tri->v[1].position.y;
		float z2 = tri->v[1].position.z;


		float x3 = tri->v[2].position.x;
		float y3 = tri->v[2].position.y;
		float z3 = tri->v[2].position.z;

		float fz1 = tri->v[0].position.w;
		float fz2 = tri->v[1].position.w;
		float fz3 = tri->v[2].position.w;

		
#ifdef USE_CLIP_NEAR_TRI

#else
		//if (fz1 <= 0 || fz2 <= 0 || fz3 <= 0)
			//continue;
		//out of far panel and near panel
		if ((z1 > fz1&&z2 > fz2&&z3 > fz3) || (z1 <= 0 && z2 <= 0 && z3 <= 0))
		{
			continue;
		}


		if ((x1 <= -fz1 && x2 <= -fz2 && x3 <= -fz3) || (x1 >= fz1 && x2 >= fz2 && x3 >= fz3))
		{
			continue;
		}

		if ((y1 <= -fz1 && y2 <= -fz2 && y3 <= -fz3) || (y1 >= fz1 && y2 >= fz2 && y3 >= fz3))
		{
			continue;
		}

		if ((x1 > -fz1 && x2 > -fz2 && x3 > -fz3) && (x1 < fz1 && x2 < fz2 && x3 < fz3) && (y1 > -fz1 && y2 > -fz2 && y3 > -fz3) && (y1 < fz1 && y2 < fz2 && y3 < fz3))
		{
			if (z1 >= 0 && z2 >= 0 && z3 >= 0)
			{
				_triangles_near_far_cull[ret++] = (*tri);
				continue;
			}
		}

		if (z1 > 0 && z2 > 0 && z3 > 0)
		{
			_triangles_near_far_cull[ret++] = (*tri);
			continue;
		}
		//if (fz1 == 0) tri->v[0].position.w = 0.0001f;
		//if (fz2 == 0) tri->v[1].position.w = 0.0001f;
		//if (fz3 == 0) tri->v[2].position.w = 0.0001f;

#if 1
		//处理所有近裁剪面相交的三角形,保绕序
		//注意！！！此处的z与w是成比例的，此处没有除以w，不需要透视插值
		if (z1 < 0 && z2>0 && z3 > 0)
		{
			//continue;
			float t = -z1 / (z2 - z1);
			VertexP3N3T2 newv1;
			 _lerp_position_nopersp(tri->v[0].position, tri->v[1].position, t, newv1.position);
			newv1.position.z = 0.f;
			//newv1.position.w += 0.001f;
			 _lerp_normal(tri->v[0].normal, tri->v[1].normal, t, newv1.normal);
			 _lerp_uv_nopersp(tri->v[0].text_coord, tri->v[1].text_coord, t, newv1.text_coord);


			t = -z1 / (z3 - z1);
			VertexP3N3T2 newv2;
			 _lerp_position_nopersp(tri->v[0].position, tri->v[2].position, t, newv2.position);
			newv2.position.z = 0.f;

			//newv2.position.w += 0.001f;
			 _lerp_normal(tri->v[0].normal, tri->v[2].normal, t, newv2.normal);
			 _lerp_uv_nopersp(tri->v[0].text_coord, tri->v[2].text_coord, t, newv2.text_coord);
			SrTriangle addtri = SrTriangle(newv1, tri->v[2], newv2);
			_triangles_near_far_cull[ret++] = (addtri);

			tri->v[0] = newv1;

			_triangles_near_far_cull[ret++] = (*tri);
		}

		
		if (z1 > 0 && z2 < 0 && z3>0)
		{
			//continue;
			float t = -z2 / (z3 - z2);
			VertexP3N3T2 newv1;
			 _lerp_position_nopersp(tri->v[1].position, tri->v[2].position, t, newv1.position);
			newv1.position.z = 0.f;
			 _lerp_normal(tri->v[1].normal, tri->v[2].normal, t, newv1.normal);
			 _lerp_uv_nopersp(tri->v[1].text_coord, tri->v[2].text_coord, t, newv1.text_coord);



			t = -z2 / (z1 - z2);
			VertexP3N3T2 newv2;
			 _lerp_position_nopersp(tri->v[1].position, tri->v[0].position, t, newv2.position);
			newv2.position.z = 0.f;
			 _lerp_normal(tri->v[1].normal, tri->v[0].normal, t, newv2.normal);
			 _lerp_uv_nopersp(tri->v[1].text_coord, tri->v[0].text_coord, t, newv2.text_coord);
			SrTriangle addtri = SrTriangle(newv1, tri->v[0], newv2);
			_triangles_near_far_cull[ret++] = (addtri);


			tri->v[1] = newv1;

			_triangles_near_far_cull[ret++] = (*tri);

		}
		if (z1 > 0 && z2 > 0 && z3 < 0)
		{
			//continue;
			float t = -z3 / (z1 - z3);
			VertexP3N3T2 newv1;
			 _lerp_position_nopersp(tri->v[2].position, tri->v[0].position, t, newv1.position);
			newv1.position.z = 0.f;
			 _lerp_normal(tri->v[2].normal, tri->v[0].normal, t, newv1.normal);
			 _lerp_uv_nopersp(tri->v[2].text_coord, tri->v[0].text_coord, t, newv1.text_coord);

			t = -z3 / (z2 - z3);
			VertexP3N3T2 newv2;
			 _lerp_position_nopersp(tri->v[2].position, tri->v[1].position, t, newv2.position);
			newv2.position.z = 0.f;
			 _lerp_normal(tri->v[2].normal, tri->v[1].normal, t, newv2.normal);
			 _lerp_uv_nopersp(tri->v[2].text_coord, tri->v[1].text_coord, t, newv2.text_coord);
			SrTriangle addtri = SrTriangle(newv1, tri->v[1], newv2);
			_triangles_near_far_cull[ret++] = (addtri);

			tri->v[2] = newv1;

			_triangles_near_far_cull[ret++] = (*tri);
		}

		//continue;
		if (z1 > 0 && z2 < 0 && z3 < 0)
		{
			//continue;
			float t = -z3 / (z1 - z3);
			VertexP3N3T2 newv1;
			 _lerp_position_nopersp(tri->v[2].position, tri->v[0].position, t, newv1.position);
			newv1.position.z = 0.f;
			 _lerp_normal(tri->v[2].normal, tri->v[0].normal, t, newv1.normal);
			 _lerp_uv_nopersp(tri->v[2].text_coord, tri->v[0].text_coord, t, newv1.text_coord);

			t = -z2 / (z1 - z2);
			VertexP3N3T2 newv2;
			 _lerp_position_nopersp(tri->v[1].position, tri->v[0].position, t, newv2.position);
			newv2.position.z = 0.f;
			 _lerp_normal(tri->v[1].normal, tri->v[0].normal, t, newv2.normal);
			 _lerp_uv_nopersp(tri->v[1].text_coord, tri->v[0].text_coord, t, newv2.text_coord);
			tri->v[2] = newv1;
			tri->v[1] = newv2;

			_triangles_near_far_cull[ret++] = (*tri);
		}
		if (z1 < 0 && z2 < 0 && z3 > 0)
		{
			//continue;
			float t = -z2 / (z3 - z2);
			VertexP3N3T2 newv1;
			 _lerp_position_nopersp(tri->v[1].position, tri->v[2].position, t, newv1.position);
			newv1.position.z = 0.f;
			 _lerp_normal(tri->v[1].normal, tri->v[2].normal, t, newv1.normal);
			 _lerp_uv_nopersp(tri->v[1].text_coord, tri->v[2].text_coord, t, newv1.text_coord);

			t = -z1 / (z3 - z1);
			VertexP3N3T2 newv2;
			 _lerp_position_nopersp(tri->v[0].position, tri->v[2].position, t, newv2.position);
			newv2.position.z = 0.f;
			 _lerp_normal(tri->v[0].normal, tri->v[2].normal, t, newv2.normal);
			 _lerp_uv_nopersp(tri->v[0].text_coord, tri->v[2].text_coord, t, newv2.text_coord);
			tri->v[1] = newv1;
			tri->v[0] = newv2;

			_triangles_near_far_cull[ret++] = (*tri);
		}
		if (z1 < 0 && z2 > 0 && z3 < 0)
		{
			//continue;
			float t = -z1 / (z2 - z1);
			t += 0.001f;
			VertexP3N3T2 newv1;
			 _lerp_position_nopersp(tri->v[0].position, tri->v[1].position, t, newv1.position);
			newv1.position.z = 0.f;
			 _lerp_normal(tri->v[0].normal, tri->v[1].normal, t, newv1.normal);
			 _lerp_uv_nopersp(tri->v[0].text_coord, tri->v[1].text_coord, t, newv1.text_coord);

			t = -z3 / (z2 - z3);
			t += 0.001f;
			VertexP3N3T2 newv2;
			 _lerp_position_nopersp(tri->v[2].position, tri->v[1].position, t, newv2.position);
			newv2.position.z = 0.f;
			 _lerp_normal(tri->v[2].normal, tri->v[1].normal, t, newv2.normal);
			 _lerp_uv_nopersp(tri->v[2].text_coord, tri->v[1].text_coord, t, newv2.text_coord);
			tri->v[0] = newv1;
			tri->v[2] = newv2;


			_triangles_near_far_cull[ret++] = (*tri);

		}
#endif // 0




#endif
	}
	return ret;
}


int SrRasterizer::back_cull(SrTriangle* _triangles, SrTriangle* _triangles_back_cull, int n)
{
	int ret = 0;
	for (int i = 0; i < n; ++i)
	{

		auto* tri = &_triangles[i];
		if (tri->v[0].position.w <= 0 || tri->v[1].position.w <= 0 || tri->v[2].position.w <= 0)
			printf("fkoff\n");
		float inv_w = RBMath::abs(1.f / (tri->v[0].position.w));
		if (inv_w == 0)
			printf("fp\n");
		tri->v[0].position.x *= inv_w;
		tri->v[0].position.y *= inv_w;
		tri->v[0].position.z *= inv_w;

		inv_w = RBMath::abs(1.f / (tri->v[1].position.w));
		if (inv_w == 0)
			printf("fp\n");
		tri->v[1].position.x *= inv_w;
		tri->v[1].position.y *= inv_w;
		tri->v[1].position.z *= inv_w;

		inv_w = RBMath::abs(1.f / (tri->v[2].position.w));
		if (inv_w == 0)
			printf("fp\n");
		tri->v[2].position.x *= inv_w;
		tri->v[2].position.y *= inv_w;
		tri->v[2].position.z *= inv_w;
		RBVector4 v1 = tri->v[1].position - tri->v[0].position;
		RBVector4 v2 = tri->v[2].position - tri->v[0].position;
		RBVector4 cr = v1 ^ v2;
		RBVector4 z(0, 0, 1, 1);
		float res = RBVector4::dot3(cr, z);
		if (res < 0)
		{
			
			_triangles_back_cull[ret++] = (*tri);
		}

	}
	return ret;
}

int SrRasterizer::clip_besidenp(SrTriangle* _triangles, SrTriangle* _triangles_clip, int n)
{
	int res = 0;
	for (int i = 0; i < n; ++i)
	{
		//裁剪
		_triangles_clip[res++] = (_triangles[i]);
	}
	return res;
}

void SrRasterizer::trangle_setup_gpu_tiled(SrTriangle * tri, const RBAABB2D& quad, bool wireframe)
{
	

	if (wireframe)
	{
		auto draw_line = [&](const VertexP3N3T2& a, const VertexP3N3T2& b)->void
		{
			int x1 = a.position.x; int y1 = a.position.y;
			int x2 = b.position.x; int y2 = b.position.y;
			//参数c为颜色值
			int dx = abs(x2 - x1), dy = abs(y2 - y1), yy = 0;
			if (dx < dy)
			{
				yy = 1;
				int temp = x1;
				x1 = y1;
				y1 = temp;
				temp = x2;
				x2 = y2;
				y2 = temp;
				temp = dx;
				dx = dy;
				dy = temp;
			}
			int ix = (x2 - x1) > 0 ? 1 : -1, iy = (y2 - y1) > 0 ? 1 : -1, cx = x1, cy = y1, n2dy = dy * 2, n2dydx = (dy - dx) * 2, d = dy * 2 - dx;
			int i = 0;
			//如果直线与x轴的夹角大于45度  
			if (yy)
			{
				while (cx != x2)
				{
					if (d < 0)
					{
						d += n2dy;
					}
					else
					{
						cy += iy;
						d += n2dydx;
					}
					VertexP3N3T2 v;
					v.position.z = (a.position.z + b.position.z)*0.5f;
					v.position.x = cy;
					v.position.y = cx;
					v.normal = a.normal;
					v.position.x = v.position.x + _viewport_w *0.5f;
					v.position.y = v.position.y + _viewport_h *0.5f;
					if (_discard_invisible_tiled(v, quad))
					{
						_stage_ps->proccess(v);
						_stage_om->proccess(v, (_color_buffer),_main_buffer, *(_depth_buffer));
					}


					cx += ix;
					++i;
				}
			}
			//如果直线与x轴的夹角小于45度 
			else
			{
				while (cx != x2)
				{
					if (d < 0)
					{
						d += n2dy;
					}
					else
					{
						cy += iy;
						d += n2dydx;
					}
					VertexP3N3T2 v;
					v.position.z = (a.position.z + b.position.z)*0.5f;
					v.position.x = cx;
					v.position.y = cy;
					v.normal = a.normal;
					v.position.x = v.position.x + _viewport_w *0.5f;
					v.position.y = v.position.y + _viewport_h *0.5f;
					if (_discard_invisible_tiled(v, quad))
					{
						_stage_ps->proccess(v);
						_stage_om->proccess(v, (_color_buffer), _main_buffer, *(_depth_buffer));
					}

					cx += ix;
					++i;
				}
			}
		};

		draw_line(tri->v[0], tri->v[1]);
		draw_line(tri->v[1], tri->v[2]);
		draw_line(tri->v[2], tri->v[0]);


#ifdef DRAWVERTEX
		for (int i = 0; i < 3; ++i)
		{
			VertexP3N3T2 &vr = tri->v[i];
			vr.position.x = vr.position.x + _viewport_w *0.5f;
			vr.position.y = vr.position.y + _viewport_h *0.5f;
			VertexP3N3T2 v = vr;
			
			v.position.x += 1;
			if (_discard_invisible_tiled(v, quad))
			{
				_stage_ps->proccess(v);
				_stage_om->proccess(v, *(_color_buffer), *(_depth_buffer));
			}

			v = vr;
			v.position.x -= 1;
			if (_discard_invisible_tiled(v, quad))
			{
				_stage_ps->proccess(v);
				_stage_om->proccess(v, *(_color_buffer), *(_depth_buffer));
			}

			v = vr;
			v.position.y += 1;
			if (_discard_invisible_tiled(v, quad))
			{
				_stage_ps->proccess(v);
				_stage_om->proccess(v, *(_color_buffer), *(_depth_buffer));
			}

			v = vr;
			v.position.y -= 1;
			if (_discard_invisible_tiled(v, quad))
			{
				_stage_ps->proccess(v);
				_stage_om->proccess(v, *(_color_buffer), *(_depth_buffer));
			}

			v = vr;
			v.position.x += 1;
			v.position.y += 1;
			if (_discard_invisible_tiled(v, quad))
			{
				_stage_ps->proccess(v);
				_stage_om->proccess(v, *(_color_buffer), *(_depth_buffer));
			}



			v = vr;
			v.position.x += -1;
			v.position.y += -1;
			if (_discard_invisible_tiled(v, quad))
			{
				_stage_ps->proccess(v);
				_stage_om->proccess(v, *(_color_buffer), *(_depth_buffer));
			}

			v = vr;
			v.position.x += -1;
			v.position.y += 1;
			if (_discard_invisible_tiled(v, quad))
			{
				_stage_ps->proccess(v);
				_stage_om->proccess(v, *(_color_buffer), *(_depth_buffer));
			}

			v = vr;
			v.position.x += 1;
			v.position.y += -1;
			if (_discard_invisible_tiled(v, quad))
			{
				_stage_ps->proccess(v);
				_stage_om->proccess(v, *(_color_buffer), *(_depth_buffer));
			}

			if (_discard_invisible_tiled(vr, quad))
			{
				_stage_ps->proccess(vr);
				_stage_om->proccess(vr, *(_color_buffer), *(_depth_buffer));
			}

		}
#endif
	}
	else
	{
		/*
		//把坐标转换到视口
		tri->v[0].position.x = tri->v[0].position.x*0.5*_viewport_w;
		tri->v[0].position.y = tri->v[0].position.y*0.5*_viewport_h;
		tri->v[1].position.x = tri->v[1].position.x*0.5*_viewport_w;
		tri->v[1].position.y = tri->v[1].position.y*0.5*_viewport_h;
		tri->v[2].position.x = tri->v[2].position.x*0.5*_viewport_w;
		tri->v[2].position.y = tri->v[2].position.y*0.5*_viewport_h;
		*/
		//检测三角形是否退化为直线

		if (RBMath::is_nearly_equal(tri->v[0].position.y, tri->v[1].position.y) && RBMath::is_nearly_equal(tri->v[1].position.y, tri->v[2].position.y) ||
			RBMath::is_nearly_equal(tri->v[0].position.x, tri->v[1].position.x) && RBMath::is_nearly_equal(tri->v[1].position.x, tri->v[2].position.x))
			return;
		_sort_y(tri);
		_new_set_tri2_tiled(tri, quad);
		//new_set_tri2(tri, _stage_ps, _stage_om);
	}
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


}

void SrRasterizer::_new_set_tri2_tiled(SrTriangle* tri, const RBAABB2D& quad)
{
	//!!!性能issue
	//SrFragment* frg = new SrFragment();
	//用于判断左右三角
	float tc = (tri->v[1].position.y - tri->v[0].position.y) / (tri->v[2].position.y - tri->v[0].position.y);
	float xc = tri->v[0].position.x*(1 - tc) + tri->v[2].position.x*tc;

	if (tri->v[1].position.x == xc)
		printf("");
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
		if (y0 > y1 || y0 > y2 || y1 > y2)
			printf("");

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
		 _lerp_position(tri->v[0].position, tri->v[2].position, tempt, ph.position);
		ph.position.y = RBMath::round_f(ph.position.y);
		 _lerp_normal(tri->v[0].normal, tri->v[2].normal, tempt, ph.normal);
		 _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[2].text_coord, tri->v[2].position.w, tempt, ph.text_coord);
		if (ph.position.y < -100000)
		{
			if (RBMath::is_nearly_equal(tri->v[0].position.y, tri->v[1].position.y) && RBMath::is_nearly_equal(tri->v[1].position.y, tri->v[2].position.y) ||
				RBMath::is_nearly_equal(tri->v[0].position.x, tri->v[1].position.x) && RBMath::is_nearly_equal(tri->v[1].position.x, tri->v[2].position.x))
				return;
			printf("");
		}
		tempt = (tri->v[1].position.y - 1 - tri->v[0].position.y) / (tri->v[2].position.y - tri->v[0].position.y);
		 _lerp_position(tri->v[0].position, tri->v[2].position, tempt, ph1.position);
		ph1.position.y = RBMath::round_f(ph1.position.y);
		 _lerp_normal(tri->v[0].normal, tri->v[2].normal, tempt, ph1.normal);
		 _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[2].text_coord, tri->v[2].position.w, tempt, ph1.text_coord);

		tempt = (tri->v[1].position.y - 1 - tri->v[0].position.y) / (tri->v[1].position.y - tri->v[0].position.y);
		 _lerp_position(tri->v[0].position, tri->v[1].position, tempt, p11.position);
		p11.position.y = RBMath::round_f(p11.position.y);
		 _lerp_normal(tri->v[0].normal, tri->v[1].normal, tempt, p11.normal);
		 _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[1].text_coord, tri->v[1].position.w, tempt, p11.text_coord);

		tempt = (tri->v[2].position.y - 1 - ph.position.y) / (y2 - ph.position.y);
		 _lerp_position(ph.position, tri->v[2].position, tempt, p21.position);
		p21.position.y = RBMath::round_f(p21.position.y);
		 _lerp_normal(ph.normal, tri->v[2].normal, tempt, p21.normal);
		 _lerp_uv(ph.text_coord, ph.position.w, tri->v[2].text_coord, tri->v[2].position.w, tempt, p21.text_coord);

		 _lerp_position(tri->v[1].position, tri->v[2].position, tempt, p22.position);
		p22.position.y = RBMath::round_f(p22.position.y);
		 _lerp_normal(tri->v[1].normal, tri->v[2].normal, tempt, p22.normal);
		 _lerp_uv(tri->v[1].text_coord, tri->v[1].position.w, tri->v[2].text_coord, tri->v[2].position.w, tempt, p22.text_coord);

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

			 _lerp_position(tri->v[0].position, ph1.position, t, vs.position);
			vs.position.y = loop_y;
			vs.position.x = xs;
			 _lerp_normal(tri->v[0].normal, ph1.normal, t, vs.normal);
			 _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, ph1.text_coord, ph1.position.w, t, vs.text_coord);

			//t = (loop_y - y0) / dyr;
			//ve.position _lerp_position(tri->v[0].position, tri->v[1].position, t);
			 _lerp_position(tri->v[0].position, p11.position, t, ve.position);
			ve.position.y = loop_y;
			ve.position.x = xe;
			 _lerp_normal(tri->v[0].normal, p11.normal, t, ve.normal);
			 _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, p11.text_coord, p11.position.w, t, ve.text_coord);

			if (scan_line_tiled(vs, ve, quad))
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

			 _lerp_position(ph.position, p21.position, t, vs.position);
			vs.position.y = loop_y;
			vs.position.x = xs;
			 _lerp_normal(ph.normal, p21.normal, t, vs.normal);
			 _lerp_uv(ph.text_coord, ph.position.w, p21.text_coord, p21.position.w, t, vs.text_coord);

			//t = (loop_y - ph.position.y) / dyr;
			//ve.position _lerp_position(tri->v[0].position, tri->v[1].position, t);
			 _lerp_position(tri->v[1].position, p22.position, t, ve.position);
			ve.position.y = loop_y;
			ve.position.x = xe;
			 _lerp_normal(tri->v[1].normal, p22.normal, t, ve.normal);
			 _lerp_uv(tri->v[1].text_coord, tri->v[1].position.w, p22.text_coord, p22.position.w, t, ve.text_coord);

			if (scan_line_tiled(vs, ve, quad))
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
			RBMath::is_nearly_equal(tri->v[0].position.x, tri->v[1].position.x) && RBMath::is_nearly_equal(tri->v[1].position.x, tri->v[2].position.x)
			)
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
		 _lerp_position(tri->v[0].position, tri->v[2].position, tempt, ph.position);
		ph.position.y = RBMath::round_f(ph.position.y);
		 _lerp_normal(tri->v[0].normal, tri->v[2].normal, tempt, ph.normal);
		 _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[2].text_coord, tri->v[2].position.w, tempt, ph.text_coord);

		tempt = (tri->v[1].position.y - 1 - tri->v[0].position.y) / (tri->v[2].position.y - tri->v[0].position.y);
		 _lerp_position(tri->v[0].position, tri->v[2].position, tempt, ph1.position);
		ph1.position.y = RBMath::round_f(ph1.position.y);
		 _lerp_normal(tri->v[0].normal, tri->v[2].normal, tempt, ph1.normal);
		 _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[2].text_coord, tri->v[2].position.w, tempt, ph1.text_coord);

		tempt = (tri->v[1].position.y - 1 - tri->v[0].position.y) / (tri->v[1].position.y - tri->v[0].position.y);
		 _lerp_position(tri->v[0].position, tri->v[1].position, tempt, p11.position);
		p11.position.y = RBMath::round_f(p11.position.y);
		 _lerp_normal(tri->v[0].normal, tri->v[1].normal, tempt, p11.normal);
		 _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, tri->v[1].text_coord, tri->v[1].position.w, tempt, p11.text_coord);

		tempt = (tri->v[2].position.y - 1 - ph.position.y) / (y2 - ph.position.y);
		 _lerp_position(ph.position, tri->v[2].position, tempt, p21.position);
		p21.position.y = RBMath::round_f(p21.position.y);
		 _lerp_normal(ph.normal, tri->v[2].normal, tempt, p21.normal);
		 _lerp_uv(ph.text_coord, ph.position.w, tri->v[2].text_coord, tri->v[2].position.w, tempt, p21.text_coord);

		 _lerp_position(tri->v[1].position, tri->v[2].position, tempt, p22.position);
		p22.position.y = RBMath::round_f(p22.position.y);
		 _lerp_normal(tri->v[1].normal, tri->v[2].normal, tempt, p22.normal);
		 _lerp_uv(tri->v[1].text_coord, tri->v[1].position.w, tri->v[2].text_coord, tri->v[2].position.w, tempt, p22.text_coord);

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

			_lerp_position(tri->v[0].position, ph1.position, t, vs.position);
			vs.position.y = loop_y;
			/*
			if (abs(xs - vs.position.x)<0.001)
			{
			xs += dx_right_02;
			xe += dx_left_01;
			continue;
			}
			*/
			vs.position.x = xs;
			_lerp_normal(tri->v[0].normal, ph1.normal, t,vs.normal);
			_lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, ph1.text_coord, ph1.position.w, t,vs.text_coord);

			//t = (loop_y - y0) / dyr;
			//ve.position = _lerp_position(tri->v[0].position, tri->v[1].position, t);
			_lerp_position(tri->v[0].position, p11.position, t,ve.position);
			ve.position.y = loop_y;

			ve.position.x = xe;

			 _lerp_normal(tri->v[0].normal, p11.normal, t, ve.normal);
			 _lerp_uv(tri->v[0].text_coord, tri->v[0].position.w, p11.text_coord, p11.position.w, t, ve.text_coord);

			if (scan_line_tiled(ve, vs, quad))
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
			_lerp_position(ph.position, p21.position, t,vs.position);
			vs.position.y = loop_y;
			vs.position.x = xs;
			_lerp_normal(ph.normal, p21.normal, t, vs.normal);
			_lerp_uv(ph.text_coord, ph.position.w, p21.text_coord, p21.position.w, t, vs.text_coord);

			//t = (loop_y - ph.position.y) / dyr;
			//ve.position = _lerp_position(tri->v[0].position, tri->v[1].position, t);
			_lerp_position(tri->v[1].position, p22.position, t, ve.position);
			ve.position.y = loop_y;
			ve.position.x = xe;
			_lerp_normal(tri->v[1].normal, p22.normal, t, ve.normal);
			_lerp_uv(tri->v[1].text_coord, tri->v[1].position.w, p22.text_coord, p22.position.w, t, ve.text_coord);

			if (scan_line_tiled(ve, vs, quad))
				return;

			xs += dx_right_02;
			xe += dx_left_12;
		}
		//RBlog("line\n");
	}

	//_triangles_fragments.push_back(frg);

}



bool SrRasterizer::scan_line_tiled(VertexP3N3T2& sv, VertexP3N3T2& ev, const RBAABB2D& quad)
{
	sv.position.x = ceil(sv.position.x - 0.5);
	ev.position.x = ceil(ev.position.x - 0.5);

	if (!RBMath::is_nearly_equal(ev.position.y, sv.position.y))
		return false;

	float t = 0;
	float inv_dx = 1.f/(ev.position.x - sv.position.x + 1);
	int loop_x;
	int xc = ev.position.x - sv.position.x;
	for (loop_x = 0; loop_x < xc; ++loop_x)
	{
		t = loop_x * inv_dx;

		VertexP3N3T2 v;
		//1~2
		//v.position = _lerp_position(sv.position, ev.position, t);
		//2~3
		v.position.x = sv.position.x + loop_x;
		v.position.y = int(sv.position.y - 0.5);

		v.position.x = v.position.x + _viewport_w *0.5f;
		v.position.y = v.position.y + _viewport_h *0.5f;
		//issue:intersect with quad shoud only once! still use left-top rule!
		if (_discard_invisible_tiled(v, quad))
		{
			_lerp_position_onlyzw(sv.position, ev.position, t, v.position);
			//~2
			_lerp_normal(sv.normal, ev.normal, t, v.normal);
			//~2
			_lerp_uv(sv.text_coord, sv.position.w, ev.text_coord, ev.position.w, t, v.text_coord);
			//f32 vd = _depth_buffer->get_data(RBMath::round_f(v.position.x) + _depth_buffer->w*0.5, -RBMath::round_f(v.position.y) + _depth_buffer->h *0.5);
			//if(vd>1|| (v.position.z > vd))
			{
				//if (quad.is_contain(v.position))
				{
					_stage_ps->proccess(v);
					_stage_om->proccess(v, (_color_buffer), _main_buffer, *(_depth_buffer));
				}
			}
		}
	}

	return false;
}


bool SrRasterizer::_discard_invisible_tiled(const VertexP3N3T2& data, const RBAABB2D& quad)
{
	RBVector2 v(data.position.x, data.position.y);
	if (!quad.is_contain(v))
	{
		return false;
	}
	_total_frag++;
	return true;
}
