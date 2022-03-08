#include <Windows.h>

// std includes
#include <iostream>
#include <sstream>


// dx includes
#include <d3d11.h>
#include <dxgi.h>

#define WIN32_LEAN_AND_MEAN

#define DEBUG

bool InitD3D11(HWND hWnd, RECT wRect)
{
	HRESULT hr;
	UINT wWidth = wRect.right - wRect.left;
	UINT wHeight = wRect.bottom - wRect.top;
	// Feature level
	D3D_FEATURE_LEVEL fLevel = { D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0 };

	ID3D11Device* DX11device;
	D3D_FEATURE_LEVEL DX11featureLevel;
	ID3D11DeviceContext* DX11immediateContext;
	// Create Device and Context
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
		&DX11device,
		&DX11featureLevel,
		&DX11immediateContext
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
	//NOTE(Fran): when the swapEffect is set to FLIP_DISCARD the swapchain creation fails
	// CHG[1]: DXGI_SWAP_EFFECT_FLIP_DISCARD
	scDescriptor.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scDescriptor.Flags = 0;

	//Create Swap chain
	IDXGISwapChain* DX11swapChain;

	//Get the factory
	IDXGIDevice* dxgiDevice = 0;
	hr = DX11device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed to get the DXGIDevice", 0, 0);
	}
	
	IDXGIAdapter* dxgiAdapter = 0;
	hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed to get the DXGIAdapter", 0, 0);
	}

	IDXGIFactory* dxgiFactory = 0;
	hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed to get the DXGIFactory", 0, 0);
	}
	
	// finally create the swapchain...
	hr = dxgiFactory->CreateSwapChain(DX11device, &scDescriptor, &DX11swapChain);
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed to create the SwapChain", 0, 0);
	}

	//Release the COM interfaces
	dxgiDevice->Release();
	dxgiAdapter->Release();
	dxgiFactory->Release();

	// Create Render Target view
	ID3D11RenderTargetView* renderTargetView = nullptr;
	ID3D11Texture2D* backBuffer;
	hr = DX11swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	if (FAILED(hr))
	{
		MessageBox(0, L"Failed accquiring backbuffer", 0, 0);
	}
	else
	{
		DX11device->CreateRenderTargetView(backBuffer, 0, &renderTargetView);
	}
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

	ID3D11Texture2D* depthStencilBuffer;
	ID3D11DepthStencilView* depthStencilView;
	hr = DX11device->CreateTexture2D(&dsDescriptor, 0, &depthStencilBuffer);
	if (FAILED(hr)) {
		MessageBox(0, L"Failed creating depth buffer", 0, 0);
	}
	else
	{
		hr = DX11device->CreateDepthStencilView(depthStencilBuffer, 0, &depthStencilView);
		if (FAILED(hr)) {
			MessageBox(0, L"Failed creating depth view", 0, 0);
		}
		//bind views to output merger stage
		// we can bind multiple render target views.
		DX11immediateContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
	}

	// create viewport and set it
	// maybe split screen or stuff could be done with several viewports.
	D3D11_VIEWPORT vp = { 0 };
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<float>(wWidth);
	vp.Height = static_cast<float>(wHeight);

	DX11immediateContext->RSSetViewports(1, &vp);

	return true;
}

void CalculateFrameStats(HWND hWnd, float totalTime)
{
	// avg frames per second
	// avg time in ms per frame
	// append stats to window caption bar

	static int frameCount = 0;
	static float elapsedTime = 0.0f;

	++frameCount;

	if (totalTime - elapsedTime >= 1.0f)
	{
		float fps = (float)frameCount;
		float mspf = 1000.0f / fps;

		//build the caption string
		std::wostringstream outstream;
		outstream.precision(6);
		outstream << L"CGraph window" << L" "
			<< L"FPS: " << fps << L" "
			<< L"Frame Time: " << mspf << L" (ms)";
		SetWindowText(hWnd, outstream.str().c_str());

		//reset
		frameCount = 0;
		elapsedTime += 1.0f;
	}
}

LRESULT WINAPI WndProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0); break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

struct TimeInfo
{
	__int64 baseTime;
	__int64 currTime;
	__int64 prevTime;
	__int64 countsPerSec;
	double secondsPerCount;
	double deltaTime;
	double totalTime;
} typedef TimeInfo;

void ResetTimeInformation(TimeInfo * tInfo)
{
	QueryPerformanceCounter((LARGE_INTEGER*)&tInfo->baseTime);
	tInfo->currTime = tInfo->baseTime;
	tInfo->prevTime = tInfo->baseTime;
	QueryPerformanceFrequency((LARGE_INTEGER*)&tInfo->countsPerSec);
	tInfo->secondsPerCount = 1.0 / (double)tInfo->countsPerSec;
	tInfo->deltaTime = 0.0;
	tInfo->totalTime = 0.0;
}

void UpdateTimeInformation(TimeInfo* tInfo)
{
	// get current time
	QueryPerformanceCounter((LARGE_INTEGER*)&tInfo->currTime);
	tInfo->deltaTime = (tInfo->currTime - tInfo->prevTime) * tInfo->secondsPerCount;
	tInfo->prevTime = tInfo->currTime;

	// Force non negative
	tInfo->deltaTime = tInfo->deltaTime < 0.0 ? 0.0 : tInfo->deltaTime;

	// update total time
	tInfo->totalTime += tInfo->deltaTime;

}

INT WINAPI wWinMain(
	_In_ HINSTANCE hInstance, 
	_In_opt_ HINSTANCE hPrevInstance, 
	_In_ LPWSTR lpCmdLine, 
	_In_ int nCmdShow)
{
	// Initialize and reset the time information for the application
	TimeInfo Time;
	ResetTimeInformation(&Time);
	
	// create and register the class to spawn the window
	LPCWSTR wcName = L"CGraphWindow";
	WNDCLASSEX wclass = { 0 };
	wclass.cbSize = sizeof(WNDCLASSEX);
	wclass.style = CS_HREDRAW | CS_VREDRAW;
	wclass.lpfnWndProc = WndProc;
	wclass.hInstance = hInstance;
	wclass.hCursor = LoadCursor(0, IDC_ARROW);
	wclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wclass.lpszClassName = wcName;
	RegisterClassEx(&wclass);

	// create the window and display it.
	RECT wRect = { 0,0,1280,720 };
	AdjustWindowRect(&wRect, WS_OVERLAPPEDWINDOW, FALSE);
	HWND wHandler = CreateWindow(
		wcName,
		L"CGraph Window", // window name
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, // style for the window
		CW_USEDEFAULT, CW_USEDEFAULT, // X AND Y start positions for the window
		wRect.right - wRect.left,
		wRect.bottom - wRect.top,
		0, 0, hInstance, 0);
	ShowWindow(wHandler, nCmdShow);
	InitD3D11(wHandler, wRect);
	// Message loop
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			UpdateTimeInformation(&Time);
			// calculate and show frame stats:
			// Note(Fran): Currently it averages it every second.
			CalculateFrameStats(wHandler, (float)Time.totalTime);
			
			//do the update and render

			
		}
	}

	return 0;
}