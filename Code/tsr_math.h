#ifndef TSR_MATH
#define TSR_MATH
// MATH

#define POW(VALUE) (TYPECAST(r64,VALUE) * TYPECAST(r64,VALUE))
#define MIN(VAL, VAL_) ((VAL > VAL_) ? VAL_ : VAL)
#define MAX(VAL, VAL_) ((VAL < VAL_) ? VAL_ : VAL)
#define CLAMP(VAL, MIN_, MAX_) MIN(MAX_, MAX(MIN_, VAL))

inline DirectX::XMFLOAT2 ToFloat2(DirectX::XMVECTOR v)
{
	DirectX::XMFLOAT2 R = {};
	DirectX::XMStoreFloat2(&R, v);
	return R;
}

inline DirectX::XMFLOAT3 ToFloat3(DirectX::XMVECTOR v)
{
	DirectX::XMFLOAT3 R = {};
	DirectX::XMStoreFloat3(&R, v);
	return R;
}

inline DirectX::XMFLOAT4 ToFloat4(DirectX::XMVECTOR v)
{
	DirectX::XMFLOAT4 R = {};
	DirectX::XMStoreFloat4(&R, v);
	return R;
}

inline r32 V2LEN(DirectX::XMFLOAT2 f)
{
	r64 R = sqrt(POW(f.x) + POW(f.y));
	return TYPECAST(r32, R);
}

inline r32 V3LEN(DirectX::XMFLOAT3 f)
{
	r64 R = sqrt(POW(f.x) + POW(f.y) + POW(f.z));
	return TYPECAST(r32, R);
}

inline r32 V4LEN(DirectX::XMFLOAT4 f)
{
	r64 R = sqrt(POW(f.x) + POW(f.y) + POW(f.z) + POW(f.w));
	return TYPECAST(r32, R);
}

//check the *= operator

inline DirectX::XMFLOAT2 operator*(const r32& s, const DirectX::XMFLOAT2& fv)
{
	DirectX::XMFLOAT2 R;
	R.x = fv.x * s;
	R.y = fv.y * s;
	return R;
}

inline DirectX::XMFLOAT3 operator*(const r32& s, const DirectX::XMFLOAT3& fv)
{
	DirectX::XMFLOAT3 R;
	R.x = fv.x * s;
	R.y = fv.y * s;
	R.z = fv.z * s;
	return R;
}

inline DirectX::XMFLOAT4 operator*(const r32& s, const DirectX::XMFLOAT4& fv)
{
	DirectX::XMFLOAT4 R;
	R.x = fv.x * s;
	R.y = fv.y * s;
	R.z = fv.z * s;
	R.w = fv.w * s;
	return R;
}

inline DirectX::XMFLOAT2 operator*(const DirectX::XMFLOAT2& fv, const r32& s)
{
	return s * fv;
}

inline DirectX::XMFLOAT3 operator*(const DirectX::XMFLOAT3& fv, const r32& s)
{
	return s * fv;
}

inline DirectX::XMFLOAT4 operator*(const DirectX::XMFLOAT4& fv, const r32& s)
{
	return s * fv;
}

inline DirectX::XMFLOAT2 operator/(const DirectX::XMFLOAT2 fv, const r32 d)
{
	DirectX::XMFLOAT2 R;
	r32 s = 1.0f / d;
	R = fv * s;
	return R;
}

inline DirectX::XMFLOAT3 operator/(const DirectX::XMFLOAT3 fv, const r32 d)
{
	DirectX::XMFLOAT3 R;
	r32 s = 1.0f / d;
	R = fv * s;
	return R;
}

inline DirectX::XMFLOAT4 operator/(const DirectX::XMFLOAT4 fv, const r32 d)
{
	DirectX::XMFLOAT4 R;
	r32 s = 1.0f / d;
	R = fv * s;
	return R;
}

inline DirectX::XMFLOAT2 TSR_DX_NormalizeFLOAT2(DirectX::XMFLOAT2 f)
{
	DirectX::XMFLOAT2 R;
	r32 v4len = V2LEN(f);
	R = f / v4len;
	return R;
}

inline DirectX::XMFLOAT3 TSR_DX_NormalizeFLOAT3(DirectX::XMFLOAT3 f)
{
	DirectX::XMFLOAT3 R;
	r32 v4len = V3LEN(f);
	R = f / v4len;
	return R;
}

inline DirectX::XMFLOAT4 TSR_DX_NormalizeFLOAT4(DirectX::XMFLOAT4 f)
{
	DirectX::XMFLOAT4 R;
	r32 v4len = V4LEN(f);
	R = f / v4len;
	return R;
}

inline DirectX::XMFLOAT2 operator+(const DirectX::XMFLOAT2& f1, const DirectX::XMFLOAT2 f2)
{
	DirectX::XMFLOAT2 R = { f1.x + f2.x, f1.y + f2.y};
	return R;
}

inline DirectX::XMFLOAT3 operator+(const DirectX::XMFLOAT3 &f1, const DirectX::XMFLOAT3 f2)
{
	DirectX::XMFLOAT3 R = { f1.x + f2.x, f1.y + f2.y, f1.z + f2.z };
	return R;
}

inline DirectX::XMFLOAT4 operator+(const DirectX::XMFLOAT4& f1, const DirectX::XMFLOAT4 f2)
{
	DirectX::XMFLOAT4 R = { f1.x + f2.x, f1.y + f2.y, f1.z + f2.z, f1.w + f2.w };
	return R;
}

inline void operator+=(DirectX::XMFLOAT2& f1, const DirectX::XMFLOAT2 f2)
{
	f1 = (f1 + f2);
}

inline void operator+=(DirectX::XMFLOAT3& f1, const DirectX::XMFLOAT3 f2)
{
	f1 = (f1 + f2);
}

inline void operator+=(DirectX::XMFLOAT4& f1, const DirectX::XMFLOAT4 f2)
{
	f1 = (f1 + f2);
}

inline DirectX::XMFLOAT2 operator-(const DirectX::XMFLOAT2& f1, const DirectX::XMFLOAT2 f2)
{
	DirectX::XMFLOAT2 R = { f1.x - f2.x, f1.y - f2.y };
	return R;
}

inline DirectX::XMFLOAT3 operator-(const DirectX::XMFLOAT3& f1, const DirectX::XMFLOAT3 f2)
{
	DirectX::XMFLOAT3 R = { f1.x - f2.x, f1.y - f2.y, f1.z - f2.z };
	return R;
}

inline DirectX::XMFLOAT4 operator-(const DirectX::XMFLOAT4& f1, const DirectX::XMFLOAT4 f2)
{
	DirectX::XMFLOAT4 R = { f1.x - f2.x, f1.y - f2.y, f1.z - f2.z, f1.w - f2.w };
	return R;
}

inline void operator-=(DirectX::XMFLOAT2& f1, const DirectX::XMFLOAT2 f2)
{
	f1 = (f1 - f2);
}

inline void operator-=(DirectX::XMFLOAT3& f1, const DirectX::XMFLOAT3 f2)
{
	f1 = (f1 - f2);
}

inline void operator-=(DirectX::XMFLOAT4& f1, const DirectX::XMFLOAT4 f2)
{
	f1 = (f1 - f2);
}

#endif // TSR_MATH
