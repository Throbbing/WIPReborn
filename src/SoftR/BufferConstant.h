#pragma once


class SrBufferConstant
{
public:
	SrBufferConstant();
	~SrBufferConstant();
	void init(void* data,int size)
	{
		_size = size;
		_data = data;
	}


	void *_data;
	int _size;
private:

};

