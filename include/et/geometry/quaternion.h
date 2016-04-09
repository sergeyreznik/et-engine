/*
 * This file is part of `et engine`
 * Copyright 2009-2016 by Sergey Reznik
 * Please, modify content only if you know what are you doing.
 *
 */

#pragma once

namespace et
{
	template <typename T>
	struct Quaternion
	{
		T scalar;
		vector3<T> vector;

		Quaternion() :
			scalar(static_cast<T>(1)), vector(static_cast<T>(0)) { }

		Quaternion(T s, T x, T y, T z) :
			scalar(s), vector(x, y, z) { }

		explicit Quaternion(const vector3<T>& axis) :
			scalar(0), vector(axis) { }
		
		Quaternion(T angle, const vector3<T>& axis)
		{
			T half = angle  / static_cast<T>(2);
			scalar = static_cast<T>(std::cos(half));
			vector = static_cast<T>(std::sin(half)) * axis;
		}

		T& operator[](int32_t i)
			{ return *(&scalar + i); }

		const T& operator[](int32_t i) const
			{ return *(&scalar + i); }

		T& operator[](uint32_t i)
			{ return *(&scalar + i); }
		
		const T& operator[](uint32_t i) const
			{ return *(&scalar + i); }
		
		Quaternion operator !() const
			{ return Quaternion(scalar, -vector.x, -vector.y, -vector.z); }

		Quaternion operator - () const
			{ return Quaternion(-scalar, -vector.x, -vector.y, -vector.z); }
		
		Quaternion operator * (const Quaternion &q) const
		{
			Quaternion result;
			result.scalar = scalar * q.scalar - dot(vector, q.vector);
			result.vector = vector.cross(q.vector) + scalar * q.vector + q.scalar * vector;
			return result;
		};

		Quaternion operator + (Quaternion v) const
		{
			return Quaternion<T>(scalar + v.scalar,
				vector.x + v.vector.x, vector.y + v.vector.y, vector.z + v.vector.z);
		};
		
		Quaternion operator * (T v) const
			{ return Quaternion<T>(scalar * v, vector.x * v, vector.y * v, vector.z * v); };

		Quaternion operator / (T v) const
		{
			Quaternion result;
			result.scalar = scalar / v;
			result.vector = vector / v;
			return result;
		};
		
		T lengthSquared() const
			{ return scalar*scalar + vector.dotSelf(); }

		T length() const
			{ return std::sqrt(scalar*scalar + vector.dotSelf()); }

		void normalize() 
		{ 
			T len = lengthSquared();
			if (len > 0)
			{
				len = std::sqrt(len);
				scalar /= len;
				vector /= len;
			}
		}

		Quaternion<T> normalized() const
		{
			Quaternion<T> result(*this);
			result.normalize();
			return result;
		}

		vector3<T> transform(const vector3<T> &v) const
		{
			const Quaternion& thisOne = *this;
			return (thisOne * Quaternion(v) * (!thisOne)).vector;
		}

		vector3<T> invtransform(const vector3<T> &v) const
		{
			const Quaternion& thisOne = *this;
			return ((!thisOne) * Quaternion(v) * thisOne).vector;
		}

		Quaternion& operator *= (const Quaternion &q)
		{
			T s = scalar * q.scalar - dot(vector, q.vector);
			vector3<T> v = vector.cross(q.vector) + scalar * q.vector + q.scalar * vector;
			scalar = s;
			vector = v;
			return *this;
		}

		matrix4<T> toMatrix() const
		{
			const T one = static_cast<T>(1);
			const T two = static_cast<T>(2);
			
			T len = length();
			T qx = vector.x / len;
			T qy = vector.y / len;
			T qz = vector.z / len;
			T qw = scalar / len;
			return matrix4<T>
			(
				vector4<T>(one - two*qy*qy - two*qz*qz,       two*qx*qy - two*qz*qw,       two*qx*qz + two*qy*qw, 0.0f),
				vector4<T>(      two*qx*qy + two*qz*qw, one - two*qx*qx - two*qz*qz,       two*qy*qz - two*qx*qw, 0.0f),
				vector4<T>(      two*qx*qz - two*qy*qw,       two*qy*qz + two*qx*qw, one - two*qx*qx - two*qy*qy, 0.0f),
				vector4<T>(                       0.0f,                        0.0f,                        0.0f, 1.0f)
			);
		}
	};

	template <typename T>
	inline Quaternion<T> operator * (T value, const Quaternion<T>& q)
		{ return q * value; }
	
	template<typename T>
	inline Quaternion<T> slerp(const Quaternion<T>& from, const Quaternion<T>& to, T t)
	{
		const T one(1);
		const T epsilon(0.0001f);
		
		Quaternion<T> target = to;
		
		T cosom = dot(from.vector, to.vector) + from.scalar * to.scalar;
		if (cosom < 0.0f)
		{
			target = -to;
			cosom = -cosom;
		}
		
		T scale0 = one - t;
		T scale1 = t;
		if (one - cosom > epsilon)
		{
			T omega = std::acos(cosom);
			T sinom = one / std::sin(omega);
			scale0 = std::sin(scale0 * omega) * sinom;
			scale1 = std::sin(scale1 * omega) * sinom;
		}
		return from * scale0 + target * scale1;
	}
	
	template <typename T>
	inline Quaternion<T> normalize(const Quaternion<T>& q)
	{
		return q.normalized();
		T l = q.lengthSquared();
		return (l > 0) ? (q / std::sqrt(l)) : Quaternion<T>();
	}
}
