#ifndef TSR_MACROS
#define TSR_MACROS

#define TSR_Red			{1.0f, 0.0f, 0.0f, 1.0f}
#define TSR_Green		{0.0f, 1.0f, 0.0f, 1.0f}
#define TSR_Blue		{0.0f, 0.0f, 1.0f, 1.0f}
#define TSR_Magenta		{1.0f, 0.0f, 1.0f, 1.0f}
#define TSR_Yellow		{1.0f, 1.0f, 0.0f, 1.0f}
#define TSR_Cyan		{0.0f, 1.0f, 1.0f, 1.0f}
#define TSR_Black		{0.0f, 0.0f, 0.0f, 1.0f}
#define TSR_White		{1.0f, 1.0f, 1.0f, 1.0f}

#define PTRCAST(TYPE,POINTER)	reinterpret_cast<TYPE>(POINTER)
#define TYPECAST(TYPE,VALUE)	static_cast<TYPE>(VALUE)

// LOGGER

enum class LOGTYPE {
	LOG_DEBUG,
	LOG_ERROR,
	LOG_WARNING,
	COUNT
};

#define STR(VALUE) eastl::to_string(VALUE)
#define TEXTMESSAGE(VALUE) eastl::string(VALUE).c_str()

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

#endif //TSR_MACROS
