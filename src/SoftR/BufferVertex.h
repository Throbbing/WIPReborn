#pragma once
#include "BufferBase.h"
#include <vector>
#include "VertexFormat.h"

class SrBufferVertex
{
public:
	SrBufferVertex();
	~SrBufferVertex();

	VertexP3N3T2& operator[](unsigned int index)
	{
		return _vert[index];
	}
	const VertexP3N3T2& operator[](unsigned int index) const
	{
		return _vert[index];
	}

	void put_data(const VertexP3N3T2& data)
	{
		_vert[pos++]=(data);
	}
	void resize(size_t sz)
	{
		delete[] _vert;
		_vert = new VertexP3N3T2[sz];
		pos = 0;
	}
private:
	VertexP3N3T2* _vert=nullptr;
	size_t pos = 0;
};

