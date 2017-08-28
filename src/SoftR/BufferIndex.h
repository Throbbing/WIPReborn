#pragma once
#include "BufferBase.h"
#include <vector>
#include "VertexFormat.h"

class SrBufferIndex
{
public:
	SrBufferIndex();
	~SrBufferIndex();

	unsigned int& operator[](unsigned int index)
	{
		return _indeces[index];
	}
	unsigned int operator[](unsigned int index) const
	{
		return _indeces[index];
	}
	void put_data(unsigned int data)
	{
		_indeces.push_back(data);
	}
private:
	std::vector<unsigned int> _indeces;
};

