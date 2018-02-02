#pragma once
#include "../RBMath/Inc/Vector4.h"
#include "VertexFormat.h"
#include "../MemoryPool.h"
#include "../MemoryFrame.h"
#include <vector>
#include <mutex>
#define POOL
struct SrTriangle
{
	VertexP3N3T2 v[3];

	SrTriangle(VertexP3N3T2 v1, VertexP3N3T2 v2, VertexP3N3T2 v3)
	{
		v[0] = v1;
		v[1] = v2;
		v[2] = v3;
	}

	static void* operator new(size_t size)
	{

#ifdef POOL
		
		return pool->alloc(&nodes, size);
		//printf("new\n");
		//print_mem();
#else
		return frame->alloc(size, false);
#endif


	}

	static void operator delete(void* p)
	{
#ifdef POOL


		pool->free(&nodes, p, sizeof(SrTriangle));
		//printf("delete\n");
		//print_mem();
#endif
	}
	
	static bool Init()
	{
#ifdef POOL
		pool = RBPoolAllctor::instance();
		nodes = nullptr;
		pool->new_pool(&nodes, sizeof(SrTriangle));
		print_mem();
#else   

		frame = new RBFrameAlloctor();
		//20M
		frame->init((1 << 20)*20);
		frame->getframe(me, false);
#endif
		return true;
	}

#ifndef POOL
	static void ReleaseAll()
	{
		frame->release(me);
	}

	static void Deinit()
	{
		frame->shutdown();
		frame = nullptr;
	}
#else
	static void print_mem()
	{
		pool->print_list<SrTriangle>(nodes);
	}
#endif
	
	void print()
	{
		printf("%02X:%f\n",this,v[1].normal.x);
	}
#ifdef POOL
	static void* nodes;
	static RBPoolAllctor* pool;
#else
	static MemoryFrame me;
	static RBFrameAlloctor* frame;
#endif

};


struct SrFragment
{
	std::vector<VertexP3N3T2> frag;
};

//屏幕空间buffer，专门用于处理和屏幕空间相关的数据
//左下角原点
template <class T>
struct SrSSBuffer
{
	SrSSBuffer()
	{
		w = 0;
		h = 0;
		offset = 0;
		size = 0;
	}

	void init(int w,int h)
	{
		this->w = w;
		this->h = h;
		_buffer.resize(w*h);
		offset = w*sizeof(T);
		size = w*h*sizeof(T);
	}

	void set_vals(T v)
	{
		for (auto& i : _buffer)
		{
			i = v;
		}
	}

	void set_data(int x,int y,T data)
	{
		std::lock_guard<std::mutex> lk(lock);
		int index = y*w + x;
		if ((size_t)index<_buffer.size())
		{
			_buffer[index] = data;
		}
	}

	T get_data(int x,int y)
	{
		int index = y*w + x;
		if ((size_t)index < _buffer.size())
			return _buffer[index];
		else
			return _buffer[_buffer.size()-1];
	}

	void clear()
	{
		_buffer.clear();
	}

	~SrSSBuffer()
	{
		std::vector<T>().swap(_buffer);
	}

	std::vector<T>& get_buffer()
	{
		return _buffer;
	}

	int w;
	int h;
	int offset;
	unsigned int size;

private:
	std::vector<T> _buffer;
	std::mutex lock;
};