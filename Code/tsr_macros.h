#ifndef TSR_MACROS
#define TSR_MACROS

DirectX::XMFLOAT4 red{ 1.0f, 0.0f, 0.0f, 1.0f };
DirectX::XMFLOAT4 green{ 0.0f, 1.0f, 0.0f, 1.0f };
DirectX::XMFLOAT4 blue{ 0.0f, 0.0f, 1.0f, 1.0f };
DirectX::XMFLOAT4 magenta{ 1.0f, 0.0f, 1.0f, 1.0f };
DirectX::XMFLOAT4 yellow{ 1.0f, 1.0f, 0.0f, 1.0f };
DirectX::XMFLOAT4 cyan{ 0.0f, 1.0f, 1.0f, 1.0f };
DirectX::XMFLOAT4 black{ 0.0f, 0.0f, 0.0f, 1.0f };
DirectX::XMFLOAT4 white{ 1.0f, 1.0f, 1.0f, 1.0f };

#define PTRCAST(TYPE,POINTER)	reinterpret_cast<TYPE>(POINTER)
#define TYPECAST(TYPE,VALUE)	static_cast<TYPE>(VALUE)

enum class LOGTYPE {
	LOG_DEBUG,
	LOG_ERROR,
	LOG_WARNING,
	COUNT
};

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
#define LOGCHECK(SYSTEM, MESSAGE, EXPRESSION)	if(!(EXPRESSION)){ LOGERROR(SYSTEM, MESSAGE);}

#endif
