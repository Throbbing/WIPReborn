#pragma once
#include "BufferBase.h"
#include <vector>
#include "VertexFormat.h"

class SrBufferVertex
{
public:
	SrBufferVertex();
	~SrBufferVertex();

	VertexP3N3T2*& operator[](unsigned int index)
	{
		return _vert[index];
	}
	VertexP3N3T2* operator[](unsigned int index) const
	{
		return _vert[index];
	}

	void put_data(VertexP3N3T2* data)
	{
		_vert.push_back(data);
	}
private:
	std::vector<VertexP3N3T2*> _vert;
};

