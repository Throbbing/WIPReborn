#pragma once
#include <thread>
#include "VertexFormat.h"
#include "InnerData.h"
#include <deque>
#include <mutex>
#include "StageOM.h"
#include "StagePS.h"
#include "StageVS.h"
#include "Rasterizer.h"
#include <condition_variable>
#include "..\\Logger.h"
#include "..\\ProducerConsumerQueue.h"
#include "../RBMath/Inc/Color32.h"
#include <condition_variable>
#include "../Uitilities.h"
#include "../ThreadPool.h"
#include "../Mem/MemoryFrame.h"
/*
保序问题，透明物体渲染http://zhuanlan.zhihu.com/sildenafil/20180902
http ://graphics.cs.cmu.edu/courses/15869/fall2014/article/1
*/
#include <atomic>




//const int thread_num = 2;
#define TRI

class SrSimGPU
{
public:
	struct DataPak
	{
		DataPak(SrSSBuffer<RBColorf>* colort, 
			SrSSBuffer<float>* deptht, SrStagePS*sp, SrStageOM* so, SrRasterizer* rs,
			RBSem* semt,SrTriangle* vt, SrStageVS* stage_vs=nullptr) :
			color(colort), depth(deptht),
			_stage_om(so), _stage_ps(sp), _raster(rs),sem(semt),v(vt),_stage_vs(stage_vs) {}
		SrSSBuffer<RBColorf>* color;
		SrSSBuffer<float>* depth;
		SrStagePS* _stage_ps;
		SrStageOM* _stage_om;
		SrRasterizer* _raster;
		SrTriangle* v;
		SrStageVS* _stage_vs;
		RBSem *sem;
	};
	struct DataPak2
	{
		DataPak2(SrSSBuffer<RBColorf>* colort,SrSSBuffer<RBColor32>* back,
			SrSSBuffer<float>* deptht, 
			SrStagePS*sp, SrStageOM* so, SrRasterizer* rs,
			RBSem* semt, const std::vector<SrTriangle*>& vt,const  RBAABBI& q, SrStageVS* stage_vs = nullptr) :
			color(colort),back_buffer(back), depth(deptht),
			_stage_om(so), _stage_ps(sp), _raster(rs), sem(semt), _stage_vs(stage_vs) 
		{
			v.resize(vt.size());
			memcpy(&v[0], &vt[0], vt.size() * sizeof(SrTriangle*));
			quad = q;
		}
		SrSSBuffer<RBColor32>* back_buffer;
		SrSSBuffer<RBColorf>* color;
		SrSSBuffer<float>* depth;
		SrStagePS* _stage_ps;
		SrStageOM* _stage_om;
		SrRasterizer* _raster;
		std::vector<SrTriangle*> v;
		SrStageVS* _stage_vs;
		RBSem *sem;
		RBAABBI quad;
	};
	struct DataPak1
	{
		DataPak1(SrRasterizer* rs,
			SrTriangle* tris_mem,size_t tris_mem_size,
			SrTriangle* inmem,size_t inmem_size,
			RBSem* semt, const SrBufferVertex & vertex_buffer,
			const SrBufferIndex & index_buffer, uint start,uint end, 
			SrStageVS* stage_vs,bool wire_f) :
			vertex_buffer_ref(vertex_buffer),
			index_buffer_ref(index_buffer),
			start_ref(start),end_ref(end),
			_raster(rs), sem(semt), _stage_vs(stage_vs),
			tris(tris_mem),tri_num(mem_size),mem(inmem),mem_size(inmem_size),wire(wire_f)
		{
		}
		SrRasterizer* _raster;
		SrStageVS* _stage_vs;
		const SrBufferVertex& vertex_buffer_ref;
		const SrBufferIndex&  index_buffer_ref;
		const uint start_ref;
		const uint end_ref;
		RBSem *sem;
		//local mem
		SrTriangle* tris=nullptr;
		size_t tri_num=0;
		SrTriangle* mem;
		size_t mem_size;

		bool wire;
	};
	struct DataPak3
	{
		DataPak3( SrStagePS*sp, SrStageOM* so, SrRasterizer* rs,
			RBSem* semt, const std::vector<SrTriangle*>& vt, const  RBAABB2D& q, bool bwire, SrStageVS* stage_vs = nullptr) :
			_stage_om(so), _stage_ps(sp), _raster(rs), sem(semt), _stage_vs(stage_vs),v(vt),wire(bwire)
		{
			//v.resize(vt.size());
			//memcpy(&v[0], &vt[0], vt.size() * sizeof(SrTriangle*));
			quad = q;
		}

		SrStagePS* _stage_ps;
		SrStageOM* _stage_om;
		SrRasterizer* _raster;
		const  std::vector<SrTriangle*>& v;
		SrStageVS* _stage_vs;
		RBSem *sem;
		RBAABB2D quad;
		bool wire;
	};
	class RenderTask2 : public Task
	{
	public:
		RenderTask2() { }
		virtual int run(int id) override
		{
			DataPak3* args = static_cast<DataPak3*>(this->arg_);

			for (auto i : args->v)
			{
				SrTriangle tri = *i;
				args->_raster->trangle_setup_gpu_tiled(&tri, args->quad,args->wire);
			}
			SrSimGPU::handle_count++;
			args->sem->signal();
			return 0;
		}

	};

	class RenderTaskSortEverywhere : public Task
	{
	public:
		RenderTaskSortEverywhere() { }
		virtual int run(int id) override
		{

			DataPak1* args = static_cast<DataPak1*>(this->arg_);

			uint pos = 0;

			for (uint i = args->start_ref; i <= args->end_ref; i += 3)
			{
				uint index = i;
				SrTriangle& tri = args->tris[pos++];
				tri.v[0] = args->vertex_buffer_ref[args->index_buffer_ref[index]];
				tri.v[1] = args->vertex_buffer_ref[args->index_buffer_ref[index + 1]];
				tri.v[2] = args->vertex_buffer_ref[args->index_buffer_ref[index + 2]];
			}


			args->_stage_vs->proccess(args->tris, pos);
			int size = args->_raster->near_far_cull(args->tris, args->mem, pos);
			size = args->_raster->back_cull(args->mem, args->tris, size);
			//size = args->_raster->proj_divide(args->tris, args->mem, size);

			float _viewport_w = (float)args->_raster->get_width();
			float _viewport_h = (float)args->_raster->get_height();

			for (int i = 0; i < size; ++i)
			{
				SrTriangle* tri = &args->tris[i];
				//把坐标转换到视口
				tri->v[0].position.x = tri->v[0].position.x*0.5f*_viewport_w;
				tri->v[0].position.y = tri->v[0].position.y*0.5f*_viewport_h;
				tri->v[1].position.x = tri->v[1].position.x*0.5f*_viewport_w;
				tri->v[1].position.y = tri->v[1].position.y*0.5f*_viewport_h;
				tri->v[2].position.x = tri->v[2].position.x*0.5f*_viewport_w;
				tri->v[2].position.y = tri->v[2].position.y*0.5f*_viewport_h;



				args->_raster->_gpu->put_res(tri, id);

			}
			//args->_raster->trangle_setup_gpu(&args->mem[i], args->wire);


			SrSimGPU::handle_count++;

			args->sem->signal();
			return 0;
		}

	};

	void add_task_sort_everywhere(const SrBufferVertex & vertex_buffer,
		const SrBufferIndex & index_buffer, uint start, uint end, bool w)
	{
		size_t sz = (end - start + 1) / 3 * sizeof(SrTriangle) * 5;
		SrTriangle* tris = (SrTriangle*)mem_list->frame.alloc(sz, false);
		SrTriangle* mem = (SrTriangle*)mem_list->frame.alloc(sz, false);
		DataPak1* pak = new(
			mem_list->frame.alloc(sizeof(DataPak1), false)
			) DataPak1(_raster, tris, sz, mem, sz, &sem, vertex_buffer, index_buffer, start, end, _stage_vs, w);
		RenderTaskSortEverywhere* task = new(
			mem_list->frame.alloc(sizeof(RenderTaskSortEverywhere), false)
			)
			RenderTaskSortEverywhere();

		task->setArg(pak);
		excutor.addTask(task);
		total_task++;
	}
	static void mem_list_init();
	static void mem_list_deinit();
	static ThreadMem *mem_list;
	void put_res(SrTriangle* tri,int id)
	{
		thread_res[id][thread_res_n[id]++] = tri;
	}
	SrSimGPU(int thread_n):excutor(thread_n)
	{
		sem.reset();
		total_task = 0;
		thread_res.resize(thread_n);
		thread_res_n.resize(thread_n);
		for (int i = 0; i < thread_n; ++i)
		{
			thread_res_n[i] = 0;
			thread_res[i] = new SrTriangle*[1000000];
		}

	}
	~SrSimGPU()
	{
		for (size_t i = 0; i < thread_res.size(); ++i)
		{
			delete[] thread_res[i];
		}
	}





	void add_task(const std::vector<SrTriangle*>& v, const RBAABB2D& quad,bool wire)
	{
		DataPak3* pak = new(
			mem_list->frame.alloc(sizeof(DataPak3), false)
			) DataPak3( _stage_ps, _stage_om, _raster, &sem, v, quad,wire);
		RenderTask2* task = new(
			mem_list->frame.alloc(sizeof(RenderTask2), false)
			)
			RenderTask2();
		task->setArg(pak);
		excutor.addTask(task);
		total_task++;
	}

	void wait()
	{

		for (int i = 0; i < total_task; ++i)
		{
			sem.wait();
		}
		if (total_task != handle_count)
		{
			printf("%d/%d\n", SrSimGPU::handle_count.load(),total_task);
		}
		total_task = 0;
		handle_count = 0;
		
	}
	void reuse_tri()
	{
		for (size_t i = 0; i < thread_res_n.size(); ++i)
		{
			thread_res_n[i] = 0;
		}
	}
	void release()
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				if (j == i)
					continue;
				for (auto p : debug_frag_pos[i])
				{
					for (auto p1 : debug_frag_pos[j])
					{
						if (p == p1)
							printf("fuck\n");
					}
				}
				
			}
		}
		for (int i = 0; i < 4; ++i)
		{
				debug_frag_pos[i].clear();
		}
		mem_list->frame.release(mem_list->mf);
		for (size_t i = 0; i < thread_res_n.size(); ++i)
		{
			thread_res_n[i] = 0;
		}
	}


	//extern static const int thread_num;
private:

	int write_num;

	

public:
	//void set_color_buffer(SrSSBuffer<RBColorf>* colort)
	//{
		//_color = colort;
	//}
#if 0
	void set_main_buffer(SrSSBuffer<RBColor32>* buffer)
	{
		_back_buffer = buffer;
	}
	void set_render_target(SrSSBuffer<RBColorf>* buffer)
	{
		_color = buffer;
		_back_buffer = nullptr;
	}
	void set_depth_buffer(SrSSBuffer<float>* deptht)
	{
		_depth = deptht;
	}
#endif // 0

	void set_stage_ps(SrStagePS* stage_ps)
	{
		_stage_ps = stage_ps;
	}
	void set_stage_vs(SrStageVS* stage_vs)
	{
		_stage_vs = stage_vs;
	}
	void set_stage_om(SrStageOM* stage_om)
	{
		_stage_om = stage_om;
	}
	void set_raster(SrRasterizer* rs)
	{
		_raster = rs;
	}
	float get_width()
	{
		return (float)_raster->get_width();
	}
	float get_height()
	{
		return (float)_raster->get_height();
	}
private:

	ThreadPool excutor;
public:
	std::vector<SrTriangle**> thread_res;
	std::vector<int> thread_res_n;
private:
	//SrSSBuffer<RBColor32>* _back_buffer = nullptr;
	///SrSSBuffer<RBColorf>* _color;
	//SrSSBuffer<float>* _depth;
	SrStagePS* _stage_ps;
	SrStageVS* _stage_vs;
	SrStageOM* _stage_om;
	SrRasterizer* _raster;
	RBSem sem;

	int total_task;

	static atomic_int handle_count;
public:

	std::vector<RBVector2> debug_frag_pos[4];
};
