#include "BufferIndex.h"

SrBufferIndex::SrBufferIndex()
{
}

SrBufferIndex::~SrBufferIndex()
{
	delete[] _indeces;
}