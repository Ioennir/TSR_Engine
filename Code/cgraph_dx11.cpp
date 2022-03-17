/*
	This .cpp contains all functions to interact with D3D11.
*/
#include "cgraph_dx11_data.h"

//TODO(Fran): Do the debug implementation with dxtrace etc.
//TODO(Fran): Check MSAA thingy.
bool InitD3D11(HWND hWnd, RECT wRect, DX11Data* dxData)
{
	HRESULT hr;
	UINT wWidth = wRect.right - wRect.left;
	UINT wHeight = wRect.bottom - wRect.top;

	UINT rtWidth = 640;
	UINT rtHeight = 360;

	// Desired Feature level
	D3D_FEATURE_LEVEL fLevel = { D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0 };
	// Device Flags
	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

	hr = D3D11CreateDevice(
		0, // uses primary display
		D3D_DRIVER_TYPE_HARDWARE, // hardware rendering acceleration
		0, // we are rendering with hardware
		createDeviceFlags, // enable debug layer
		&fLevel,
		1,
		D3D11_SDK_VERSION,
		&dxData->device,
		dxData->featureLevel,
		&dxData->imDeviceContext
	);
	if (FAILED(hr))
	{
		MessageBox(0, L"D3D11CreateDevice Failed", 0, 0);
		return false;
	}

	//NOTE(Fran): check MSAA support later.
	bool msaaOn = false;

	//Describe Swap Chain
	DXGI_SWAP_CHAIN_DESC scDescriptor;
	scDescriptor.BufferDesc.Width = wWidth;
	scDescriptor.BufferDesc.Height = wHeight;
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
	scDescriptor.OutputWindow = hWnd;
	scDescriptor.Windowed = true;
	// NOTE(Fran): when the swapEffect is set to FLIP_DISCARD the swapchain creation fails
	// DXGI_SWAP_EFFECT_FLIP_DISCARD
	scDescriptor.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scDescriptor.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;


	//Describe Swap Chain
	DXGI_SWAP_CHAIN_DESC scDescriptor2;
	scDescriptor2.BufferDesc.Width = rtWidth;
	scDescriptor2.BufferDesc.Height = rtHeight;
	// TODO(Fran): maybe pool displays and query refresh rate to get this exact
	scDescriptor2.BufferDesc.RefreshRate.Numerator = 60;
	scDescriptor2.BufferDesc.RefreshRate.Denominator = 1;
	scDescriptor2.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDescriptor2.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scDescriptor2.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	if (msaaOn)
	{

	}
	else
	{
		scDescriptor2.SampleDesc.Count = 1;
		scDescriptor2.SampleDesc.Quality = 0;
	}
	scDescriptor2.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDescriptor2.BufferCount = 1;
	scDescriptor2.OutputWindow = hWnd;
	scDescriptor2.Windowed = true;
	scDescriptor2.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scDescriptor2.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;


	//Create Swap chain
	//Get the factory
	IDXGIDevice* dxgiDevice;
	hr = dxData->device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed to get the DXGIDevice", 0, 0);
		return false;
	}

	IDXGIAdapter* dxgiAdapter;
	hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed to get the DXGIAdapter", 0, 0);
		return false;
	}

	IDXGIFactory* dxgiFactory;
	hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed to get the DXGIFactory", 0, 0);
		return false;
	}

	// finally create the swapchain 1...
	hr = dxgiFactory->CreateSwapChain(dxData->device, &scDescriptor, &dxData->swapChain);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed to create the SwapChain 1", 0, 0);
		return false;
	}

	hr = dxgiFactory->CreateSwapChain(dxData->device, &scDescriptor2, &dxData->swapChain2);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed to create the SwapChain 2", 0, 0);
		return false;
	}

	//Release the COM interfaces (decrement references)
	dxgiDevice->Release();
	dxgiAdapter->Release();
	dxgiFactory->Release();

	// Create Render Target view
	ID3D11Texture2D* backBuffer;
	hr = dxData->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed acquiring back buffer", 0, 0);
		return false;
	}
	else
	{
		dxData->device->CreateRenderTargetView(backBuffer, 0, &dxData->renderTargetView);
	}
	//Release COM interface
	backBuffer->Release();

	// Render Target Texture
	D3D11_TEXTURE2D_DESC rtDescriptor{ 0 };
	rtDescriptor.Width = rtWidth;
	rtDescriptor.Height = rtHeight;
	rtDescriptor.MipLevels = 1;
	rtDescriptor.ArraySize = 1;
	rtDescriptor.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rtDescriptor.SampleDesc.Count = 1;
	rtDescriptor.Usage = D3D11_USAGE_DEFAULT;
	rtDescriptor.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	
	hr = dxData->device->CreateTexture2D(&rtDescriptor, 0, &dxData->renderTexture);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed to create render target texture.", 0, 0);
		return false;
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = rtDescriptor.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	hr = dxData->device->CreateRenderTargetView(dxData->renderTexture, &rtvDesc, &dxData->textureRTView);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed to create texture RT view.", 0, 0);
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{ };
	srvDesc.Format = rtvDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	
	hr = dxData->device->CreateShaderResourceView(dxData->renderTexture, &srvDesc, &dxData->shaderResView);
	
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed to shader resource view.", 0, 0);
		return false;
	}


	// Depth buffer
	D3D11_TEXTURE2D_DESC dsDescriptor{ 0 };
	dsDescriptor.Width = wWidth;
	dsDescriptor.Height = wHeight;
	dsDescriptor.MipLevels = 1;
	dsDescriptor.ArraySize = 1;
	dsDescriptor.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
	if (msaaOn)
	{

	}
	else
	{
		dsDescriptor.SampleDesc.Count = 1;
		dsDescriptor.SampleDesc.Quality = 0;
	}
	dsDescriptor.Usage = D3D11_USAGE_DEFAULT;
	dsDescriptor.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	//dsDescriptor.CPUAccessFlags = 0; no cpu access
	//dsDescriptor.MiscFlags = 0; optional flags

	// set depth buffer and view
	hr = dxData->device->CreateTexture2D(&dsDescriptor, 0, &dxData->depthStencilBuffer);
	if (FAILED(hr)) {
		MessageBox(0, L"Failed creating depth buffer", 0, 0);
		return false;
	}
	else
	{
		hr = dxData->device->CreateDepthStencilView(dxData->depthStencilBuffer, 0, &dxData->depthStencilView);
		if (FAILED(hr)) {
			MessageBox(0, L"Failed creating depth view", 0, 0);
			return false;
		}
		//bind views to output merger stage
		// we can bind multiple render target views.
		dxData->imDeviceContext->OMSetRenderTargets(1, &dxData->renderTargetView, dxData->depthStencilView);
	}

	// create viewport and set it
	// maybe split screen or stuff could be done with several viewports.
	dxData->screenViewport = { 0 };
	dxData->screenViewport.MinDepth = 0.0f;
	dxData->screenViewport.MaxDepth = 1.0f;
	dxData->screenViewport.TopLeftX = 0.0f;
	dxData->screenViewport.TopLeftY = 0.0f;
	dxData->screenViewport.Width = static_cast<float>(wWidth);
	dxData->screenViewport.Height = static_cast<float>(wHeight);

	dxData->imDeviceContext->RSSetViewports(1, &dxData->screenViewport);
	/*
	//Rasterizer state??
	D3D11_RASTERIZER_DESC rsDescriptor;
	ZeroMemory(&rsDescriptor, sizeof(rsDescriptor));
	rsDescriptor.FillMode = D3D11_FILL_SOLID;
	rsDescriptor.CullMode = D3D11_CULL_BACK;
	rsDescriptor.FrontCounterClockwise = false;
	rsDescriptor.DepthClipEnable = true;

	ID3D11RasterizerState* defaultRasterizer;
	hr = dxData->device->CreateRasterizerState(&rsDescriptor, &defaultRasterizer);
	if (FAILED(hr)) {
		MessageBox(0, L"Failed creating Rasterizer State", 0, 0);
		return false;
	}
	dxData->currentRasterizerState = defaultRasterizer;
	dxData->imDeviceContext->RSSetState(dxData->currentRasterizerState);
	*/
	return true;
}

bool BuildTriangleGeometryBuffers(ID3D11Device & device, BufferData * vBuffer, BufferData * iBuffer)
{
	//testing purposes now
	DirectX::XMFLOAT4 green { 0.0f, 1.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT4 red { 1.0f, 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT4 blue { 0.0f, 0.0f, 1.0f, 1.0f };
	// Triangle vertex buffer
	Vertex triangleVertices []=
	{
		{DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f), green},
		{DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f), red},
		{DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), blue}
	};

	vBuffer->stride = sizeof(Vertex);
	vBuffer->offset = 0;

	D3D11_BUFFER_DESC tvbd { 0 };
	tvbd.Usage = D3D11_USAGE_IMMUTABLE;
	tvbd.ByteWidth = vBuffer->stride * 3; //num of members in vertex array
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
		0, 1, 2
	};

	iBuffer->stride = sizeof(UINT);
	iBuffer->offset = 0;

	D3D11_BUFFER_DESC tibd { 0 };
	tibd.Usage = D3D11_USAGE_IMMUTABLE;
	tibd.ByteWidth = iBuffer->stride * 3;
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

bool BuildTriangleShaders(ID3D11Device & device, DX11VertexShaderData * vsData, DX11PixelShaderData * psData)
{
	D3D11_INPUT_ELEMENT_DESC vsInputLayoutDescriptor[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	//BUILD VERTEX SHADER
	//TODO(Fran): establish a shader folder path for debug and release
	HRESULT hr = D3DReadFileToBlob(L"./x64/Debug/mainVS.cso", &vsData->shaderBuffer);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed loading vertex shader", 0, 0);
		return false;
	}

	hr = device.CreateVertexShader(
		vsData->shaderBuffer->GetBufferPointer(),
		vsData->shaderBuffer->GetBufferSize(), 
		0, 
		&vsData->shader);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed creating vertex shader", 0, 0);
		return false;
	}

	hr = device.CreateInputLayout(
		vsInputLayoutDescriptor, 
		2, 
		vsData->shaderBuffer->GetBufferPointer(), 
		vsData->shaderBuffer->GetBufferSize(), 
		&vsData->inputLayout);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed creating Input Layout", 0, 0);
		return false;
	}

	//BUILD PIXEL SHADER
	hr = D3DReadFileToBlob(L"./x64/Debug/mainPS.cso", &psData->shaderBuffer);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed loading pixel shader", 0, 0);
		return false;
	}

	hr = device.CreatePixelShader(
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
