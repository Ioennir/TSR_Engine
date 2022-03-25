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
	scDescriptor.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scDescriptor.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

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

	// finally create the swapchain ...
	hr = dxgiFactory->CreateSwapChain(dxData->device, &scDescriptor, &dxData->swapChain);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed to create the SwapChain for the window", 0, 0);
		return false;
	}

	//Release the COM interfaces (decrement references)
	dxgiDevice->Release();
	dxgiAdapter->Release();
	dxgiFactory->Release();

	// Create main window Render Target view
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

	// RENDER TEXTURE
	D3D11_TEXTURE2D_DESC rtDescriptor{ 0 };
	rtDescriptor.Width = rtWidth;
	rtDescriptor.Height = rtHeight;
	rtDescriptor.MipLevels = 1;
	rtDescriptor.ArraySize = 1;
	rtDescriptor.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rtDescriptor.SampleDesc.Count = 1;
	rtDescriptor.SampleDesc.Quality = 0;
	rtDescriptor.Usage = D3D11_USAGE_DEFAULT;
	rtDescriptor.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	hr = dxData->device->CreateTexture2D(&rtDescriptor, 0, &dxData->scnData.renderTexture);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed creating render texture", 0, 0);
		return false;
	}

	// RENDER TEXTURE DEPTH BUFFER
	D3D11_TEXTURE2D_DESC rtdbDescriptor{ 0 };
	rtdbDescriptor.Width = rtWidth;
	rtdbDescriptor.Height = rtHeight;
	rtdbDescriptor.MipLevels = 1;
	rtdbDescriptor.ArraySize = 1;
	rtdbDescriptor.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	rtdbDescriptor.SampleDesc.Count = 1;
	rtdbDescriptor.SampleDesc.Quality = 0;
	rtdbDescriptor.Usage = D3D11_USAGE_DEFAULT;
	rtdbDescriptor.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	hr = dxData->device->CreateTexture2D(&rtdbDescriptor, 0, &dxData->scnData.depthStencilBuffer);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed creating depth stencil texture", 0, 0);
		return false;
	}
	else
	{
		hr = dxData->device->CreateDepthStencilView(dxData->scnData.depthStencilBuffer, 0, &dxData->scnData.depthStencilView);
		if (FAILED(hr)) {
			MessageBox(0, L"Failed creating depth view from depth stencil texture", 0, 0);
			return false;
		}
	}

	// RENDER TARGET VIEW DESCRIPTION FOR THE RENDER TEXTURE
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;
	dxData->device->CreateRenderTargetView(dxData->scnData.renderTexture, &rtvDesc, &dxData->scnData.renderTargetView);

	// SHADER RESOURCE VIEW FOR THE RENDER TEXTURE
	D3D11_SHADER_RESOURCE_VIEW_DESC srDesc{};
	srDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srDesc.Texture2D.MostDetailedMip = 0;
	srDesc.Texture2D.MipLevels = 1;
	dxData->device->CreateShaderResourceView(dxData->scnData.renderTexture, &srDesc, &dxData->scnData.shaderResourceView);


	// Depth buffer
	D3D11_TEXTURE2D_DESC dsDescriptor{ 0 };
	dsDescriptor.Width = wWidth;
	dsDescriptor.Height = wHeight;
	dsDescriptor.MipLevels = 1;
	dsDescriptor.ArraySize = 1;
	dsDescriptor.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
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

	//TODO(Frna): do this!!
	//dxData->device->CreateDepthStencilState(, );

	// create viewport and set it
	// maybe split screen or stuff could be done with several viewports.
	dxData->windowViewport = { 0 };
	dxData->windowViewport.MinDepth = 0.0f;
	dxData->windowViewport.MaxDepth = 1.0f;
	dxData->windowViewport.TopLeftX = 0.0f;
	dxData->windowViewport.TopLeftY = 0.0f;
	dxData->windowViewport.Width = static_cast<float>(wWidth);
	dxData->windowViewport.Height = static_cast<float>(wHeight);

	dxData->scnData.viewport = { 0 };
	dxData->scnData.viewport.MinDepth = 0.0f;
	dxData->scnData.viewport.MaxDepth = 1.0f;
	dxData->scnData.viewport.TopLeftX = 0.0f;
	dxData->scnData.viewport.TopLeftY = 0.0f;
	dxData->scnData.viewport.Width = static_cast<float>(rtWidth);
	dxData->scnData.viewport.Height = static_cast<float>(rtHeight);


	dxData->imDeviceContext->RSSetViewports(1, &dxData->windowViewport);
	
	return true;
}

bool BuildTriangleGeometryBuffers(ID3D11Device & device, BufferData * vBuffer, BufferData * iBuffer)
{
	//testing purposes now
	DirectX::XMFLOAT4 green { 0.0f, 1.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT4 red { 1.0f, 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT4 blue { 0.0f, 0.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT4 purple{ 1.0f, 0.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT4 cyan{ 0.0f, 1.0f, 1.0f, 1.0f };
	DirectX::XMFLOAT4 yell{ 1.0f, 1.0f, 0.0f, 1.0f };


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
		{DirectX::XMFLOAT3(0.5f, 0.5f, -0.5f), purple},
		{DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f), purple},
		{DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f), purple},

		{DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f), purple},
		{DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), purple},
		{DirectX::XMFLOAT3(0.5f, 0.5f, -0.5f), purple},

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
		{DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), yell},
		{DirectX::XMFLOAT3(0.5f, -0.5f, -0.5f), yell},
		{DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f), yell},

		{DirectX::XMFLOAT3(0.5f, -0.5f, 0.5f), yell},
		{DirectX::XMFLOAT3(-0.5f, -0.5f, 0.5f), yell},
		{DirectX::XMFLOAT3(-0.5f, -0.5f, -0.5f), yell},

		//{DirectX::XMFLOAT3(0.0f, 0.5f, 0.0f), green},
		//{DirectX::XMFLOAT3(0.3f, 0.0f, 0.0f), red},
		//{DirectX::XMFLOAT3(-0.3f, 0.0f, 0.0f), blue}
	};

	vBuffer->stride = sizeof(Vertex);
	vBuffer->offset = 0;

	float memberCount = sizeof(triangleVertices) / vBuffer->stride;

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
