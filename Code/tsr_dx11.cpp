/*
	This .cpp contains all functions to interact with D3D11.
*/

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXPackedVector.h>

#define NO_VSYNC 0
#define VSYNC 1

//These structs are as simple as possible for now to perform
// a basic 3D projection and get things going.
struct CameraData
{
	DirectX::XMMATRIX mWorld;
	DirectX::XMMATRIX mView;
	DirectX::XMMATRIX mProj;
};

struct ConstantBuffer
{
	DirectX::XMMATRIX mWorld;
	DirectX::XMMATRIX mVWP;
	DirectX::XMMATRIX normalMatrix;
};

struct Mesh
{
	eastl::vector<DirectX::XMFLOAT3> vertices;
	eastl::vector<ui32> indices;
};

struct Vertex
{
	DirectX::XMFLOAT3 Position{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4 Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 Normal{ 0.0f, 0.0f, 0.0f };
};

//TODO(Fran): recreate swapchain when window resizes...

struct DX11ViewportData 
{
	ID3D11Texture2D*			RenderTargetTexture;
	ID3D11Texture2D*			DepthStencilTexture;
	ID3D11RenderTargetView*		RenderTargetView;
	ID3D11DepthStencilView*		DepthStencilView;
	ID3D11DepthStencilState*	DepthStencilState;
	ID3D11ShaderResourceView*	ShaderResourceView;
	D3D11_VIEWPORT				Viewport{};
};

struct DX11Data
{
	ID3D11Device*				device{};
	ID3D11DeviceContext*		context{};
	D3D_FEATURE_LEVEL*			featureLevel{};
	IDXGISwapChain*				swapChain{};
	ID3D11Texture2D*			depthStencilBuffer{};
	ID3D11RenderTargetView*		renderTargetView{};
	ID3D11DepthStencilView*		depthStencilView{};
	ID3D11RasterizerState*		currentRasterizerState{};
	DX11ViewportData			VP;
	ID3D11Buffer*				dx11_cbuffer{}; // this will be moved elsewhere
};

struct DX11VertexShaderData
{
	ID3DBlob* shaderBuffer{};
	ID3D11VertexShader* shader{};
	ID3D11InputLayout* inputLayout{};
};

struct DX11PixelShaderData
{
	ID3DBlob* shaderBuffer{};
	ID3D11PixelShader* shader{};

};

struct BufferData
{
	ID3D11Buffer* buffer{};
	UINT stride{ 0 };
	UINT offset{ 0 };
};

struct IMData
{
	r32 rot[3]{ 0.0f, 1.0f, 0.0f };
	r32 rotSpeed{ 60.0f };
};

//Globally accessible stuff related to DX11 should be under this namespace
namespace DX11 
{
	DX11Data dxData{};
}

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

// TODO(Fran): update this to do all the checks we need.
// Fetch displays and create the DX11 device
void TSR_DX11_CreateDeviceAndSwapChain(WindowData & winData, bool msaaOn, DX11Data * dxData)
{
	HRESULT hr;
	// Desired Feature level
	D3D_FEATURE_LEVEL fLevel = { D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0 };
	// Device Flags
	UINT createDeviceFlags = 0;
	
#ifdef _DEBUG
	createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

	DXGI_SWAP_CHAIN_DESC scDescriptor;
	scDescriptor.BufferDesc.Width = winData.width;
	scDescriptor.BufferDesc.Height = winData.height;
	// TODO(Fran): maybe pool displays and query refresh rate to get this exact
	scDescriptor.BufferDesc.RefreshRate.Numerator = 60;
	scDescriptor.BufferDesc.RefreshRate.Denominator = 1;
	scDescriptor.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDescriptor.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scDescriptor.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	if (msaaOn)
	{
	}
	else
	{
		scDescriptor.SampleDesc.Count = 1;
		scDescriptor.SampleDesc.Quality = 0;
	}
	scDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDescriptor.BufferCount = 1;
	//NOTE(Fran): This assumes you are using windows
	scDescriptor.OutputWindow = PTRCAST(HWND, winData.handle);
	scDescriptor.Windowed = true;
	scDescriptor.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scDescriptor.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	hr = D3D11CreateDeviceAndSwapChain(
		0,
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		createDeviceFlags,
		&fLevel,
		1,
		D3D11_SDK_VERSION,
		&scDescriptor,
		&dxData->swapChain,
		&dxData->device,
		dxData->featureLevel,
		&dxData->context
	);
	LOGASSERT(LOGSYSTEM_DX11, "Failed creating Device and/or SwapChain.", !FAILED(hr));
	LOG(LOGTYPE::LOG_DEBUG, LOGSYSTEM_DX11, "Device & Swapchain created!");
}

//DESCRIPTION: Acquires the DX11 BackBuffer and RT View
void TSR_DX11_CreateBackBufferAndRTView(DX11Data * dxData)
{
	HRESULT hr;
	ID3D11Texture2D* bBuffer;
	hr = dxData->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&bBuffer));
	LOGASSERT(LOGSYSTEM_DX11,"Failed acquiring the backbuffer",!FAILED(hr));
	hr = dxData->device->CreateRenderTargetView(bBuffer, 0, &dxData->renderTargetView);
	LOGASSERT(LOGSYSTEM_DX11, "Failed creating the RT View",!FAILED(hr));
	bBuffer->Release();
	LOG(LOGTYPE::LOG_DEBUG, LOGSYSTEM_DX11, "Backbuffer & RT View created!");
}

struct TextureInfo
{
	ui32 width;
	ui32 height;
	DXGI_FORMAT format;
	D3D11_USAGE usage;
	ui32 bindFlags;
};

HRESULT TSR_DX11_CreateTexture2D(TextureInfo & tInfo, ID3D11Device * device, ID3D11Texture2D ** textureHandle)
{
	HRESULT hr;
	D3D11_TEXTURE2D_DESC tDesc{ 0 };
	tDesc.Width = tInfo.width;
	tDesc.Height = tInfo.height;

	//NOTE(Fran): check this.
	tDesc.MipLevels = 1;
	tDesc.ArraySize = 1;
	tDesc.SampleDesc.Count = 1;
	tDesc.SampleDesc.Quality = 0;

	tDesc.Format = tInfo.format;
	tDesc.Usage = tInfo.usage;
	tDesc.BindFlags = tInfo.bindFlags;
	hr = device->CreateTexture2D(&tDesc, 0, textureHandle);
	LOGASSERT(LOGSYSTEM_DX11, "TextureHandle is nullptr.", (*textureHandle != nullptr));
	return hr;
}

void TSR_DX11_InitRenderTargetTexture(ui32 tWidth, ui32 tHeight, ID3D11Device * device, ID3D11Texture2D ** textureHandle)
{
	HRESULT hr;
	TextureInfo tInfo{};
	tInfo.width = tWidth;
	tInfo.height = tHeight;
	tInfo.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	tInfo.usage = D3D11_USAGE_DEFAULT;
	tInfo.bindFlags = (D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);
	hr = TSR_DX11_CreateTexture2D(tInfo, device, textureHandle);
	LOGASSERT(LOGSYSTEM_DX11, "Failed initializing RT Texture", !FAILED(hr));
	LOG(LOGTYPE::LOG_DEBUG, LOGSYSTEM_DX11, "RT Texture created!");
}

void TSR_DX11_InitDepthBuffer(ui32 tWidth, ui32 tHeight, ID3D11Device * device, ID3D11Texture2D ** textureHandle)
{
	HRESULT hr;
	TextureInfo tInfo{};
	tInfo.width = tWidth;
	tInfo.height = tHeight;
	tInfo.format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	tInfo.usage = D3D11_USAGE_DEFAULT;
	tInfo.bindFlags = (D3D11_BIND_DEPTH_STENCIL);
	hr = TSR_DX11_CreateTexture2D(tInfo, device, textureHandle);
	LOGASSERT(LOGSYSTEM_DX11, "Failed initializing Depth-Stencil Buffer Texture", !FAILED(hr));
	LOG(LOGTYPE::LOG_DEBUG, LOGSYSTEM_DX11, "Depth Stencil Texture created!");
}

HRESULT TSR_DX11_SetDepthBufferState(ID3D11Device* device, ID3D11DeviceContext* deviceCtx, DX11ViewportData* VP)
{
	HRESULT hr;
	D3D11_DEPTH_STENCIL_DESC dsDesc;
	// Depth test parameters
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	// Stencil test parameters
	dsDesc.StencilEnable = false;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;
	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	// Create depth stencil state
	hr = device->CreateDepthStencilState(&dsDesc, &VP->DepthStencilState);
	deviceCtx->OMSetDepthStencilState(VP->DepthStencilState, 1);
	return hr;
}

//TODO(Fran): now the format and dimension is fixed, this might change in the future, as well as the miplevels.
HRESULT TSR_DX11_CreateShaderResourceView(ID3D11Device* device, ID3D11Texture2D * texture, ID3D11ShaderResourceView ** shaderResourceView)
{
	HRESULT hr;
	// SHADER RESOURCE VIEW FOR THE RENDER TEXTURE
	D3D11_SHADER_RESOURCE_VIEW_DESC srDesc{};
	srDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srDesc.Texture2D.MostDetailedMip = 0;
	srDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(texture, &srDesc, shaderResourceView);
	return hr;
}
HRESULT TSR_DX11_CreateRenderTargetView(ID3D11Device* device, ID3D11Texture2D* texture, ID3D11RenderTargetView** renderTargetView)
{
	HRESULT hr;
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	hr = device->CreateRenderTargetView(texture, &rtvDesc, renderTargetView);
	return hr;
}

void TSR_DX11_InitViewport(ID3D11Device * device, ID3D11DeviceContext* deviceCtx, DX11ViewportData * VP)
{
	HRESULT hr;
	hr = device->CreateDepthStencilView(VP->DepthStencilTexture, 0, &VP->DepthStencilView);
	LOGASSERT(LOGSYSTEM_DX11, "Failed creating Depth View from Depth Stencil texture.", !FAILED(hr));
	hr = TSR_DX11_SetDepthBufferState(device, deviceCtx, VP);
	LOGASSERT(LOGSYSTEM_DX11, "Failed creating and setting the Depth Stencil State.", !FAILED(hr));
	hr = TSR_DX11_CreateRenderTargetView(device, VP->RenderTargetTexture, &VP->RenderTargetView);
	LOGASSERT(LOGSYSTEM_DX11, "Failed Creating the Render Target view.", !FAILED(hr));
	hr = TSR_DX11_CreateShaderResourceView(device, VP->RenderTargetTexture, &VP->ShaderResourceView);
	LOGASSERT(LOGSYSTEM_DX11, "Failed Creating the Shader Resource view for the viewport.", !FAILED(hr));
}

void TSR_DX11_SetGameViewport(ui32 vpWidth, ui32 vpHeight, DX11ViewportData * VP)
{
	VP->Viewport = { 0 };
	VP->Viewport.MinDepth = 0.0f;
	VP->Viewport.MaxDepth = 1.0f;
	VP->Viewport.TopLeftX = 0.0f;
	VP->Viewport.TopLeftY = 0.0f;
	VP->Viewport.Width = static_cast<float>(vpWidth);
	VP->Viewport.Height = static_cast<float>(vpHeight);
}

//TODO(Fran): check MSAA support later.
//TODO(Fran): Do the debug implementation with dxtrace etc.
//TODO(Fran): Check MSAA thingy.
void TSR_DX11_Init(WindowData & winData, DX11Data* dxData)
{
	UINT wWidth = winData.width;
	UINT wHeight = winData.height;
	UINT rtWidth = 640;
	UINT rtHeight = 360;
	bool msaaOn = false;

	//issue with the textures
	TSR_DX11_CreateDeviceAndSwapChain(winData, msaaOn, dxData);
	TSR_DX11_CreateBackBufferAndRTView(dxData);

	// Viewport render target initialization
	TSR_DX11_InitRenderTargetTexture(rtWidth, rtHeight, dxData->device, &dxData->VP.RenderTargetTexture);
	// Viewport depth buffer initialization
	TSR_DX11_InitDepthBuffer(rtWidth, rtHeight, dxData->device, &dxData->VP.DepthStencilTexture);
	TSR_DX11_InitViewport(dxData->device, dxData->context, &dxData->VP);
	TSR_DX11_SetGameViewport(rtWidth, rtHeight, &dxData->VP);
}

//TODO(Fran): Maybe implement a template here
//TODO(Fran): Maybe implement a way of knowing what kind of buffer we are creating to ease debugging if it fails. (maybe with the bindflag)
void TSR_DX11_BuildBuffer(ID3D11Device * device, ui32 bStride, ui32 bOffset, ui32 bElementCount, D3D11_USAGE bUsage, UINT bBindFlags, void * bMemoryPtr, BufferData * buffer)
{
	buffer->stride = bStride;
	buffer->offset = bOffset;
	D3D11_BUFFER_DESC bDesc{ 0 };
	bDesc.Usage = bUsage;
	bDesc.BindFlags = bBindFlags;
	bDesc.ByteWidth = bStride * bElementCount;

	D3D11_SUBRESOURCE_DATA bData{ 0 };
	bData.pSysMem = bMemoryPtr;

	HRESULT hr = device->CreateBuffer(&bDesc, &bData, &buffer->buffer);
	LOGASSERT(LOGSYSTEM_DX11, "Buffer creation failed.", !FAILED(hr));
	LOGDEBUG(LOGSYSTEM_DX11, "Buffer creation succeeded.");
}

struct ModelData
{
	eastl::vector<DirectX::XMFLOAT3>	totalVertices;
	eastl::vector<DirectX::XMFLOAT3>	normals;
	eastl::vector<DirectX::XMFLOAT2>	texCoords;
	eastl::vector<ui32>					totalIndices;
	eastl::vector<ui32>					submeshStartIndex;
	eastl::vector<ui32>					submeshEndIndex;
	eastl::vector<ui32>					submeshTexcoordStart;
	eastl::vector<ui32>					submeshTexcoordEnd;
	eastl::vector<ui32>					submeshMaterialIndex;
	ui32								submeshCount;
	ui32								vertexCount;
	ui32								indexCount;
	eastl::string						name;
};

struct DrawComponent
{
	ModelData model;
	eastl::vector<Vertex> vertexBufferInput;
};

//Fix these names I dont like them
void TSR_FillComponentVertexInput(DrawComponent * drawComponent)
{
	drawComponent->vertexBufferInput.reserve(TYPECAST(eastl_size_t, drawComponent->model.vertexCount));
	for (ui32 i = 0; i < drawComponent->model.vertexCount; ++i)
	{
		DirectX::XMFLOAT3 position = drawComponent->model.totalVertices[i];
		DirectX::XMFLOAT3 normal = drawComponent->model.normals[i];
		Vertex v{position, white, normal};
		drawComponent->vertexBufferInput.push_back(v);
	}
}

//NOTE(Fran): to do the typeof(T) use templates so I can have multiple input layouts, for now vertex
struct ModelBuffers
{
	BufferData vertexBuffer;
	BufferData indexBuffer;
};

void TSR_DX11_BuildGeometryBuffersFromComponent(ID3D11Device * device, DrawComponent * drawComponent, ModelBuffers * buffers)
{
	TSR_DX11_BuildBuffer(device,
		sizeof(Vertex),
		0,
		drawComponent->model.vertexCount,
		D3D11_USAGE_IMMUTABLE,
		D3D11_BIND_VERTEX_BUFFER,
		drawComponent->vertexBufferInput.data(),
		&buffers->vertexBuffer
	);
	TSR_DX11_BuildBuffer(device,
		sizeof(ui32),
		0,
		drawComponent->model.indexCount,
		D3D11_USAGE_IMMUTABLE,
		D3D11_BIND_INDEX_BUFFER,
		drawComponent->model.totalIndices.data(),
		&buffers->indexBuffer
	);
}

HRESULT TSR_DX11_CreateShaderInputLayout(ID3D11Device* device, ui32 inputElements, D3D11_INPUT_ELEMENT_DESC inputLayoutDescriptor[], ID3DBlob* shaderBuffer, ID3D11InputLayout** inputLayout)
{
	HRESULT hr;

	hr = device->CreateInputLayout(
		inputLayoutDescriptor,
		inputElements,
		shaderBuffer->GetBufferPointer(),
		shaderBuffer->GetBufferSize(),
		inputLayout);

	return hr;
}

//TODO(Fran): Maybe include shader information such as shadername etc
void TSR_DX11_BuildVertexShader(ID3D11Device* device, eastl::wstring csoPath, ui32 inputElements, D3D11_INPUT_ELEMENT_DESC inputLayoutDescriptor[], DX11VertexShaderData* vsData)
{
	HRESULT hr;

	hr = D3DReadFileToBlob(csoPath.begin(), &vsData->shaderBuffer);
	LOGASSERT(LOGSYSTEM_DX11, "Failed loading Vertex Shader.", !FAILED(hr));
	LOGDEBUG(LOGSYSTEM_DX11, "Vertex Shader successfully loaded.");

	hr = device->CreateVertexShader(vsData->shaderBuffer->GetBufferPointer(), 
									vsData->shaderBuffer->GetBufferSize(), 
									0, 
									&vsData->shader
									);
	LOGASSERT(LOGSYSTEM_DX11, "Failed creating Vertex Shader.", !FAILED(hr));
	LOGDEBUG(LOGSYSTEM_DX11, "Vertex Shader successfully created.");

	hr = TSR_DX11_CreateShaderInputLayout(device, inputElements, inputLayoutDescriptor, vsData->shaderBuffer, &vsData->inputLayout);
	LOGASSERT(LOGSYSTEM_DX11, "Failed creating Vertex Shader Input Layout.", !FAILED(hr));
	LOGDEBUG(LOGSYSTEM_DX11, "Vertex Shader Input Layout successfully created.");
}

void TSR_DX11_BuildPixelShader(ID3D11Device* device, eastl::wstring csoPath, DX11PixelShaderData* psData)
{
	HRESULT hr;
	hr = D3DReadFileToBlob(csoPath.begin(), &psData->shaderBuffer);
	LOGASSERT(LOGSYSTEM_DX11, "Failed loading Pixel Shader.", !FAILED(hr));
	LOGDEBUG(LOGSYSTEM_DX11, "Pixel Shader successfully loaded.");

	hr = device->CreatePixelShader( psData->shaderBuffer->GetBufferPointer(),
									psData->shaderBuffer->GetBufferSize(), 
									0, 
									&psData->shader
									);
	LOGASSERT(LOGSYSTEM_DX11, "Failed creating Pixel Shader.", !FAILED(hr));
	LOGDEBUG(LOGSYSTEM_DX11, "Pixel Shader successfully created.");

}

namespace DX11InputLayout
{
	D3D11_INPUT_ELEMENT_DESC PCN[] =
	{
		{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,								D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	ui32 pcnsize = 3;

	D3D11_INPUT_ELEMENT_DESC PNT[] =
	{
		{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,								D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	ui32 pntsize = 3;
}

void TSR_DX11_BuildShaders(ID3D11Device * device, DX11VertexShaderData * vsData, DX11PixelShaderData * psData)
{
	TSR_DX11_BuildVertexShader(device, eastl::wstring(L"./CompiledShaders/mainVS.cso"), DX11InputLayout::pcnsize, DX11InputLayout::PCN, vsData);
	TSR_DX11_BuildPixelShader(device, eastl::wstring(L"./CompiledShaders/mainPS.cso"), psData);
}

//https://docs.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-resources-textures-how-to
void TSR_DX11_ImportTextures(ID3D11Device * device, eastl::vector<MaterialMapNames> mapNames)
{
	
}