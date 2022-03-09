/*
	This .cpp contains all functions to interact with D3D11.
*/
#include "cgraph_dx11_data.h"

//TODO(Fran): Do the debug implementation with dxtrace etc.
//TODO(Fran): Check MSAA thingy.
bool InitD3D11(HWND hWnd, RECT wRect, DX11Info* dxInfo)
{
	HRESULT hr;
	UINT wWidth = wRect.right - wRect.left;
	UINT wHeight = wRect.bottom - wRect.top;

	// Desired Feature level
	D3D_FEATURE_LEVEL fLevel = { D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0 };
	// Device Flags
	UINT createDeviceFlags = 0;

#ifdef DEBUG
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
		&dxInfo->device,
		dxInfo->featureLevel,
		&dxInfo->imDeviceContext
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
	scDescriptor.BufferDesc.RefreshRate.Numerator = 60; //this is weird
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
	scDescriptor.Flags = 0;

	//Create Swap chain
	//Get the factory
	IDXGIDevice* dxgiDevice = 0;
	hr = dxInfo->device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed to get the DXGIDevice", 0, 0);
		return false;
	}

	IDXGIAdapter* dxgiAdapter = 0;
	hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed to get the DXGIAdapter", 0, 0);
		return false;
	}

	IDXGIFactory* dxgiFactory = 0;
	hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed to get the DXGIFactory", 0, 0);
		return false;
	}

	// finally create the swapchain...
	hr = dxgiFactory->CreateSwapChain(dxInfo->device, &scDescriptor, &dxInfo->swapChain);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed to create the SwapChain", 0, 0);
		return false;
	}

	//Release the COM interfaces (decrement references)
	dxgiDevice->Release();
	dxgiAdapter->Release();
	dxgiFactory->Release();

	// Create Render Target view
	ID3D11Texture2D* backBuffer;
	hr = dxInfo->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed acquiring back buffer", 0, 0);
		return false;
	}
	else
	{
		dxInfo->device->CreateRenderTargetView(backBuffer, 0, &dxInfo->renderTargetView);
	}
	//Release COM interface
	backBuffer->Release();

	// Depth buffer
	D3D11_TEXTURE2D_DESC dsDescriptor = { 0 };
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
	hr = dxInfo->device->CreateTexture2D(&dsDescriptor, 0, &dxInfo->depthStencilBuffer);
	if (FAILED(hr)) {
		MessageBox(0, L"Failed creating depth buffer", 0, 0);
		return false;
	}
	else
	{
		hr = dxInfo->device->CreateDepthStencilView(dxInfo->depthStencilBuffer, 0, &dxInfo->depthStencilView);
		if (FAILED(hr)) {
			MessageBox(0, L"Failed creating depth view", 0, 0);
			return false;
		}
		//bind views to output merger stage
		// we can bind multiple render target views.
		dxInfo->imDeviceContext->OMSetRenderTargets(1, &dxInfo->renderTargetView, dxInfo->depthStencilView);
	}

	// create viewport and set it
	// maybe split screen or stuff could be done with several viewports.
	dxInfo->screenViewport = { 0 };
	dxInfo->screenViewport.MinDepth = 0.0f;
	dxInfo->screenViewport.MaxDepth = 1.0f;
	dxInfo->screenViewport.TopLeftX = 0.0f;
	dxInfo->screenViewport.TopLeftY = 0.0f;
	dxInfo->screenViewport.Width = static_cast<float>(wWidth);
	dxInfo->screenViewport.Height = static_cast<float>(wHeight);

	dxInfo->imDeviceContext->RSSetViewports(1, &dxInfo->screenViewport);

	//Rasterizer state??
	D3D11_RASTERIZER_DESC rsDescriptor;
	ZeroMemory(&rsDescriptor, sizeof(rsDescriptor));
	rsDescriptor.FillMode = D3D11_FILL_SOLID;
	rsDescriptor.CullMode = D3D11_CULL_BACK;
	rsDescriptor.FrontCounterClockwise = false;
	rsDescriptor.DepthClipEnable = true;

	ID3D11RasterizerState* defaultRasterizer;
	hr = dxInfo->device->CreateRasterizerState(&rsDescriptor, &defaultRasterizer);
	if (FAILED(hr)) {
		MessageBox(0, L"Failed creating Rasterizer State", 0, 0);
		return false;
	}
	dxInfo->currentRasterizerState = defaultRasterizer;
	dxInfo->imDeviceContext->RSSetState(dxInfo->currentRasterizerState);

	return true;
}

bool BuildTriangleGeometryBuffers(ID3D11Device & device, ID3D11Buffer * vBuffer, ID3D11Buffer * iBuffer)
{
	//testing purposes now
	DirectX::XMFLOAT4 green = { 0.0f, 1.0f, 0.0f, 1.0f };
	// Triangle vertex buffer
	Vertex triangleVertices []=
	{
		{DirectX::XMFLOAT3(0.0f, 0.5f, 1.0f), green},
		{DirectX::XMFLOAT3(0.5f, -0.5f, 1.0f), green},
		{DirectX::XMFLOAT3(-0.5f, -0.5f, 1.0f), green}
	};

	D3D11_BUFFER_DESC tvbd = { 0 };
	tvbd.Usage = D3D11_USAGE_IMMUTABLE;
	tvbd.ByteWidth = sizeof(Vertex) * 3; //num of members in vertex array
	tvbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	
	D3D11_SUBRESOURCE_DATA tvInitData = { 0 };
	tvInitData.pSysMem = triangleVertices;

	HRESULT hr = device.CreateBuffer(&tvbd, &tvInitData, &vBuffer);
	if (FAILED(hr)) {
		MessageBox(0, L"Vertex ID3D11Buffer creation failed", 0, 0);
		return false;
	}

	UINT indices[] =
	{
		0, 1, 2
	};

	D3D11_BUFFER_DESC tibd = { 0 };
	tibd.Usage = D3D11_USAGE_IMMUTABLE;
	tibd.ByteWidth = sizeof(UINT) * 3;
	tibd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA tiInitData = { 0 };
	tiInitData.pSysMem = indices;

	hr = device.CreateBuffer(&tibd, &tiInitData, &iBuffer);
	if (FAILED(hr)) {
		MessageBox(0, L"Index ID3D11Buffer creation failed", 0, 0);
		return false;
	}

	return true;
}

bool BuildTriangleInputLayout(ID3D11Device & device, ID3D11InputLayout * inputLayout, ID3D10Blob * vs_buffer)
{
	D3D11_INPUT_ELEMENT_DESC vertexDescriptor[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HRESULT hr = D3DReadFileToBlob(L"./x64/Debug/mainVS.cso", &vs_buffer);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed creating vertex shader blob", 0, 0);
		return false;
	}
	ID3D11VertexShader* shader = nullptr;
	hr = device.CreateVertexShader(vs_buffer->GetBufferPointer(), vs_buffer->GetBufferSize(), 0, &shader);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed creating vertex shader", 0, 0);
		return false;
	}

	hr = device.CreateInputLayout(vertexDescriptor, 2, vs_buffer->GetBufferPointer(), vs_buffer->GetBufferSize(), &inputLayout);

	if (FAILED(hr))
	{
		MessageBox(0, L"Failed creating Input Layout", 0, 0);
		return false;
	}

	return true;
}
