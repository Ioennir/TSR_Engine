// windows includes
#include <Windows.h>
#define WIN32_LEAN_AND_MEAN

// std includes
#include <iostream>
#include <sstream>


// dx includes
#include <d3d11.h>
#include <dxgi.h>


#define DEBUG


struct DX11Info
{
	ID3D11Device* device;
	ID3D11DeviceContext* imDeviceContext;
	D3D_FEATURE_LEVEL* featureLevel;
	IDXGISwapChain* swapChain;
	ID3D11Texture2D* depthStencilBuffer;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11DepthStencilView* depthStencilView;
	D3D11_VIEWPORT screenViewport;

} typedef DX11Info;

bool InitD3D11(HWND hWnd, RECT wRect, DX11Info * dxInfo)
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
		outstream << L"CGraph Window" << L" "
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

HWND CreateAndSpawnWindow(LPCWSTR winName, RECT wRect, HINSTANCE hInstance, int nCmdShow)
{
	// create and register the class to spawn the window
	LPCWSTR wcName = L"CGraphWindowClass";
	WNDCLASSEX wclass = { 0 };
	wclass.cbSize = sizeof(WNDCLASSEX);
	wclass.style = CS_HREDRAW | CS_VREDRAW;
	wclass.lpfnWndProc = WndProc;
	wclass.hInstance = hInstance;
	wclass.hCursor = LoadCursor(0, IDC_ARROW);
	wclass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wclass.lpszClassName = wcName;
	RegisterClassEx(&wclass);

	AdjustWindowRect(&wRect, WS_OVERLAPPEDWINDOW, FALSE);
	HWND wHandler = CreateWindow(
		wcName,
		winName, // window name
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, // style for the window
		CW_USEDEFAULT, CW_USEDEFAULT, // X AND Y start positions for the window
		wRect.right - wRect.left,
		wRect.bottom - wRect.top,
		0, 0, hInstance, 0);

	//Show the window
	ShowWindow(wHandler, nCmdShow);

	return wHandler;
}

void UpdateScene(float dt)
{

}

void DrawScene()
{

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

	// create the window and display it.
	RECT wRect = { 0, 0, 1280, 720 };
	HWND wHandler = CreateAndSpawnWindow(L"CGraph Window", wRect, hInstance, nCmdShow);
	
	// Initialize DX11 and get all the information needed
	DX11Info dxInfo = { 0 };
	InitD3D11(wHandler, wRect, &dxInfo);

	// Message loop
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else // "game loop"
		{
			UpdateTimeInformation(&Time);
			// calculate and show frame stats:
			// Note(Fran): Currently it averages it every second.
			CalculateFrameStats(wHandler, (float)Time.totalTime);
			
			//do the update and render
			UpdateScene((float)Time.deltaTime);
			DrawScene();
			
		}
	}

	return 0;
}