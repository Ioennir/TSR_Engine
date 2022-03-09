// windows includes
#include <Windows.h>
#define WIN32_LEAN_AND_MEAN
#define DEBUG

// std includes
#include <iostream>
#include <sstream>

// DX11 layer
#include "cgraph_dx11.cpp"

//TODO(Fran): start sort of a profiling layer and add the time thing into that.


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

void ResetTimeInformation(TimeInfo* tInfo)
{
	QueryPerformanceCounter((LARGE_INTEGER*)&tInfo->baseTime);
	tInfo->currTime = tInfo->baseTime;
	tInfo->prevTime = tInfo->baseTime;
	QueryPerformanceFrequency((LARGE_INTEGER*)&tInfo->countsPerSec);
	tInfo->secondsPerCount = 1.0 / (double)tInfo->countsPerSec;
	tInfo->deltaTime = 0.0;
	tInfo->totalTime = 0.0;
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
	if (!InitD3D11(wHandler, wRect, &dxInfo))
	{
		return -1;
	}
	//now init buffers, fx and lastly layout
	ID3D11Buffer* vertexBuff = nullptr;
	ID3D11Buffer* indexBuff = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;
	ID3D10Blob* vs_buffer = nullptr;
	BuildTriangleGeometryBuffers(*dxInfo.device, vertexBuff, indexBuff);
	
	BuildTriangleInputLayout(*dxInfo.device, inputLayout, vs_buffer);

	//this is for testing purposes;
	DirectX::XMVECTORF32 red = { 1.0f, 0.0f, 0.0f, 1.0f };

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

			//clear backbuffer
			dxInfo.imDeviceContext->ClearRenderTargetView(dxInfo.renderTargetView, reinterpret_cast<const float*>(&red));
			dxInfo.imDeviceContext->ClearDepthStencilView(dxInfo.depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
			dxInfo.swapChain->Present(0, 0);
			DrawScene();
			
		}
	}

	return 0;
}