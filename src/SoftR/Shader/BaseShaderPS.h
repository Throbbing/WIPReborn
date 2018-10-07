#include "ShaderPixel.h"
#include "Texture2D.h"
#include "SamplePoint.h"

class BaseShaderPS : public SrShaderPixel
{
public:
	float lum(const RBColorf& c)
	{
		return c.r*0.299 + 0.587*c.g + c.b*0.114;
	}
	//作为成员变量线程不安全！
	RBColorf shade(VertexP3N3T2& vert_lerp)
	{
		//return RBColorf::white;
		SrSamplerPoint sp;
		SrTexture2D* tex;
		RBColorf tc;
		//高消耗
		tex = get_texture2d_index(0);
		tc = sp.sample_l(tex, vert_lerp.text_coord.x, vert_lerp.text_coord.y);
		//tc.r = RBMath::pow(tc.r, 2.2);
		//tc.g = RBMath::pow(tc.g, 2.2);
		//tc.b = RBMath::pow(tc.b, 2.2);
		//tc = RBColorf::white;
		
		vert_lerp.normal.normalize();
		//vert_lerp.normal = vert_lerp.normal.get_abs();
		
		RBVector3 v = vert_lerp.position;
		v = v.get_abs();
		v.normalize();

		RBVector3 light(0.f,-12.2f,1.f);

		RBVector3 half = light + v;
		half.normalize();

		float cost = RBVector3::dot_product(vert_lerp.normal, light);

		RBColorf oc = RBColorf::white;
		if (cost <= 0.f)
			oc = RBColorf::black;
		RBVector4 outc = oc;
		outc = outc * cost;
		outc.w = 1.0f;

		if (outc.x > 1.f) outc.x = 1.f;
		if (outc.y > 1.f) outc.y = 1.f;
		if (outc.z > 1.f) outc.z = 1.f;

		if (tc.a < 0.0001)
			RBColorf f = RBColorf::black;

		RBVector4 abt(0.2f,0.2f,0.2f,1.0f);


		RBColorf c =outc* tc + abt*tc;

		//c.r = RBMath::pow(c.r, 1/2.2);
		//c.g = RBMath::pow(c.g, 1 / 2.2);
		//c.b = RBMath::pow(c.b, 1 / 2.2);

		//c.a = RBMath::clamp(c.a, 0.f, 1.f);
		//c.r = RBMath::clamp(c.r, 0.f, 1.f);
		//c.g = RBMath::clamp(c.g, 0.f, 1.f);
		//c.b = RBMath::clamp(c.b, 0.f, 1.f);
		
		//if (tc.r < 0.0001&&tc.g < 0.0001&&tc.b < 0.0001&&tc.a < 1)
			//c = RBColorf::black;
		//c = RBVector4(vert_lerp.normal, 1);
	

		return c;
	}

private:



};

class PBRShaderPS : public SrShaderPixel
{
public:
	inline RBColorf lambert_brdf(const RBColorf& cdiffuse)
	{
		return cdiffuse / PI;
	}

	inline RBVector3 blinn_phong_brdf(const RBVector3& normal,const RBVector3& lighting, const RBVector3& view_dir, float power, const RBVector3& cspecular)
	{
		RBVector3 half_dir = (lighting + view_dir).get_normalized();
		float fs = RBMath::pow(RBMath::clamp(RBVector3::dot_product(normal, half_dir),0.f,1.f), power)*(8.f + power) / 8.f;
		return cspecular*fs / PI;
	}
	float lum(const RBColorf& c)
	{
		return c.r*0.299 + 0.587*c.g + c.b*0.114;
	}
	//作为成员变量线程不安全！
	RBColorf shade(VertexP3N3T2& vert_lerp)
	{
		//return RBColorf::white;
		SrSamplerPoint sp;
		SrTexture2D* tex;
		SrTexture2D* tex_mra; 
		SrTexture2D* tex_normal;
		RBColorf tc;
		//高消耗
		tex = get_texture2d_index(0);
		tex_mra = get_texture2d_index(1);
		tex_normal = get_texture2d_index(2);
		tc = sp.sample_l(tex, vert_lerp.text_coord.x, vert_lerp.text_coord.y);
		tc.r = RBMath::pow(tc.r, 2.2f);
		tc.g = RBMath::pow(tc.g, 2.2f);
		tc.b = RBMath::pow(tc.b, 2.2f);
		//tc = RBColorf::white;

		vert_lerp.normal.normalize();
		//vert_lerp.normal = vert_lerp.normal.get_abs();

		RBVector3 v = vert_lerp.position;
		v = v.get_abs();
		v.normalize();

		RBVector3 light(0.f, 12.2f, 1.f);

		RBVector3 half = light + v;
		half.normalize();

		float cost = RBVector3::dot_product(vert_lerp.normal, light);

		float r = sp.sample(tex_mra, vert_lerp.text_coord.x, vert_lerp.text_coord.y).g+0.001f;

		RBColorf oc = RBColorf::white;
		if (cost <= 0)
			oc = RBColorf::black;
		RBVector4 outc = blinn_phong_brdf(vert_lerp.normal,light,v,1.f/RBMath::pow(r,3),RBVector3(1,1,1)) + RBVector4(lambert_brdf(tc));
		outc = outc * cost;
		outc.w = 1.0;

		if (outc.x > 1) outc.x = 1.f;
		if (outc.y > 1) outc.y = 1.f;
		if (outc.z > 1) outc.z = 1.f;

		if (tc.a < 0.0001)
			RBColorf f = RBColorf::black;

		RBVector4 abt(0.2f, 0.2f, 0.2f, 1.0f);


		RBColorf c = outc* tc + abt*tc;

		

		return c;
	}

private:
};


class CopyShaderPS : public SrShaderPixel
{
public:
	RBColorf shade(VertexP3N3T2& vert_lerp)
	{
		SrSamplerPoint sp;
		SrTexture2D* tex;
		RBColorf tc;
		tex = get_texture2d_index(0);
		tc = sp.sample_l(tex, vert_lerp.text_coord.x, vert_lerp.text_coord.y);
		RBColorf c = tc;
		//c.a = RBMath::clamp(c.a, 0.f, 1.f);
		//c.r = RBMath::clamp(c.r, 0.f, 1.f);
		//c.g = RBMath::clamp(c.g, 0.f, 1.f);
		//c.b = RBMath::clamp(c.b, 0.f, 1.f);
		c.r = RBMath::pow(c.r, 1 / 2.2f);
		c.g = RBMath::pow(c.g, 1 / 2.2f);
		c.b = RBMath::pow(c.b, 1 / 2.2f);

		float fct = 2.f;
		c.r /= fct;
		c.g /= fct;
		c.b /= fct;
		c.a = 1.f;

		return c;
	}

};
