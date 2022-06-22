#ifndef TSR_MACROS
#define TSR_MACROS

#define red			{1.0f, 0.0f, 0.0f, 1.0f}
#define green		{0.0f, 1.0f, 0.0f, 1.0f}
#define blue		{0.0f, 0.0f, 1.0f, 1.0f}
#define magenta		{1.0f, 0.0f, 1.0f, 1.0f}
#define yellow		{1.0f, 1.0f, 0.0f, 1.0f}
#define cyam		{0.0f, 1.0f, 1.0f, 1.0f}
#define black		{0.0f, 0.0f, 0.0f, 1.0f}
#define white		{1.0f, 1.0f, 1.0f, 1.0f}

#define PTRCAST(TYPE,POINTER)	reinterpret_cast<TYPE>(POINTER)
#define TYPECAST(TYPE,VALUE)	static_cast<TYPE>(VALUE)

// LOGGER

enum class LOGTYPE {
	LOG_DEBUG,
	LOG_ERROR,
	LOG_WARNING,
	COUNT
};

#define MESSAGE(VALUE) eastl::string(VALUE).c_str()

#define LOGTYPE_ERROR		"[ERROR]"
#define LOGTYPE_WARNING		"[WARNING]"
#define LOGTYPE_DEBUG		"[DEBUG]"

#define LOGSYSTEM_TSR		"[TSR ENGINE]:"
#define LOGSYSTEM_EASTL		"[EASTL]:"
#define LOGSYSTEM_DX11		"[DX11]:"
#define LOGSYSTEM_ASSIMP	"[ASSIMP]:"

#define LOGERROR(SYSTEM, MESSAGE) printf("\033[0;31m%-15s%-15s%s\033[0m\n",LOGTYPE_ERROR,SYSTEM,MESSAGE)
#define LOGWARNING(SYSTEM, MESSAGE) printf("\033[0;33m%-15s%-15s%s\033[0m\n",LOGTYPE_WARNING,SYSTEM,MESSAGE)
#define LOGDEBUG(SYSTEM, MESSAGE) printf("%-15s%-15s%s\n",LOGTYPE_DEBUG,SYSTEM,MESSAGE)

#define LOG(TYPE, SYSTEM, MESSAGE)	switch(TYPE){\
case LOGTYPE::LOG_ERROR: LOGERROR(SYSTEM, MESSAGE);break;\
case LOGTYPE::LOG_DEBUG: LOGDEBUG(SYSTEM, MESSAGE);break;\
case LOGTYPE::LOG_WARNING: LOGWARNING(SYSTEM, MESSAGE);break;\
default: LOGDEBUG(SYSTEM,MESSAGE);break;}

//NOTE(Fran): see if the if can be removed
#define LOGASSERT(SYSTEM, MESSAGE, EXPRESSION)	if(!(EXPRESSION)){ LOGERROR(SYSTEM, MESSAGE); abort();}
#define LOGCHECK(SYSTEM, MESSAGE, EXPRESSION)	if(!(EXPRESSION)){ LOGWARNING(SYSTEM, MESSAGE);}

// MATH

#define POW(VALUE) (TYPECAST(r64,VALUE) * TYPECAST(r64,VALUE))

inline r32 V3LEN(DirectX::XMFLOAT3 f)
{
	r64 R = sqrt(POW(f.x) + POW(f.y) + POW(f.z));
	return TYPECAST(r32, R);
}

//check the *= operator

inline DirectX::XMFLOAT3 operator*(const r32& s, const DirectX::XMFLOAT3& fv)
{
	DirectX::XMFLOAT3 R;
	R.x = fv.x * s;
	R.y = fv.y * s;
	R.z = fv.z * s;
	return R;
}

inline DirectX::XMFLOAT3 operator*(const DirectX::XMFLOAT3& fv, const r32& s)
{
	DirectX::XMFLOAT3 R;
	R.x = fv.x * s;
	R.y = fv.y * s;
	R.z = fv.z * s;
	return R;
}

inline DirectX::XMFLOAT3 operator/(const DirectX::XMFLOAT3 fv, const r32 d)
{
	DirectX::XMFLOAT3 R;
	r32 s = 1.0f / d;
	R = R * s;
	return R;
}

//NOTE(Fran): once I've got plenty of math helpers I think it might be wise to move them elsewhere
// Also, I would like to measure the speed of this, as we have a sqrt over here and when we calculate vector lengths we usually want to calculate them
// in bulk, so we could take advantage of SIMD instructions, I could have a VECLENBULK and VECLEN to support multiple vector amounts
inline DirectX::XMFLOAT3 TSR_DX_NormalizeFLOAT3(DirectX::XMFLOAT3 f)
{
	DirectX::XMFLOAT3 R;
	r32 v3len = V3LEN(f);
	R = f / v3len;
	return R;
}



#endif
