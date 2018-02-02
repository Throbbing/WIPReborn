#pragma once

#include "../ResourceManager.h"
#include "../RBMath/Inc/Colorf.h"
#include "../Logger.h"

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
      LOG_ERROR("No this file!\n");
		TextureData *hd = (TextureData *)_handler->extra;
		ret->_w = hd->width; ret->_h = hd->height;
		ret->_size = hd->size;
		ret->_data = new unsigned char[ret->_size];
		memcpy(ret->_data, _handler->ptr, _handler->size);
		g_res_manager->free(_handler, _handler->size);
		return ret;
	}

	RBColorf get_color(int x,int y) const
	{
		int index = y*_w*4 + x*4;
		if ((uint)index < _size)
		{
			RBColorf c;
			c.r = _data[index] / 255.f;
			c.g = _data[index + 1] / 255.f;
			c.b = _data[index + 2] / 255.f;
			c.a = _data[index + 3] / 255.f;
			return c;
		}
		else
		{
			g_logger->debug_log(WIP_WARNING, "Texture %d :Index out of texture 2d:%d/%d¡ª¡ªx:%d/%d::y:%d/%d", this, index, _size, x, _w, y, _h);
			return RBColorf::blank;
		}
		
	}

	unsigned int get_width() const
	{
		return _w;
	}

	unsigned int get_height() const
	{
		return _h;
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
			LOG_ERROR("load picture resource filed!");
			return NULL;
		}
	}



	unsigned int _w;
	unsigned int _h;
	unsigned int _size;
	unsigned char* _data;



};

