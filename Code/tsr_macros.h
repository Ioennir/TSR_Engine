#ifndef TSR_MACROS
#define TSR_MACROS
#include <DirectXMath.h>

DirectX::XMFLOAT4 red{ 1.0f, 0.0f, 0.0f, 1.0f };
DirectX::XMFLOAT4 green{ 0.0f, 1.0f, 0.0f, 1.0f };
DirectX::XMFLOAT4 blue{ 0.0f, 0.0f, 1.0f, 1.0f };
DirectX::XMFLOAT4 magenta{ 1.0f, 0.0f, 1.0f, 1.0f };
DirectX::XMFLOAT4 yellow{ 1.0f, 1.0f, 0.0f, 1.0f };
DirectX::XMFLOAT4 cyan{ 0.0f, 1.0f, 1.0f, 1.0f };
DirectX::XMFLOAT4 black{ 0.0f, 0.0f, 0.0f, 1.0f };
DirectX::XMFLOAT4 white{ 1.0f, 1.0f, 1.0f, 1.0f };

#define LOGTYPE_ERROR		"[ERROR]"
#define LOGTYPE_WARNING		"[WARNING]"
#define LOGTYPE_DEBUG		"[DEBUG]"
#define LOGTYPE_TSR			"[TSR Engine]"
#define LOGTYPE_EASTL		"[EASTL]"
#define LOGTYPE_DX11		"[DX11]"
#define LOG(TYPE, MESSAGE)	printf("%s: %s\n",TYPE,MESSAGE)

#endif
