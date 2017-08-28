#include "ShaderVertex.h"
#include "..\\RBMath\\Inc\\Matrix.h"
#include "BufferConstant.h"


struct ShaderMatrixBuffer
{
	RBMatrix m;
	RBMatrix v;
	RBMatrix p;
};

class BaseShaderVS : public SrShaderVertex
{
public:
	void shade(VertexP3N3T2& v)
	{
		VertexP3N3T2 iv = v;
		SrBufferConstant* mb = get_cbuffer_index(0);
		ShaderMatrixBuffer* cmb = (ShaderMatrixBuffer*)(mb->_data);

		//right mult

		v.position = iv.position * cmb->m;
		v.position = v.position * cmb->v;
		v.position = v.position * cmb->p;

		RBMatrix rot = cmb->m.get_rotation();

		v.normal = RBVector4(v.normal,1) * rot;


	}


private:



};