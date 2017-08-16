#include "stdafx.h"
#include "BBE/Vector2.h"
#include "BBE/Vector3.h"
#include "BBE/Vector4.h"
#include "BBE/Exceptions.h"
#include "BBE\Vector4.h"

bbe::Vector4::Vector4()
	: x(0), y(0), z(0), w(0)
{
	//UNTESTED
}

bbe::Vector4::Vector4(float xyzw)
	: x(xyzw), y(xyzw), z(xyzw), w(xyzw)
{
}

bbe::Vector4::Vector4(float xyz, float w)
	: x(xyz), y(xyz), z(xyz), w(w)
{
}

bbe::Vector4::Vector4(float x, float y, float z, float w)
	: x(x), y(y), z(z), w(w)
{
	//UNTESTED
}

bbe::Vector4::Vector4(float x, float y, const bbe::Vector2 &zw)
	: x(x), y(y), z(zw.x), w(zw.y)
{
	//UNTESTED
}

bbe::Vector4::Vector4(const bbe::Vector2 &xy, float z, float w)
	: x(xy.x), y(xy.y), z(z), w(w)
{
	//UNTESTED
}

bbe::Vector4::Vector4(const bbe::Vector2 &xy, const bbe::Vector2 &zw)
	: x(xy.x), y(xy.y), z(zw.x), w(zw.y)
{
	//UNTESTED
}

bbe::Vector4::Vector4(float x, const bbe::Vector2 &yz, float w)
	: x(x), y(yz.x), z(yz.y), w(w)
{
	//UNTESTED
}

bbe::Vector4::Vector4(const bbe::Vector3 &xyz, float w)
	: x(xyz.x), y(xyz.y), z(xyz.z), w(w)
{
	//UNTESTED
}

bbe::Vector4::Vector4(float x, const bbe::Vector3 &yzw)
	: x(x), y(yzw.x), z(yzw.y), w(yzw.z)
{
	//UNTESTED
}

bbe::Vector4 bbe::Vector4::operator+(const bbe::Vector4 & other) const
{
	//UNTESTED
	return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
}

bbe::Vector4 bbe::Vector4::operator-(const Vector4 & other) const
{
	//UNTESTED
	return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
}

bbe::Vector4 bbe::Vector4::operator-() const
{
	return Vector4(-x, -y, -z, -w);
}

bbe::Vector4 bbe::Vector4::operator*(float scalar) const
{
	//UNTESTED
	return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
}

bbe::Vector4 bbe::Vector4::operator/(float scalar) const
{
	//UNTESTED
	return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
}

float& bbe::Vector4::operator[](int index)
{
	//UNTESTED
	switch (index)
	{
	case 0:
		return x;
	case 1:
		return y;
	case 2:
		return z;
	case 3:
		return w;
	default:
		throw IllegalIndexException();
	}
}

const float& bbe::Vector4::operator[](int index) const
{
	//UNTESTED
	switch (index)
	{
	case 0:
		return x;
	case 1:
		return y;
	case 2:
		return z;
	case 3:
		return w;
	default:
		throw IllegalIndexException();
	}
}


//Start Swizzles
bbe::Vector2 bbe::Vector4::xx() const
{
	return Vector2(x, x);
}

bbe::Vector2 bbe::Vector4::xy() const
{
	return Vector2(x, y);
}

bbe::Vector2 bbe::Vector4::xz() const
{
	return Vector2(x, z);
}

bbe::Vector2 bbe::Vector4::xw() const
{
	return Vector2(x, w);
}

bbe::Vector2 bbe::Vector4::yx() const
{
	return Vector2(y, x);
}

bbe::Vector2 bbe::Vector4::yy() const
{
	return Vector2(y, y);
}

bbe::Vector2 bbe::Vector4::yz() const
{
	return Vector2(y, z);
}

bbe::Vector2 bbe::Vector4::yw() const
{
	return Vector2(y, w);
}

bbe::Vector2 bbe::Vector4::zx() const
{
	return Vector2(z, x);
}

bbe::Vector2 bbe::Vector4::zy() const
{
	return Vector2(z, y);
}

bbe::Vector2 bbe::Vector4::zz() const
{
	return Vector2(z, z);
}

bbe::Vector2 bbe::Vector4::zw() const
{
	return Vector2(z, w);
}

bbe::Vector2 bbe::Vector4::wx() const
{
	return Vector2(w, x);
}

bbe::Vector2 bbe::Vector4::wy() const
{
	return Vector2(w, y);
}

bbe::Vector2 bbe::Vector4::wz() const
{
	return Vector2(w, z);
}

bbe::Vector2 bbe::Vector4::ww() const
{
	return Vector2(w, w);
}

bbe::Vector3 bbe::Vector4::xxx() const
{
	return Vector3(x, x, x);
}

bbe::Vector3 bbe::Vector4::xxy() const
{
	return Vector3(x, x, y);
}

bbe::Vector3 bbe::Vector4::xxz() const
{
	return Vector3(x, x, z);
}

bbe::Vector3 bbe::Vector4::xxw() const
{
	return Vector3(x, x, w);
}

bbe::Vector3 bbe::Vector4::xyx() const
{
	return Vector3(x, y, x);
}

bbe::Vector3 bbe::Vector4::xyy() const
{
	return Vector3(x, y, y);
}

bbe::Vector3 bbe::Vector4::xyz() const
{
	return Vector3(x, y, z);
}

bbe::Vector3 bbe::Vector4::xyw() const
{
	return Vector3(x, y, w);
}

bbe::Vector3 bbe::Vector4::xzx() const
{
	return Vector3(x, z, x);
}

bbe::Vector3 bbe::Vector4::xzy() const
{
	return Vector3(x, z, y);
}

bbe::Vector3 bbe::Vector4::xzz() const
{
	return Vector3(x, z, z);
}

bbe::Vector3 bbe::Vector4::xzw() const
{
	return Vector3(x, z, w);
}

bbe::Vector3 bbe::Vector4::xwx() const
{
	return Vector3(x, w, x);
}

bbe::Vector3 bbe::Vector4::xwy() const
{
	return Vector3(x, w, y);
}

bbe::Vector3 bbe::Vector4::xwz() const
{
	return Vector3(x, w, z);
}

bbe::Vector3 bbe::Vector4::xww() const
{
	return Vector3(x, w, w);
}

bbe::Vector3 bbe::Vector4::yxx() const
{
	return Vector3(y, x, x);
}

bbe::Vector3 bbe::Vector4::yxy() const
{
	return Vector3(y, x, y);
}

bbe::Vector3 bbe::Vector4::yxz() const
{
	return Vector3(y, x, z);
}

bbe::Vector3 bbe::Vector4::yxw() const
{
	return Vector3(y, x, w);
}

bbe::Vector3 bbe::Vector4::yyx() const
{
	return Vector3(y, y, x);
}

bbe::Vector3 bbe::Vector4::yyy() const
{
	return Vector3(y, y, y);
}

bbe::Vector3 bbe::Vector4::yyz() const
{
	return Vector3(y, y, z);
}

bbe::Vector3 bbe::Vector4::yyw() const
{
	return Vector3(y, y, w);
}

bbe::Vector3 bbe::Vector4::yzx() const
{
	return Vector3(y, z, x);
}

bbe::Vector3 bbe::Vector4::yzy() const
{
	return Vector3(y, z, y);
}

bbe::Vector3 bbe::Vector4::yzz() const
{
	return Vector3(y, z, z);
}

bbe::Vector3 bbe::Vector4::yzw() const
{
	return Vector3(y, z, w);
}

bbe::Vector3 bbe::Vector4::ywx() const
{
	return Vector3(y, w, x);
}

bbe::Vector3 bbe::Vector4::ywy() const
{
	return Vector3(y, w, y);
}

bbe::Vector3 bbe::Vector4::ywz() const
{
	return Vector3(y, w, z);
}

bbe::Vector3 bbe::Vector4::yww() const
{
	return Vector3(y, w, w);
}

bbe::Vector3 bbe::Vector4::zxx() const
{
	return Vector3(z, x, x);
}

bbe::Vector3 bbe::Vector4::zxy() const
{
	return Vector3(z, x, y);
}

bbe::Vector3 bbe::Vector4::zxz() const
{
	return Vector3(z, x, z);
}

bbe::Vector3 bbe::Vector4::zxw() const
{
	return Vector3(z, x, w);
}

bbe::Vector3 bbe::Vector4::zyx() const
{
	return Vector3(z, y, x);
}

bbe::Vector3 bbe::Vector4::zyy() const
{
	return Vector3(z, y, y);
}

bbe::Vector3 bbe::Vector4::zyz() const
{
	return Vector3(z, y, z);
}

bbe::Vector3 bbe::Vector4::zyw() const
{
	return Vector3(z, y, w);
}

bbe::Vector3 bbe::Vector4::zzx() const
{
	return Vector3(z, z, x);
}

bbe::Vector3 bbe::Vector4::zzy() const
{
	return Vector3(z, z, y);
}

bbe::Vector3 bbe::Vector4::zzz() const
{
	return Vector3(z, z, z);
}

bbe::Vector3 bbe::Vector4::zzw() const
{
	return Vector3(z, z, w);
}

bbe::Vector3 bbe::Vector4::zwx() const
{
	return Vector3(z, w, x);
}

bbe::Vector3 bbe::Vector4::zwy() const
{
	return Vector3(z, w, y);
}

bbe::Vector3 bbe::Vector4::zwz() const
{
	return Vector3(z, w, z);
}

bbe::Vector3 bbe::Vector4::zww() const
{
	return Vector3(z, w, w);
}

bbe::Vector3 bbe::Vector4::wxx() const
{
	return Vector3(w, x, x);
}

bbe::Vector3 bbe::Vector4::wxy() const
{
	return Vector3(w, x, y);
}

bbe::Vector3 bbe::Vector4::wxz() const
{
	return Vector3(w, x, z);
}

bbe::Vector3 bbe::Vector4::wxw() const
{
	return Vector3(w, x, w);
}

bbe::Vector3 bbe::Vector4::wyx() const
{
	return Vector3(w, y, x);
}

bbe::Vector3 bbe::Vector4::wyy() const
{
	return Vector3(w, y, y);
}

bbe::Vector3 bbe::Vector4::wyz() const
{
	return Vector3(w, y, z);
}

bbe::Vector3 bbe::Vector4::wyw() const
{
	return Vector3(w, y, w);
}

bbe::Vector3 bbe::Vector4::wzx() const
{
	return Vector3(w, z, x);
}

bbe::Vector3 bbe::Vector4::wzy() const
{
	return Vector3(w, z, y);
}

bbe::Vector3 bbe::Vector4::wzz() const
{
	return Vector3(w, z, z);
}

bbe::Vector3 bbe::Vector4::wzw() const
{
	return Vector3(w, z, w);
}

bbe::Vector3 bbe::Vector4::wwx() const
{
	return Vector3(w, w, x);
}

bbe::Vector3 bbe::Vector4::wwy() const
{
	return Vector3(w, w, y);
}

bbe::Vector3 bbe::Vector4::wwz() const
{
	return Vector3(w, w, z);
}

bbe::Vector3 bbe::Vector4::www() const
{
	return Vector3(w, w, w);
}

bbe::Vector4 bbe::Vector4::xxxx() const
{
	return Vector4(x, x, x, x);
}

bbe::Vector4 bbe::Vector4::xxxy() const
{
	return Vector4(x, x, x, y);
}

bbe::Vector4 bbe::Vector4::xxxz() const
{
	return Vector4(x, x, x, z);
}

bbe::Vector4 bbe::Vector4::xxxw() const
{
	return Vector4(x, x, x, w);
}

bbe::Vector4 bbe::Vector4::xxyx() const
{
	return Vector4(x, x, y, x);
}

bbe::Vector4 bbe::Vector4::xxyy() const
{
	return Vector4(x, x, y, y);
}

bbe::Vector4 bbe::Vector4::xxyz() const
{
	return Vector4(x, x, y, z);
}

bbe::Vector4 bbe::Vector4::xxyw() const
{
	return Vector4(x, x, y, w);
}

bbe::Vector4 bbe::Vector4::xxzx() const
{
	return Vector4(x, x, z, x);
}

bbe::Vector4 bbe::Vector4::xxzy() const
{
	return Vector4(x, x, z, y);
}

bbe::Vector4 bbe::Vector4::xxzz() const
{
	return Vector4(x, x, z, z);
}

bbe::Vector4 bbe::Vector4::xxzw() const
{
	return Vector4(x, x, z, w);
}

bbe::Vector4 bbe::Vector4::xxwx() const
{
	return Vector4(x, x, w, x);
}

bbe::Vector4 bbe::Vector4::xxwy() const
{
	return Vector4(x, x, w, y);
}

bbe::Vector4 bbe::Vector4::xxwz() const
{
	return Vector4(x, x, w, z);
}

bbe::Vector4 bbe::Vector4::xxww() const
{
	return Vector4(x, x, w, w);
}

bbe::Vector4 bbe::Vector4::xyxx() const
{
	return Vector4(x, y, x, x);
}

bbe::Vector4 bbe::Vector4::xyxy() const
{
	return Vector4(x, y, x, y);
}

bbe::Vector4 bbe::Vector4::xyxz() const
{
	return Vector4(x, y, x, z);
}

bbe::Vector4 bbe::Vector4::xyxw() const
{
	return Vector4(x, y, x, w);
}

bbe::Vector4 bbe::Vector4::xyyx() const
{
	return Vector4(x, y, y, x);
}

bbe::Vector4 bbe::Vector4::xyyy() const
{
	return Vector4(x, y, y, y);
}

bbe::Vector4 bbe::Vector4::xyyz() const
{
	return Vector4(x, y, y, z);
}

bbe::Vector4 bbe::Vector4::xyyw() const
{
	return Vector4(x, y, y, w);
}

bbe::Vector4 bbe::Vector4::xyzx() const
{
	return Vector4(x, y, z, x);
}

bbe::Vector4 bbe::Vector4::xyzy() const
{
	return Vector4(x, y, z, y);
}

bbe::Vector4 bbe::Vector4::xyzz() const
{
	return Vector4(x, y, z, z);
}

bbe::Vector4 bbe::Vector4::xyzw() const
{
	return Vector4(x, y, z, w);
}

bbe::Vector4 bbe::Vector4::xywx() const
{
	return Vector4(x, y, w, x);
}

bbe::Vector4 bbe::Vector4::xywy() const
{
	return Vector4(x, y, w, y);
}

bbe::Vector4 bbe::Vector4::xywz() const
{
	return Vector4(x, y, w, z);
}

bbe::Vector4 bbe::Vector4::xyww() const
{
	return Vector4(x, y, w, w);
}

bbe::Vector4 bbe::Vector4::xzxx() const
{
	return Vector4(x, z, x, x);
}

bbe::Vector4 bbe::Vector4::xzxy() const
{
	return Vector4(x, z, x, y);
}

bbe::Vector4 bbe::Vector4::xzxz() const
{
	return Vector4(x, z, x, z);
}

bbe::Vector4 bbe::Vector4::xzxw() const
{
	return Vector4(x, z, x, w);
}

bbe::Vector4 bbe::Vector4::xzyx() const
{
	return Vector4(x, z, y, x);
}

bbe::Vector4 bbe::Vector4::xzyy() const
{
	return Vector4(x, z, y, y);
}

bbe::Vector4 bbe::Vector4::xzyz() const
{
	return Vector4(x, z, y, z);
}

bbe::Vector4 bbe::Vector4::xzyw() const
{
	return Vector4(x, z, y, w);
}

bbe::Vector4 bbe::Vector4::xzzx() const
{
	return Vector4(x, z, z, x);
}

bbe::Vector4 bbe::Vector4::xzzy() const
{
	return Vector4(x, z, z, y);
}

bbe::Vector4 bbe::Vector4::xzzz() const
{
	return Vector4(x, z, z, z);
}

bbe::Vector4 bbe::Vector4::xzzw() const
{
	return Vector4(x, z, z, w);
}

bbe::Vector4 bbe::Vector4::xzwx() const
{
	return Vector4(x, z, w, x);
}

bbe::Vector4 bbe::Vector4::xzwy() const
{
	return Vector4(x, z, w, y);
}

bbe::Vector4 bbe::Vector4::xzwz() const
{
	return Vector4(x, z, w, z);
}

bbe::Vector4 bbe::Vector4::xzww() const
{
	return Vector4(x, z, w, w);
}

bbe::Vector4 bbe::Vector4::xwxx() const
{
	return Vector4(x, w, x, x);
}

bbe::Vector4 bbe::Vector4::xwxy() const
{
	return Vector4(x, w, x, y);
}

bbe::Vector4 bbe::Vector4::xwxz() const
{
	return Vector4(x, w, x, z);
}

bbe::Vector4 bbe::Vector4::xwxw() const
{
	return Vector4(x, w, x, w);
}

bbe::Vector4 bbe::Vector4::xwyx() const
{
	return Vector4(x, w, y, x);
}

bbe::Vector4 bbe::Vector4::xwyy() const
{
	return Vector4(x, w, y, y);
}

bbe::Vector4 bbe::Vector4::xwyz() const
{
	return Vector4(x, w, y, z);
}

bbe::Vector4 bbe::Vector4::xwyw() const
{
	return Vector4(x, w, y, w);
}

bbe::Vector4 bbe::Vector4::xwzx() const
{
	return Vector4(x, w, z, x);
}

bbe::Vector4 bbe::Vector4::xwzy() const
{
	return Vector4(x, w, z, y);
}

bbe::Vector4 bbe::Vector4::xwzz() const
{
	return Vector4(x, w, z, z);
}

bbe::Vector4 bbe::Vector4::xwzw() const
{
	return Vector4(x, w, z, w);
}

bbe::Vector4 bbe::Vector4::xwwx() const
{
	return Vector4(x, w, w, x);
}

bbe::Vector4 bbe::Vector4::xwwy() const
{
	return Vector4(x, w, w, y);
}

bbe::Vector4 bbe::Vector4::xwwz() const
{
	return Vector4(x, w, w, z);
}

bbe::Vector4 bbe::Vector4::xwww() const
{
	return Vector4(x, w, w, w);
}

bbe::Vector4 bbe::Vector4::yxxx() const
{
	return Vector4(y, x, x, x);
}

bbe::Vector4 bbe::Vector4::yxxy() const
{
	return Vector4(y, x, x, y);
}

bbe::Vector4 bbe::Vector4::yxxz() const
{
	return Vector4(y, x, x, z);
}

bbe::Vector4 bbe::Vector4::yxxw() const
{
	return Vector4(y, x, x, w);
}

bbe::Vector4 bbe::Vector4::yxyx() const
{
	return Vector4(y, x, y, x);
}

bbe::Vector4 bbe::Vector4::yxyy() const
{
	return Vector4(y, x, y, y);
}

bbe::Vector4 bbe::Vector4::yxyz() const
{
	return Vector4(y, x, y, z);
}

bbe::Vector4 bbe::Vector4::yxyw() const
{
	return Vector4(y, x, y, w);
}

bbe::Vector4 bbe::Vector4::yxzx() const
{
	return Vector4(y, x, z, x);
}

bbe::Vector4 bbe::Vector4::yxzy() const
{
	return Vector4(y, x, z, y);
}

bbe::Vector4 bbe::Vector4::yxzz() const
{
	return Vector4(y, x, z, z);
}

bbe::Vector4 bbe::Vector4::yxzw() const
{
	return Vector4(y, x, z, w);
}

bbe::Vector4 bbe::Vector4::yxwx() const
{
	return Vector4(y, x, w, x);
}

bbe::Vector4 bbe::Vector4::yxwy() const
{
	return Vector4(y, x, w, y);
}

bbe::Vector4 bbe::Vector4::yxwz() const
{
	return Vector4(y, x, w, z);
}

bbe::Vector4 bbe::Vector4::yxww() const
{
	return Vector4(y, x, w, w);
}

bbe::Vector4 bbe::Vector4::yyxx() const
{
	return Vector4(y, y, x, x);
}

bbe::Vector4 bbe::Vector4::yyxy() const
{
	return Vector4(y, y, x, y);
}

bbe::Vector4 bbe::Vector4::yyxz() const
{
	return Vector4(y, y, x, z);
}

bbe::Vector4 bbe::Vector4::yyxw() const
{
	return Vector4(y, y, x, w);
}

bbe::Vector4 bbe::Vector4::yyyx() const
{
	return Vector4(y, y, y, x);
}

bbe::Vector4 bbe::Vector4::yyyy() const
{
	return Vector4(y, y, y, y);
}

bbe::Vector4 bbe::Vector4::yyyz() const
{
	return Vector4(y, y, y, z);
}

bbe::Vector4 bbe::Vector4::yyyw() const
{
	return Vector4(y, y, y, w);
}

bbe::Vector4 bbe::Vector4::yyzx() const
{
	return Vector4(y, y, z, x);
}

bbe::Vector4 bbe::Vector4::yyzy() const
{
	return Vector4(y, y, z, y);
}

bbe::Vector4 bbe::Vector4::yyzz() const
{
	return Vector4(y, y, z, z);
}

bbe::Vector4 bbe::Vector4::yyzw() const
{
	return Vector4(y, y, z, w);
}

bbe::Vector4 bbe::Vector4::yywx() const
{
	return Vector4(y, y, w, x);
}

bbe::Vector4 bbe::Vector4::yywy() const
{
	return Vector4(y, y, w, y);
}

bbe::Vector4 bbe::Vector4::yywz() const
{
	return Vector4(y, y, w, z);
}

bbe::Vector4 bbe::Vector4::yyww() const
{
	return Vector4(y, y, w, w);
}

bbe::Vector4 bbe::Vector4::yzxx() const
{
	return Vector4(y, z, x, x);
}

bbe::Vector4 bbe::Vector4::yzxy() const
{
	return Vector4(y, z, x, y);
}

bbe::Vector4 bbe::Vector4::yzxz() const
{
	return Vector4(y, z, x, z);
}

bbe::Vector4 bbe::Vector4::yzxw() const
{
	return Vector4(y, z, x, w);
}

bbe::Vector4 bbe::Vector4::yzyx() const
{
	return Vector4(y, z, y, x);
}

bbe::Vector4 bbe::Vector4::yzyy() const
{
	return Vector4(y, z, y, y);
}

bbe::Vector4 bbe::Vector4::yzyz() const
{
	return Vector4(y, z, y, z);
}

bbe::Vector4 bbe::Vector4::yzyw() const
{
	return Vector4(y, z, y, w);
}

bbe::Vector4 bbe::Vector4::yzzx() const
{
	return Vector4(y, z, z, x);
}

bbe::Vector4 bbe::Vector4::yzzy() const
{
	return Vector4(y, z, z, y);
}

bbe::Vector4 bbe::Vector4::yzzz() const
{
	return Vector4(y, z, z, z);
}

bbe::Vector4 bbe::Vector4::yzzw() const
{
	return Vector4(y, z, z, w);
}

bbe::Vector4 bbe::Vector4::yzwx() const
{
	return Vector4(y, z, w, x);
}

bbe::Vector4 bbe::Vector4::yzwy() const
{
	return Vector4(y, z, w, y);
}

bbe::Vector4 bbe::Vector4::yzwz() const
{
	return Vector4(y, z, w, z);
}

bbe::Vector4 bbe::Vector4::yzww() const
{
	return Vector4(y, z, w, w);
}

bbe::Vector4 bbe::Vector4::ywxx() const
{
	return Vector4(y, w, x, x);
}

bbe::Vector4 bbe::Vector4::ywxy() const
{
	return Vector4(y, w, x, y);
}

bbe::Vector4 bbe::Vector4::ywxz() const
{
	return Vector4(y, w, x, z);
}

bbe::Vector4 bbe::Vector4::ywxw() const
{
	return Vector4(y, w, x, w);
}

bbe::Vector4 bbe::Vector4::ywyx() const
{
	return Vector4(y, w, y, x);
}

bbe::Vector4 bbe::Vector4::ywyy() const
{
	return Vector4(y, w, y, y);
}

bbe::Vector4 bbe::Vector4::ywyz() const
{
	return Vector4(y, w, y, z);
}

bbe::Vector4 bbe::Vector4::ywyw() const
{
	return Vector4(y, w, y, w);
}

bbe::Vector4 bbe::Vector4::ywzx() const
{
	return Vector4(y, w, z, x);
}

bbe::Vector4 bbe::Vector4::ywzy() const
{
	return Vector4(y, w, z, y);
}

bbe::Vector4 bbe::Vector4::ywzz() const
{
	return Vector4(y, w, z, z);
}

bbe::Vector4 bbe::Vector4::ywzw() const
{
	return Vector4(y, w, z, w);
}

bbe::Vector4 bbe::Vector4::ywwx() const
{
	return Vector4(y, w, w, x);
}

bbe::Vector4 bbe::Vector4::ywwy() const
{
	return Vector4(y, w, w, y);
}

bbe::Vector4 bbe::Vector4::ywwz() const
{
	return Vector4(y, w, w, z);
}

bbe::Vector4 bbe::Vector4::ywww() const
{
	return Vector4(y, w, w, w);
}

bbe::Vector4 bbe::Vector4::zxxx() const
{
	return Vector4(z, x, x, x);
}

bbe::Vector4 bbe::Vector4::zxxy() const
{
	return Vector4(z, x, x, y);
}

bbe::Vector4 bbe::Vector4::zxxz() const
{
	return Vector4(z, x, x, z);
}

bbe::Vector4 bbe::Vector4::zxxw() const
{
	return Vector4(z, x, x, w);
}

bbe::Vector4 bbe::Vector4::zxyx() const
{
	return Vector4(z, x, y, x);
}

bbe::Vector4 bbe::Vector4::zxyy() const
{
	return Vector4(z, x, y, y);
}

bbe::Vector4 bbe::Vector4::zxyz() const
{
	return Vector4(z, x, y, z);
}

bbe::Vector4 bbe::Vector4::zxyw() const
{
	return Vector4(z, x, y, w);
}

bbe::Vector4 bbe::Vector4::zxzx() const
{
	return Vector4(z, x, z, x);
}

bbe::Vector4 bbe::Vector4::zxzy() const
{
	return Vector4(z, x, z, y);
}

bbe::Vector4 bbe::Vector4::zxzz() const
{
	return Vector4(z, x, z, z);
}

bbe::Vector4 bbe::Vector4::zxzw() const
{
	return Vector4(z, x, z, w);
}

bbe::Vector4 bbe::Vector4::zxwx() const
{
	return Vector4(z, x, w, x);
}

bbe::Vector4 bbe::Vector4::zxwy() const
{
	return Vector4(z, x, w, y);
}

bbe::Vector4 bbe::Vector4::zxwz() const
{
	return Vector4(z, x, w, z);
}

bbe::Vector4 bbe::Vector4::zxww() const
{
	return Vector4(z, x, w, w);
}

bbe::Vector4 bbe::Vector4::zyxx() const
{
	return Vector4(z, y, x, x);
}

bbe::Vector4 bbe::Vector4::zyxy() const
{
	return Vector4(z, y, x, y);
}

bbe::Vector4 bbe::Vector4::zyxz() const
{
	return Vector4(z, y, x, z);
}

bbe::Vector4 bbe::Vector4::zyxw() const
{
	return Vector4(z, y, x, w);
}

bbe::Vector4 bbe::Vector4::zyyx() const
{
	return Vector4(z, y, y, x);
}

bbe::Vector4 bbe::Vector4::zyyy() const
{
	return Vector4(z, y, y, y);
}

bbe::Vector4 bbe::Vector4::zyyz() const
{
	return Vector4(z, y, y, z);
}

bbe::Vector4 bbe::Vector4::zyyw() const
{
	return Vector4(z, y, y, w);
}

bbe::Vector4 bbe::Vector4::zyzx() const
{
	return Vector4(z, y, z, x);
}

bbe::Vector4 bbe::Vector4::zyzy() const
{
	return Vector4(z, y, z, y);
}

bbe::Vector4 bbe::Vector4::zyzz() const
{
	return Vector4(z, y, z, z);
}

bbe::Vector4 bbe::Vector4::zyzw() const
{
	return Vector4(z, y, z, w);
}

bbe::Vector4 bbe::Vector4::zywx() const
{
	return Vector4(z, y, w, x);
}

bbe::Vector4 bbe::Vector4::zywy() const
{
	return Vector4(z, y, w, y);
}

bbe::Vector4 bbe::Vector4::zywz() const
{
	return Vector4(z, y, w, z);
}

bbe::Vector4 bbe::Vector4::zyww() const
{
	return Vector4(z, y, w, w);
}

bbe::Vector4 bbe::Vector4::zzxx() const
{
	return Vector4(z, z, x, x);
}

bbe::Vector4 bbe::Vector4::zzxy() const
{
	return Vector4(z, z, x, y);
}

bbe::Vector4 bbe::Vector4::zzxz() const
{
	return Vector4(z, z, x, z);
}

bbe::Vector4 bbe::Vector4::zzxw() const
{
	return Vector4(z, z, x, w);
}

bbe::Vector4 bbe::Vector4::zzyx() const
{
	return Vector4(z, z, y, x);
}

bbe::Vector4 bbe::Vector4::zzyy() const
{
	return Vector4(z, z, y, y);
}

bbe::Vector4 bbe::Vector4::zzyz() const
{
	return Vector4(z, z, y, z);
}

bbe::Vector4 bbe::Vector4::zzyw() const
{
	return Vector4(z, z, y, w);
}

bbe::Vector4 bbe::Vector4::zzzx() const
{
	return Vector4(z, z, z, x);
}

bbe::Vector4 bbe::Vector4::zzzy() const
{
	return Vector4(z, z, z, y);
}

bbe::Vector4 bbe::Vector4::zzzz() const
{
	return Vector4(z, z, z, z);
}

bbe::Vector4 bbe::Vector4::zzzw() const
{
	return Vector4(z, z, z, w);
}

bbe::Vector4 bbe::Vector4::zzwx() const
{
	return Vector4(z, z, w, x);
}

bbe::Vector4 bbe::Vector4::zzwy() const
{
	return Vector4(z, z, w, y);
}

bbe::Vector4 bbe::Vector4::zzwz() const
{
	return Vector4(z, z, w, z);
}

bbe::Vector4 bbe::Vector4::zzww() const
{
	return Vector4(z, z, w, w);
}

bbe::Vector4 bbe::Vector4::zwxx() const
{
	return Vector4(z, w, x, x);
}

bbe::Vector4 bbe::Vector4::zwxy() const
{
	return Vector4(z, w, x, y);
}

bbe::Vector4 bbe::Vector4::zwxz() const
{
	return Vector4(z, w, x, z);
}

bbe::Vector4 bbe::Vector4::zwxw() const
{
	return Vector4(z, w, x, w);
}

bbe::Vector4 bbe::Vector4::zwyx() const
{
	return Vector4(z, w, y, x);
}

bbe::Vector4 bbe::Vector4::zwyy() const
{
	return Vector4(z, w, y, y);
}

bbe::Vector4 bbe::Vector4::zwyz() const
{
	return Vector4(z, w, y, z);
}

bbe::Vector4 bbe::Vector4::zwyw() const
{
	return Vector4(z, w, y, w);
}

bbe::Vector4 bbe::Vector4::zwzx() const
{
	return Vector4(z, w, z, x);
}

bbe::Vector4 bbe::Vector4::zwzy() const
{
	return Vector4(z, w, z, y);
}

bbe::Vector4 bbe::Vector4::zwzz() const
{
	return Vector4(z, w, z, z);
}

bbe::Vector4 bbe::Vector4::zwzw() const
{
	return Vector4(z, w, z, w);
}

bbe::Vector4 bbe::Vector4::zwwx() const
{
	return Vector4(z, w, w, x);
}

bbe::Vector4 bbe::Vector4::zwwy() const
{
	return Vector4(z, w, w, y);
}

bbe::Vector4 bbe::Vector4::zwwz() const
{
	return Vector4(z, w, w, z);
}

bbe::Vector4 bbe::Vector4::zwww() const
{
	return Vector4(z, w, w, w);
}

bbe::Vector4 bbe::Vector4::wxxx() const
{
	return Vector4(w, x, x, x);
}

bbe::Vector4 bbe::Vector4::wxxy() const
{
	return Vector4(w, x, x, y);
}

bbe::Vector4 bbe::Vector4::wxxz() const
{
	return Vector4(w, x, x, z);
}

bbe::Vector4 bbe::Vector4::wxxw() const
{
	return Vector4(w, x, x, w);
}

bbe::Vector4 bbe::Vector4::wxyx() const
{
	return Vector4(w, x, y, x);
}

bbe::Vector4 bbe::Vector4::wxyy() const
{
	return Vector4(w, x, y, y);
}

bbe::Vector4 bbe::Vector4::wxyz() const
{
	return Vector4(w, x, y, z);
}

bbe::Vector4 bbe::Vector4::wxyw() const
{
	return Vector4(w, x, y, w);
}

bbe::Vector4 bbe::Vector4::wxzx() const
{
	return Vector4(w, x, z, x);
}

bbe::Vector4 bbe::Vector4::wxzy() const
{
	return Vector4(w, x, z, y);
}

bbe::Vector4 bbe::Vector4::wxzz() const
{
	return Vector4(w, x, z, z);
}

bbe::Vector4 bbe::Vector4::wxzw() const
{
	return Vector4(w, x, z, w);
}

bbe::Vector4 bbe::Vector4::wxwx() const
{
	return Vector4(w, x, w, x);
}

bbe::Vector4 bbe::Vector4::wxwy() const
{
	return Vector4(w, x, w, y);
}

bbe::Vector4 bbe::Vector4::wxwz() const
{
	return Vector4(w, x, w, z);
}

bbe::Vector4 bbe::Vector4::wxww() const
{
	return Vector4(w, x, w, w);
}

bbe::Vector4 bbe::Vector4::wyxx() const
{
	return Vector4(w, y, x, x);
}

bbe::Vector4 bbe::Vector4::wyxy() const
{
	return Vector4(w, y, x, y);
}

bbe::Vector4 bbe::Vector4::wyxz() const
{
	return Vector4(w, y, x, z);
}

bbe::Vector4 bbe::Vector4::wyxw() const
{
	return Vector4(w, y, x, w);
}

bbe::Vector4 bbe::Vector4::wyyx() const
{
	return Vector4(w, y, y, x);
}

bbe::Vector4 bbe::Vector4::wyyy() const
{
	return Vector4(w, y, y, y);
}

bbe::Vector4 bbe::Vector4::wyyz() const
{
	return Vector4(w, y, y, z);
}

bbe::Vector4 bbe::Vector4::wyyw() const
{
	return Vector4(w, y, y, w);
}

bbe::Vector4 bbe::Vector4::wyzx() const
{
	return Vector4(w, y, z, x);
}

bbe::Vector4 bbe::Vector4::wyzy() const
{
	return Vector4(w, y, z, y);
}

bbe::Vector4 bbe::Vector4::wyzz() const
{
	return Vector4(w, y, z, z);
}

bbe::Vector4 bbe::Vector4::wyzw() const
{
	return Vector4(w, y, z, w);
}

bbe::Vector4 bbe::Vector4::wywx() const
{
	return Vector4(w, y, w, x);
}

bbe::Vector4 bbe::Vector4::wywy() const
{
	return Vector4(w, y, w, y);
}

bbe::Vector4 bbe::Vector4::wywz() const
{
	return Vector4(w, y, w, z);
}

bbe::Vector4 bbe::Vector4::wyww() const
{
	return Vector4(w, y, w, w);
}

bbe::Vector4 bbe::Vector4::wzxx() const
{
	return Vector4(w, z, x, x);
}

bbe::Vector4 bbe::Vector4::wzxy() const
{
	return Vector4(w, z, x, y);
}

bbe::Vector4 bbe::Vector4::wzxz() const
{
	return Vector4(w, z, x, z);
}

bbe::Vector4 bbe::Vector4::wzxw() const
{
	return Vector4(w, z, x, w);
}

bbe::Vector4 bbe::Vector4::wzyx() const
{
	return Vector4(w, z, y, x);
}

bbe::Vector4 bbe::Vector4::wzyy() const
{
	return Vector4(w, z, y, y);
}

bbe::Vector4 bbe::Vector4::wzyz() const
{
	return Vector4(w, z, y, z);
}

bbe::Vector4 bbe::Vector4::wzyw() const
{
	return Vector4(w, z, y, w);
}

bbe::Vector4 bbe::Vector4::wzzx() const
{
	return Vector4(w, z, z, x);
}

bbe::Vector4 bbe::Vector4::wzzy() const
{
	return Vector4(w, z, z, y);
}

bbe::Vector4 bbe::Vector4::wzzz() const
{
	return Vector4(w, z, z, z);
}

bbe::Vector4 bbe::Vector4::wzzw() const
{
	return Vector4(w, z, z, w);
}

bbe::Vector4 bbe::Vector4::wzwx() const
{
	return Vector4(w, z, w, x);
}

bbe::Vector4 bbe::Vector4::wzwy() const
{
	return Vector4(w, z, w, y);
}

bbe::Vector4 bbe::Vector4::wzwz() const
{
	return Vector4(w, z, w, z);
}

bbe::Vector4 bbe::Vector4::wzww() const
{
	return Vector4(w, z, w, w);
}

bbe::Vector4 bbe::Vector4::wwxx() const
{
	return Vector4(w, w, x, x);
}

bbe::Vector4 bbe::Vector4::wwxy() const
{
	return Vector4(w, w, x, y);
}

bbe::Vector4 bbe::Vector4::wwxz() const
{
	return Vector4(w, w, x, z);
}

bbe::Vector4 bbe::Vector4::wwxw() const
{
	return Vector4(w, w, x, w);
}

bbe::Vector4 bbe::Vector4::wwyx() const
{
	return Vector4(w, w, y, x);
}

bbe::Vector4 bbe::Vector4::wwyy() const
{
	return Vector4(w, w, y, y);
}

bbe::Vector4 bbe::Vector4::wwyz() const
{
	return Vector4(w, w, y, z);
}

bbe::Vector4 bbe::Vector4::wwyw() const
{
	return Vector4(w, w, y, w);
}

bbe::Vector4 bbe::Vector4::wwzx() const
{
	return Vector4(w, w, z, x);
}

bbe::Vector4 bbe::Vector4::wwzy() const
{
	return Vector4(w, w, z, y);
}

bbe::Vector4 bbe::Vector4::wwzz() const
{
	return Vector4(w, w, z, z);
}

bbe::Vector4 bbe::Vector4::wwzw() const
{
	return Vector4(w, w, z, w);
}

bbe::Vector4 bbe::Vector4::wwwx() const
{
	return Vector4(w, w, w, x);
}

bbe::Vector4 bbe::Vector4::wwwy() const
{
	return Vector4(w, w, w, y);
}

bbe::Vector4 bbe::Vector4::wwwz() const
{
	return Vector4(w, w, w, z);
}

bbe::Vector4 bbe::Vector4::wwww() const
{
	return Vector4(w, w, w, w);
}
