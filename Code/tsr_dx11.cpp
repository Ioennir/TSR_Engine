/*
	This .cpp contains all functions to interact with D3D11.
*/

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXPackedVector.h>

#define NO_VSYNC 0
#define VSYNC 1

struct DX11ViewportData 
{
	ID3D11Texture2D* RenderTargetTexture;
	ID3D11Texture2D* DepthStencilTexture;
	ID3D11RenderTargetView* RenderTargetView;
	ID3D11DepthStencilView* DepthStencilView;
	ID3D11DepthStencilState* DepthStencilState;
	ID3D11ShaderResourceView* ShaderResourceView;
	D3D11_VIEWPORT Viewport{};
	DirectX::XMFLOAT2 ViewportDimensions{1.0f, 1.0f};
};

struct DX11Data
{
	ID3D11Device* device{};
	ID3D11DeviceContext* imDeviceContext{};
	D3D_FEATURE_LEVEL* featureLevel{};
	IDXGISwapChain* swapChain{};
	ID3D11Texture2D* depthStencilBuffer{};
	ID3D11RenderTargetView* renderTargetView{};
	ID3D11DepthStencilView* depthStencilView{};
	ID3D11RasterizerState* currentRasterizerState{};
	D3D11_VIEWPORT windowViewport{};
	DX11ViewportData VP;
	ID3D11Buffer* dx11_cbuffer{};
};

struct DX11VertexShaderData
{
	ID3D10Blob* shaderBuffer{};
	ID3D11VertexShader* shader{};
	ID3D11InputLayout* inputLayout{};
};

struct DX11PixelShaderData
{
	ID3D10Blob* shaderBuffer{};
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
	scDescriptor.OutputWindow = winData.handle;
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
		&dxData->imDeviceContext
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

//TODO(Fran): check MSAA support later.
//TODO(Fran): Do the debug implementation with dxtrace etc.
//TODO(Fran): Check MSAA thingy.
bool TSR_DX11_Init(WindowData & winData, DX11Data* dxData)
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
	TSR_DX11_InitViewport(dxData->device, dxData->imDeviceContext, &dxData->VP);

	// create viewport and set it
	// maybe split screen or stuff could be done with several viewports.
	dxData->windowViewport = { 0 };
	dxData->windowViewport.MinDepth = 0.0f;
	dxData->windowViewport.MaxDepth = 1.0f;
	dxData->windowViewport.TopLeftX = 0.0f;
	dxData->windowViewport.TopLeftY = 0.0f;
	dxData->windowViewport.Width = static_cast<float>(wWidth);
	dxData->windowViewport.Height = static_cast<float>(wHeight);

	dxData->VP.Viewport = { 0 };
	dxData->VP.Viewport.MinDepth = 0.0f;
	dxData->VP.Viewport.MaxDepth = 1.0f;
	dxData->VP.Viewport.TopLeftX = 0.0f;
	dxData->VP.Viewport.TopLeftY = 0.0f;
	dxData->VP.Viewport.Width = static_cast<float>(rtWidth);
	dxData->VP.Viewport.Height = static_cast<float>(rtHeight);
	dxData->VP.ViewportDimensions = { static_cast<float>(rtWidth), static_cast<float>(rtHeight)};
	return true;
}

bool BuildGeometryBuffer(ID3D11Device * device, RenderData & renderData, BufferData * vBuffer, BufferData* iBuffer)
{
	vBuffer->stride = sizeof(Vertex);
	vBuffer->offset = 0;

	D3D11_BUFFER_DESC tvbd{ 0 };
	tvbd.Usage = D3D11_USAGE_IMMUTABLE;
	//tvbd.ByteWidth = vBuffer->stride * renderData.meshes[0].vertices.size();//vBuffer->stride * memberCount; //num of members in vertex array
	tvbd.ByteWidth = vBuffer->stride * static_cast<UINT>(renderData.vertexData.size());
	tvbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA tvInitData{ 0 };
	tvInitData.pSysMem = renderData.vertexData.data();//renderData.meshes[0].vertices.data();

	HRESULT hr = device->CreateBuffer(&tvbd, &tvInitData, &vBuffer->buffer);
	if (FAILED(hr)) {
		MessageBox(0, L"Vertex ID3D11Buffer creation failed", 0, 0);
		return false;
	}

	iBuffer->stride = sizeof(ui32);
	iBuffer->offset = 0;

	D3D11_BUFFER_DESC tibd{ 0 };
	tibd.Usage = D3D11_USAGE_IMMUTABLE;

	//tibd.ByteWidth = iBuffer->stride * renderData.meshes[0].indices.size();//iBuffer->stride * memberCount;
	tibd.ByteWidth = iBuffer->stride * static_cast<UINT>(renderData.totalIndices.size());
	tibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA tiInitData{ 0 };
	tiInitData.pSysMem = renderData.totalIndices.data();//renderData.meshes[0].indices.data();

	hr = device->CreateBuffer(&tibd, &tiInitData, &iBuffer->buffer);
	if (FAILED(hr)) {
		MessageBox(0, L"Index ID3D11Buffer creation failed", 0, 0);
		return false;
	}

	return true;
}

bool BuildTriangleGeometryBuffers(ID3D11Device & device, BufferData * vBuffer, BufferData * iBuffer)
{
	// Triangle vertex buffer
	// now cube
	Vertex triangleVertices []=
	{
		// front face
		{DirectX::XMFLOAT3(-0.5f, 0.5f, -0.5f), green},
		{DirectX::XMFLOAT3(0.5f, 0.5f, -0.5f), green},
		{DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), green},

		{DirectX::XMFLOAT3(-0.5f, 0.5f, -0.5f), green},
		{DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), green},
		{DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), green},

		// top face
		{DirectX::XMFLOAT3(-0.5f, 0.5f, 0.5f), red},
		{DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f), red},
		{DirectX::XMFLOAT3(0.5f, 0.5f, -0.5f), red},
		
		{DirectX::XMFLOAT3(-0.5f, 0.5f, 0.5f), red},
		{DirectX::XMFLOAT3(0.5f, 0.5f, -0.5f), red},
		{DirectX::XMFLOAT3(-0.5f, 0.5f, -0.5f), red},

		// right face
		{DirectX::XMFLOAT3(0.5f, 0.5f, -0.5f), magenta},
		{DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f), magenta},
		{DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f), magenta},

		{DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f), magenta},
		{DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), magenta},
		{DirectX::XMFLOAT3(0.5f, 0.5f, -0.5f), magenta},

		// back face
		{DirectX::XMFLOAT3(-0.5f, 0.5f, 0.5f), blue},
		{DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f), blue},
		{DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f), blue},

		{DirectX::XMFLOAT3(-0.5f, 0.5f, 0.5f), blue},
		{DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f), blue},
		{DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f), blue},

		// left face
		{DirectX::XMFLOAT3(-0.5f, 0.5f, -0.5f), cyan},
		{DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), cyan},
		{DirectX::XMFLOAT3(-0.5f, 0.5f, 0.5f), cyan},

		{DirectX::XMFLOAT3(-0.5f, 0.5f, 0.5f), cyan},
		{DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), cyan},
		{DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f), cyan},

		// bottom face
		{DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), yellow},
		{DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), yellow},
		{DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f), yellow},

		{DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f), yellow},
		{DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f), yellow},
		{DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), yellow},

		//{DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f), green},
		//{DirectX::XMFLOAT3(0.3f, 0.0f, 0.0f), red},
		//{DirectX::XMFLOAT3(-0.3f, 0.0f, 0.0f), blue}
	};

	vBuffer->stride = sizeof(Vertex);
	vBuffer->offset = 0;

	UINT memberCount = sizeof(triangleVertices) / vBuffer->stride;

	D3D11_BUFFER_DESC tvbd { 0 };
	tvbd.Usage = D3D11_USAGE_IMMUTABLE;
	tvbd.ByteWidth = vBuffer->stride * memberCount; //num of members in vertex array
	tvbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	
	D3D11_SUBRESOURCE_DATA tvInitData { 0 };
	tvInitData.pSysMem = triangleVertices;

	HRESULT hr = device.CreateBuffer(&tvbd, &tvInitData, &vBuffer->buffer);
	if (FAILED(hr)) {
		MessageBox(0, L"Vertex ID3D11Buffer creation failed", 0, 0);
		return false;
	}

	UINT indices[] =
	{
		// front face
		0, 1, 2,
		3, 4, 5,

		// top face
		6, 7, 8,
		9, 10, 11,

		// right face
		12, 13, 14,
		15, 16, 17,

		// back face
		18, 19, 20,
		21, 22, 23,

		//left face
		24, 25, 26,
		27, 28, 29,

		//bottom
		30, 31, 32,
		33, 34, 35

	};

	iBuffer->stride = sizeof(UINT);
	iBuffer->offset = 0;

	D3D11_BUFFER_DESC tibd { 0 };
	tibd.Usage = D3D11_USAGE_IMMUTABLE;
	tibd.ByteWidth = iBuffer->stride * memberCount;
	tibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA tiInitData { 0 };
	tiInitData.pSysMem = indices;

	hr = device.CreateBuffer(&tibd, &tiInitData, &iBuffer->buffer);
	if (FAILED(hr)) {
		MessageBox(0, L"Index ID3D11Buffer creation failed", 0, 0);
		return false;
	}

	return true;
}

bool BuildTriangleShaders(ID3D11Device * device, DX11VertexShaderData * vsData, DX11PixelShaderData * psData)
{
	D3D11_INPUT_ELEMENT_DESC vsInputLayoutDescriptor[] =
	{
		{"POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,								D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, D3D11_APPEND_ALIGNED_ELEMENT,	D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	ui32 elementCount = sizeof(vsInputLayoutDescriptor) / sizeof(vsInputLayoutDescriptor[0]);

	//BUILD VERTEX SHADER

	HRESULT hr = D3DReadFileToBlob(L"./CompiledShaders/mainVS.cso", &vsData->shaderBuffer);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed loading vertex shader", 0, 0);
		return false;
	}

	hr = device->CreateVertexShader(
		vsData->shaderBuffer->GetBufferPointer(),
		vsData->shaderBuffer->GetBufferSize(), 
		0, 
		&vsData->shader);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed creating vertex shader", 0, 0);
		return false;
	}

	hr = device->CreateInputLayout(
		vsInputLayoutDescriptor, 
		elementCount, 
		vsData->shaderBuffer->GetBufferPointer(), 
		vsData->shaderBuffer->GetBufferSize(), 
		&vsData->inputLayout);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed creating Input Layout", 0, 0);
		return false;
	}

	//BUILD PIXEL SHADER
	hr = D3DReadFileToBlob(L"./CompiledShaders/mainPS.cso", &psData->shaderBuffer);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed loading pixel shader", 0, 0);
		return false;
	}

	hr = device->CreatePixelShader(
		psData->shaderBuffer->GetBufferPointer(),
		psData->shaderBuffer->GetBufferSize(),
		0,
		&psData->shader);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed creating pixel shader", 0, 0);
		return false;
	}


	return true;
}
