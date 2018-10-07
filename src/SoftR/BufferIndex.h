#pragma once
#include "BufferBase.h"
#include <vector>
#include "VertexFormat.h"
#include "time.h"

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
	void load3(uint idx,uint* v) const
	{
		memcpy(v, &_indeces[idx], sizeof(uint) * 3);
	}
	void put_data(unsigned int data)
	{
		_indeces[pos++]=(data);
	}
	void resize(size_t size)
	{
		delete[] _indeces;
		_indeces = new unsigned int[size];
		pos = 0;
		sz = size;
	}
	void shuffle()
	{
		srand(unsigned(time(NULL)));
		unsigned int k[3] = {0};
		for (int i = sz/3; i > 0; i--)
		{
			int index = (rand() % i)*3;
			k[0] = _indeces[index];
			k[1] = _indeces[index+1];
			k[2] = _indeces[index+2];
			int kk = i * 3;
			_indeces[index] = _indeces[kk];
			_indeces[index+1] = _indeces[kk+1];
			_indeces[index+2] = _indeces[kk+2];
			_indeces[kk] = k[0];
			_indeces[kk+1] = k[1];
			_indeces[kk+2] = k[2];
		}
	}
private:
	//std::vector<unsigned int> _indeces;
	unsigned int* _indeces=nullptr;
	size_t pos=0;
	size_t sz=0;
};

