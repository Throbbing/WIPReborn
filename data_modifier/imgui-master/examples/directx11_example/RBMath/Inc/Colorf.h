#pragma once

#include "./Platform/RBBasedata.h"
#include "RBMath.h"


class RBColor32;
class RBVector4;

//Clamp
class RBColorf
{
public:
	//0.0f-1.0f
	f32 r, g, b, a;
	RBColorf(f32 val):r(val),g(val),b(val),a(1.f){}
	RBColorf();
	RBColorf(f32 r, f32 g, f32 b, f32 a);
	RBColorf(f32 r, f32 g, f32 b);
	RBColorf(const RBVector4& v);
	bool equal(const RBColorf& other);
	f32 get_grayscale();
	void to_linear();
	//[Kinds of operators(+,-,*,/)] 

  //ignore a
  RBColorf get_sqrt()
  {
    r = RBMath::sqrt(r);
    g = RBMath::sqrt(g);
    b = RBMath::sqrt(b);
    return *this;
  }

  void clamp_to_0_1()
  {
	  r = RBMath::clamp(r, 0.f, 1.f);
	  g = RBMath::clamp(g, 0.f, 1.f);
	  b = RBMath::clamp(b, 0.f, 1.f);
  }

	bool operator==(const RBColorf& o) const
	{
		return r == o.r&&g == o.g&&b == o.g&&a == o.a;
	}

	FORCEINLINE RBColorf& operator=(const RBColorf& b)
	{
		this->r = b.r;
		this->g = b.g;
		this->b = b.b;
		this->a = b.a;
		return *this;
	}

	FORCEINLINE RBColorf operator+(f32 a) const
	{
		return RBColorf(r+a,g+a,b+a,this->a+a);
	}

	FORCEINLINE RBColorf operator+(const RBColorf& o) const
	{
		return RBColorf(r + o.r, g + o.g, b + o.b, a + o.a);
	}

	FORCEINLINE RBColorf operator-(f32 a) const
	{
		return RBColorf(r - a, g - a, b - a, this->a - a);
	}

	FORCEINLINE RBColorf operator-(const RBColorf& o) const
	{
		return RBColorf(r - o.r, g - o.g, b - o.b, a - o.a);
	}

	FORCEINLINE RBColorf operator*(f32 a) const
	{
		return RBColorf(r * a, g * a, b * a, this->a * a);
	}

	FORCEINLINE RBColorf operator*(const RBColorf& o) const
	{
		return RBColorf(r * o.r, g * o.g, b * o.b, a * o.a);
	}

	FORCEINLINE RBColorf operator+=(f32 a)
	{
		r += a; g += a; b += a; this->a += a;
		return *this;
	}

	FORCEINLINE RBColorf operator+=(const RBColorf& o)
	{
		r += o.r; g += o.g; b += o.b; this->a += o.a;
		return *this;
	}

	FORCEINLINE RBColorf operator-=(f32 a)
	{
		r -= a; g -= a; b -= a; this->a -= a;
		return *this;
	}

	FORCEINLINE RBColorf operator-=(const RBColorf& o)
	{
		r -= o.r; g -= o.g; b -= o.b; this->a -= o.a;
		return *this;
	}

	FORCEINLINE RBColorf operator*=(f32 a)
	{
		r *= a; g *= a; b *= a; this->a *= a;
		return *this;
	}

	FORCEINLINE RBColorf operator*=(const RBColorf& o)
	{
		r *= o.r; g *= o.g; b *= o.b; this->a *= o.a;
		return *this;
	}

	RBColorf operator/=(f32 c);

	FORCEINLINE RBColorf operator/(const RBColorf& o) const
	{
    RBColorf c = *this;
		c.r /= o.r; c.g /= o.g; c.b /= o.b; c.a /= o.a;
		return c;
	}

	FORCEINLINE f32 y() const
	{
		const float YWeight[3] = { 0.212671f, 0.715160f, 0.072169f };
		return YWeight[0] * r + YWeight[1] * g + YWeight[2] * b;
	}

	FORCEINLINE f32 power_y(ft times) const
	{
		return y()*times;
	}

	bool is_black() const
	{
		return RBMath::is_nearly_zero(r) && RBMath::is_nearly_zero(g) && RBMath::is_nearly_zero(b);
	}

  bool is_avaliable() const
  {
    bool b1 = RBMath::is_NaN_f32(r);
    bool b2 = RBMath::is_NaN_f32(g);
    bool b3 = RBMath::is_NaN_f32(b);
    bool b4 = RBMath::is_NaN_f32(a);
    return !(b1||b2||b3||b4);
  }

	RBColorf(const RBColor32& color32);

	
	static const RBColorf
		red,
		green,
		blue,
		white,
		black,
		yellow,
		cyan,
		magenta,
		gray,
		grey,
		blank;
		

	void out_cyan();

	void out() const;
};

FORCEINLINE RBColorf operator*(f32 scale, const RBColorf& v)
{
	return v.operator*(scale);
}

template <> struct TIsPODType < RBColorf > { enum { v = true }; };