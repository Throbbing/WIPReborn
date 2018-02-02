#pragma once
#include "./Platform/WindowsPlatformTemp.h"
#include "RBMathUtilities.h"
#include "Colorf.h"
class RBVector2;
class RBMatrix;
class RBVector3;

class RBVector4
{
public:
	f32 x,y,z,w;
	RBVector4(const RBVector3& v,f32 az = 1.0f);
	RBVector4(const RBColorf& color) :x(color.r), y(color.g), z(color.b), w(color.a){}
	explicit RBVector4(f32 ax = 0.0f,f32 ay = 0.0f,f32 az = 0.0f,f32 aw = 1.0f)
		:x(ax),y(ay),z(az),w(aw){}
	explicit RBVector4(RBVector2 axy, RBVector2 azw);
	explicit RBVector4(RBMath::EForceInits)
		:x(0.f),y(0.f),z(0.f),w(0.f){}
	FORCEINLINE f32 & operator[]( int32 index)
	{
		return (&x)[index];
	}
	FORCEINLINE f32 operator[]( int32 index) const
	{
		return (&x)[index];
	}
	FORCEINLINE void set( f32  ax, f32  ay, f32  az, f32  aw)
	{
		x = ax;
		y = ay;
		z = az;
		w = aw;
	}
	FORCEINLINE RBVector4 operator-() const
	{
		return RBVector4(-x,-y,-z,-w);
	}
	FORCEINLINE RBVector4 operator+( const RBVector4& v) const
	{
		return RBVector4(x+v.x,y+v.y,z+v.z,w+v.w);
	}
	FORCEINLINE RBVector4 operator+=( const RBVector4& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;
		return *this;
	}
	FORCEINLINE RBVector4 operator-( const RBVector4& v) const
	{
		return RBVector4(x-v.x,y-v.y,z-v.z,w-v.w);
	}
	FORCEINLINE RBVector4 operator*( f32  s) const
	{
		return RBVector4(x*s,y*s,z*s,w*s);
	}
	RBVector4 operator/( f32  s) const
	{
		const f32 s_inv = 1.f/s;
		return RBVector4(x*s_inv,y*s_inv,z*s_inv,w*s_inv);
	}

	void operator/=(f32 s)
	{
		const f32 s_inv = 1.f/s;
		x *= s_inv;
		y *= s_inv;
		z *= s_inv;
		w *= s_inv;
	}

	RBVector4 operator/( const RBVector4& v) const
	{
		return RBVector4(x/v.x,y/v.y,z/v.z,w/v.w); 
	}

	RBVector4 operator*( const RBVector4& v) const
	{
		return RBVector4(x*v.x,y*v.y,z*v.z,w*v.w); 
	}

	RBVector4 operator*=( const RBVector4& v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		w *= v.w;
		return *this;
	}

	RBVector4 operator/=( const RBVector4& v)
	{
		x /= v.x;
		y /= v.y;
		z /= v.z;
		w /= v.w;
		return *this;
	}

	RBVector4 operator*=( f32  s)
	{
		x *= s;
		y *= s;
		z *= s;
		w *= s;
		return *this;
	}

	static FORCEINLINE f32  dot3( const RBVector4& a, const RBVector4& b)
	{
		return a.x*b.x + a.y*b.y + a.z*b.z;
	}

	static FORCEINLINE f32  dot4( const RBVector4& a, const RBVector4& b)
	{
		return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
	}

	friend FORCEINLINE RBVector4 operator*( f32  s, const RBVector4& v)
	{
		return v.operator*(s);
	}

	bool operator==(const RBVector4& v) const
	{
		return ((x == v.x) && (y == v.y) && (z == v.z) && (w == v.w));
	}

	bool operator!=(const RBVector4& v) const
	{
		return ((x != v.x) || (y != v.y) || (z != v.z) || (w != v.w));
	}

	RBVector4 operator^( const RBVector4& v) const
	{
		return RBVector4(y*v.z-z*v.y,z*v.x-x*v.z,x*v.y-v.x*y,0.f);
	}

	bool tolerance_equals(const RBVector4& v, f32  tolerance=SMALL_F) const
	{
		return 
			RBMath::abs(x-v.x)<tolerance &&
			RBMath::abs(y-v.y)<tolerance &&
			RBMath::abs(z-v.z)<tolerance &&
			RBMath::abs(w-v.w)<tolerance;
	}

	bool is_unit3(f32  tolerance = SMALL_F) const
	{
		return RBMath::abs(1.f-squared_size3())<tolerance;
	}

	FORCEINLINE RBVector4 safe_normal3(f32  tolerance=SMALLER_F) const
	{
		const f32 s = x*x+y*y+z*z;
		if(s>tolerance)
		{
			const f32 scale = RBMath::inv_sqrt(s);
			return RBVector4(x*scale,y*scale,z*scale,0.f);
		}
		return RBVector4(0.f);
	}

	FORCEINLINE RBVector4 unsafe_normal3() const
	{
		const f32 s = RBMath::inv_sqrt(x*x+y*y+z*z);
		return RBVector4(x*s,y*s,z*s,0.f);
	}
	f32  size3() const
	{
		return RBMath::sqrt(x*x+y*y+z*z);
	}

	f32  squared_size3() const
	{
		return x*x+y*y+z*z;
	}

	/** Utility to check if there are any NaNs in this vector. */
	bool ContainsNaN() const;
	/** Utility to check if all of the components of this vector are nearly zero given the tolerance. */
	bool is_nearly_zero3(f32  tolerance=SMALL_F) const
	{
		return 
			(
			RBMath::abs(x)<tolerance &&
			RBMath::abs(y)<tolerance &&
			RBMath::abs(z)<tolerance
			);
	}

	/** Reflect vector. */
	RBVector4 reflect3(const RBVector4& normal) const
	{
		return *this - 2.f*dot3(*this,normal)*normal;
	}

	//Apply a matrix to this vector with side use,implemented in Matrix.h
	FORCEINLINE void apply_matrix(const RBMatrix& m);

	//Matrix multiple to right with a return value,implemented in Matrix.h
	FORCEINLINE RBVector4 operator*(const RBMatrix& m) const;

	void out() const;

	static  const RBVector4 zero_vector;
};

template <> struct TIsPODType<RBVector4> { enum { v = true }; };

