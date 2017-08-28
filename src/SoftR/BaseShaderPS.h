#include "ShaderPixel.h"
#include "Texture2D.h"
#include "SamplePoint.h"

class BaseShaderPS : public SrShaderPixel
{
public:
	//作为成员变量线程不安全！
	RBColorf shade(VertexP3N3T2& vert_lerp)
	{
		SrSamplerPoint sp;
		SrTexture2D* tex;
		RBColorf tc;
		//高消耗
		tex = get_texture2d_index(0);
		tc = sp.sample(tex, vert_lerp.text_coord.x, vert_lerp.text_coord.y);
		tc.r = RBMath::pow(tc.r, 2.2);
		tc.g = RBMath::pow(tc.g, 2.2);
		tc.b = RBMath::pow(tc.b, 2.2);
		//tc = RBColorf::white;
		
		vert_lerp.normal.normalize();
		//vert_lerp.normal = vert_lerp.normal.get_abs();
		
		RBVector3 v = vert_lerp.position;
		v = v.get_abs();
		v.normalize();

		RBVector3 light(0,1,-1);

		RBVector3 half = light + v;
		half.normalize();

		float cost = RBVector3::dot_product(vert_lerp.normal, light);

		RBColorf oc = RBColorf::white;
		if (cost <= 0)
			oc = RBColorf::black;
		RBVector4 outc = oc;
		outc = outc * cost;
		outc.w = 1.0;

		if (outc.x > 1) outc.x = 1.f;
		if (outc.y > 1) outc.y = 1.f;
		if (outc.z > 1) outc.z = 1.f;

		if (tc.a < 0.0001)
			RBColorf f = RBColorf::black;

		RBVector4 abt(0.08,0.07,0.08,1.0);


		RBColorf c =outc* tc + abt*tc;

		c.r = RBMath::pow(c.r, 1/2.2);
		c.g = RBMath::pow(c.g, 1 / 2.2);
		c.b = RBMath::pow(c.b, 1 / 2.2);

		c.a = RBMath::clamp(c.a, 0.f, 1.f);
		c.r = RBMath::clamp(c.r, 0.f, 1.f);
		c.g = RBMath::clamp(c.g, 0.f, 1.f);
		c.b = RBMath::clamp(c.b, 0.f, 1.f);
		
		//if (tc.r < 0.0001&&tc.g < 0.0001&&tc.b < 0.0001&&tc.a < 1)
			//c = RBColorf::black;
		//c = RBVector4(vert_lerp.normal, 1);
		
		return c;
	}

private:



};