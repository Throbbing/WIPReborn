#include "BufferVertex.h"

SrBufferVertex::SrBufferVertex()
{

}

SrBufferVertex::~SrBufferVertex()
{
	delete[] _vert;
}