#pragma once
#include "BufferConstant.h"
#include <vector>
#include <map>
#include <string>
#include "Texture2D.h"

#define MAX_BUFFER 32

enum SrShaderTypes
{
	E_VERTEX,
	E_GEOMETRY,
	E_PIXEL,
	E_UNKOWN,
	E_TOTAL,
};


class SrShaderBase
{
public:
	SrShaderBase();
	~SrShaderBase();

	SrBufferConstant* get_cbuffer_index(int index)
	{
		return _constant_buffers_array[index];
	}

	void set_constant_buffer_index(int index, SrBufferConstant* cb)
	{
		//assert
		_constant_buffers_array[index] = cb;
	}

	SrTexture2D* get_texture2d_index(int index)
	{
		return _tex_2d_array[index];
	}

	void set_texture_index(int index, SrTexture2D* cb)
	{
		_tex_2d_array[index] = cb;
	}

	SrBufferConstant* get_cbuffer(std::string name)
	{
		return _constant_buffers[name];
	}

	void set_constant_buffer(std::string name,SrBufferConstant* cb)
	{
		_constant_buffers.insert(std::make_pair(name, cb));
	}

	SrTexture2D* get_texture2d(std::string name)
	{
		return _tex_2d[name];
	}

	void set_texture(std::string name, SrTexture2D* cb)
	{
		_tex_2d.insert(std::make_pair(name, cb));
	}

	void clear()
	{
		_constant_buffers.clear();
		_tex_2d.clear();
	}

protected:
	//凭名字索引constant buffer
	std::map<std::string,SrBufferConstant*> _constant_buffers;
	//纹理buffer
	std::map<std::string, SrTexture2D*> _tex_2d;
	//map实在太慢了，所以使用数组来处理，性能提升了50%以上
	SrBufferConstant*  _constant_buffers_array[MAX_BUFFER];
	SrTexture2D*  _tex_2d_array[MAX_BUFFER];

	SrShaderTypes _type;
private:

};

