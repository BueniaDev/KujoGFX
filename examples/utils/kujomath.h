#ifndef KUJOMATH_H
#define KUJOMATH_H

#include <iostream>
#include <cstdint>
#include <cmath>
using namespace std;

#ifndef M_PI
#define M_PI 3.1415926535
#endif

namespace kujomath
{
    template<typename T>
    struct KujoVec3
    {
	T xpos;
	T ypos;
	T zpos;

	KujoVec3() : xpos(0), ypos(0), zpos(0)
	{

	}

	KujoVec3(T x, T y, T z) : xpos(x), ypos(y), zpos(z)
	{

	}

	KujoVec3<T> operator +(const KujoVec3<T> &vec) const
	{
	    return KujoVec3<T>((xpos + vec.xpos), (ypos + vec.ypos), (zpos + vec.zpos));
	}

	KujoVec3<T> operator -(const KujoVec3<T> &vec) const
	{
	    return KujoVec3<T>((xpos - vec.xpos), (ypos - vec.ypos), (zpos - vec.zpos));
	}
    };

    template<typename T>
    struct KujoVec4
    {
	T xpos;
	T ypos;
	T zpos;
	T wpos;

	KujoVec4() : xpos(0), ypos(0), zpos(0), wpos(0)
	{

	}

	KujoVec4(T x, T y, T z, T w) : xpos(x), ypos(y), zpos(z), wpos(w)
	{

	}

	T& operator [](int row)
	{
	    return reinterpret_cast<T*>(&xpos)[row];
	}

	const T& operator [](int row) const
	{
	    return reinterpret_cast<const T*>(&xpos)[row];
	}

	KujoVec4<T> operator +(const KujoVec4<T> &vec) const
	{
	    return KujoVec4<T>((xpos + vec.xpos), (ypos + vec.ypos), (zpos + vec.zpos), (wpos + vec.wpos));
	}

	KujoVec4<T> operator -(const KujoVec4<T> &vec) const
	{
	    return KujoVec4<T>((xpos - vec.xpos), (ypos - vec.ypos), (zpos - vec.zpos), (wpos - vec.wpos));
	}
    };

    template<typename T>
    struct KujoMat4x4
    {
	KujoVec4<T> xvec;
	KujoVec4<T> yvec;
	KujoVec4<T> zvec;
	KujoVec4<T> wvec;

	KujoMat4x4()
	{

	}

	KujoMat4x4(KujoVec4<T> x, KujoVec4<T> y, KujoVec4<T> z, KujoVec4<T> w) : xvec(x), yvec(y), zvec(z), wvec(w)
	{

	}

	KujoVec4<T>& operator [](int row)
	{
	    return reinterpret_cast<KujoVec4<T>*>(&xvec)[row];
	}

	const KujoVec4<T>& operator [](int row) const
	{
	    return reinterpret_cast<const KujoVec4<T>*>(&xvec)[row];
	}

	KujoMat4x4<T> operator +(const KujoMat4x4<T> &mat) const
	{
	    return KujoMat4x4<T>((xvec + mat.xvec), (yvec + mat.yvec), (zvec + mat.zvec), (wvec + mat.wvec));
	}

	KujoMat4x4<T> operator -(const KujoMat4x4<T> &mat) const
	{
	    return KujoMat4x4<T>((xvec - mat.xvec), (yvec - mat.yvec), (zvec - mat.zvec), (wvec - mat.wvec));
	}

	KujoMat4x4<T> operator *(const KujoMat4x4<T> &mat) const
	{
	    KujoMat4x4<T> result;

	    for (int i = 0; i < 4; i++)
	    {
		for (int j = 0; j < 4; j++)
		{
		    result[i][j] = 0;

		    for (int k = 0; k < 4; k++)
		    {
			result[i][j] += (*this)[i][k] * mat[k][j];
		    }
		}
	    }

	    return result;
	}
    };

    using KujoVec3F = KujoVec3<float>;
    using KujoVec4F = KujoVec4<float>;
    using KujoMat4x4F = KujoMat4x4<float>;

    float toRadians(float degrees)
    {
	return ((degrees * M_PI) / 180.f);
    }

    KujoVec3F normalizeVec3F(KujoVec3F vec)
    {
	float length = sqrtf((vec.xpos * vec.xpos) + (vec.ypos * vec.ypos) + (vec.zpos * vec.zpos));

	if (length == 0.f)
	{
	    return vec;
	}

	return KujoVec3F((vec.xpos / length), (vec.ypos / length), (vec.zpos / length));
    }

    KujoVec3F crossVec3F(KujoVec3F a, KujoVec3F b)
    {
	return KujoVec3F(
	    ((a.ypos * b.zpos) - (a.zpos * b.ypos)),
	    ((a.zpos * b.xpos) - (a.xpos * b.zpos)),
	    ((a.xpos * b.ypos) - (a.ypos * b.xpos))
	);
    }

    float dotVec3F(KujoVec3F a, KujoVec3F b)
    {
	return ((a.xpos * b.xpos) + (a.ypos * b.ypos) + (a.zpos * b.zpos));
    }

    KujoMat4x4F perspectiveFovRH(float fovy, float aspect, float zn, float zf)
    {
	float yscale = (1.f / tanf(fovy / 2.f));
	float xscale = (yscale / aspect);

	return KujoMat4x4F(
	    KujoVec4F(xscale, 0.f, 0.f, 0.f),
	    KujoVec4F(0.f, yscale, 0.f, 0.f),
	    KujoVec4F(0.f, 0.f, (zf / (zn - zf)), -1.f),
	    KujoVec4F(0.f, 0.f, (zn * zf / (zn - zf)), 0.f)
	);
    }

    KujoMat4x4F lookAtRH(KujoVec3F eye, KujoVec3F at, KujoVec3F up)
    {
	KujoVec3F zaxis = normalizeVec3F(eye - at);
	KujoVec3F xaxis = normalizeVec3F(crossVec3F(up, zaxis));
	KujoVec3F yaxis = crossVec3F(zaxis, xaxis);

	return KujoMat4x4F(
	    KujoVec4F(xaxis.xpos, yaxis.xpos, zaxis.xpos, 0.f),
	    KujoVec4F(xaxis.ypos, yaxis.ypos, zaxis.ypos, 0.f),
	    KujoVec4F(xaxis.zpos, yaxis.zpos, zaxis.zpos, 0.f),
	    KujoVec4F(-dotVec3F(xaxis, eye), -dotVec3F(yaxis, eye), -dotVec3F(zaxis, eye), 1.f)
	);
    }

    KujoMat4x4F rotateX(float angle)
    {
	float sine = sinf(angle);
	float cosine = cosf(angle);

	return KujoMat4x4F(
	    KujoVec4F(1.f, 0.f, 0.f, 0.f),
	    KujoVec4F(0.f, cosine, sine, 0.f),
	    KujoVec4F(0.f, -sine, cosine, 0.f),
	    KujoVec4F(0.f, 0.f, 0.f, 1.f)
	);
    }

    KujoMat4x4F rotateY(float angle)
    {
	float sine = sinf(angle);
	float cosine = cosf(angle);

	return KujoMat4x4F(
	    KujoVec4F(cosine, 0.f, -sine, 0.f),
	    KujoVec4F(0.f, 1.f, 0.f, 0.f),
	    KujoVec4F(sine, 0.f, cosine, 0.f),
	    KujoVec4F(0.f, 0.f, 0.f, 1.f)
	);
    }
};


#endif // KUJOMATH_H