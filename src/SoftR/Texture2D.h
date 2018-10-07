#pragma once

#include "..\\ResourceManager.h"
//#include "..\\Uitilities.h"
#include "..\\RBMath\\Inc\Colorf.h"
#include "..\\Logger.h"
#include "InnerData.h"
#include "../RBMath/Inc/Color32.h"

class SrTexture2D
{
public:
	~SrTexture2D()
	{
		if (_data)
			delete[]_data;

	}

	static SrTexture2D* creat(const char* filename)
	{
		SrTexture2D* ret = new SrTexture2D();
		ResHandler* _handler = ret->load_resource(filename);
		if (!_handler)
			printf("No this file!\n");
		TextureData *hd = (TextureData *)_handler->extra;
		ret->_w = hd->width; ret->_h = hd->height;
		ret->_size = hd->size;
		ret->_data = new float[ret->_size];
		unsigned char* p = (unsigned char*)_handler->ptr;
		for (unsigned int i = 0; i < ret->_h; ++i)
		{
			for (unsigned int j = 0; j < ret->_w; ++j)
			{

				int idx = (i*ret->_w + j)*4;
				ret->set_color(j, i, RBColorf( p[idx]/255.f,p[idx+1]/255.f,p[idx+2]/255.f,p[idx+3]/255.f));
			}
		}

		return ret;
	}

	static SrTexture2D* creat(const SrSSBuffer<RBColorf>& ssbuffer)
	{
		SrTexture2D* ret = new SrTexture2D();
		
		ret->_w = ssbuffer.w; ret->_h = ssbuffer.h;
		ret->_size = ssbuffer.size*4;
		ret->_data = new float[ret->_size];
		for (unsigned int i = 0; i < ret->_h; ++i)
		{
			for (unsigned int j = 0; j < ret->_w; ++j)
			{
				ret->set_color(j, i, RBColorf(ssbuffer.get_data(j, i)));
			}
		}
		return ret;
	}


	static SrTexture2D* creat(int width,int height)
	{
		SrTexture2D* ret = new SrTexture2D();

		ret->_w = width; ret->_h = height;
		ret->_size = width*height*4;
		ret->_data = new float[ret->_size];
		
		return ret;
	}

	void set_color(int x, int y,const RBColorf& c) const
	{
		unsigned int index = y*_w * 4 + x * 4;
		if (index < _size)
		{
			_data[index] = c.r;
			_data[index+1] = c.g;
			_data[index+2] = c.b;
			_data[index+3] = c.a;

		}
		else
		{

		}

	}

	RBColorf get_color(int x,int y) const
	{
		unsigned int index = y*_w*4 + x*4;
		if (index < _size)
		{
			RBColorf c;
			c.r = _data[index] ;
			c.g = _data[index + 1];
			c.b = _data[index + 2];
			c.a = _data[index + 3];
			return c;
		}
		else
		{
			g_logger->debug_log(WIP_WARNING, "Texture %d :Index out of texture 2d:%d/%d¡ª¡ªx:%d/%d::y:%d/%d", this, index, _size, x, _w, y, _h);
			return RBColorf::blank;
		}
		
	}

	inline unsigned int get_width() const
	{
		return _w;
	}

	inline unsigned int get_height() const
	{
		return _h;
	}
	inline size_t get_size()
	{
		return _size;
	}
	inline float* get_buffer()
	{
		return _data;
	}
protected:
	SrTexture2D()
	{
		_data = nullptr;
		_w = _h = 0;
		_size = 0;
	}

private:
	ResHandler * load_resource(const char* filename)
	{
		ResHandler* h = g_res_manager->load_resource(filename);
		if (h)
		{
			return h;
		}
		else
		{
			printf("load picture resource filed!");
			return NULL;
		}
	}



	unsigned int _w;
	unsigned int _h;
	unsigned int _size;
	float* _data;



};

