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


struct TimeData
{
	__int64 baseTime {0};
	__int64 currTime {0};
	__int64 prevTime {0};
	__int64 countsPerSec {0};
	double secondsPerCount {0.0};
	double deltaTime {0.0};
	double totalTime {0.0};
};

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

void UpdateTimeInformation(TimeData* tData)
{
	// get current time
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&tData->currTime));
	tData->deltaTime = (tData->currTime - tData->prevTime) * tData->secondsPerCount;
	tData->prevTime = tData->currTime;

	// Force non negative
	tData->deltaTime = tData->deltaTime < 0.0 ? 0.0 : tData->deltaTime;

	// update total time
	tData->totalTime += tData->deltaTime;

}

void ResetTimeInformation(TimeData* tData)
{
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&tData->baseTime));
	tData->currTime = tData->baseTime;
	tData->prevTime = tData->baseTime;
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&tData->countsPerSec));
	tData->secondsPerCount = 1.0 / static_cast<double>(tData->countsPerSec);
	tData->deltaTime = 0.0;
	tData->totalTime = 0.0;
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
	WNDCLASSEX wclass { 0 };
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

void DrawScene(DX11Data & dxData, DX11VertexShaderData & vsData, DX11PixelShaderData & psData, BufferData & vb, BufferData & ib)
{
	//clear backbuffer
	DirectX::XMVECTORF32 red { 1.0f, 0.0f, 0.0f, 1.0f };
	dxData.imDeviceContext->ClearRenderTargetView(dxData.renderTargetView, reinterpret_cast<const float*>(&red));
	dxData.imDeviceContext->ClearDepthStencilView(dxData.depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	dxData.imDeviceContext->IASetInputLayout(vsData.inputLayout);
	dxData.imDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	dxData.imDeviceContext->VSSetShader(vsData.shader, 0, 0);
	dxData.imDeviceContext->PSSetShader(psData.shader, 0, 0);

	dxData.imDeviceContext->IASetVertexBuffers(0, 1, &vb.buffer, &vb.stride, &vb.offset);
	dxData.imDeviceContext->IASetIndexBuffer(ib.buffer, DXGI_FORMAT_R32_UINT, ib.offset);
	
	dxData.imDeviceContext->DrawIndexed(3, 0, 0);

	dxData.swapChain->Present(NO_VSYNC, 0);
}

INT WINAPI wWinMain(
	_In_ HINSTANCE hInstance, 
	_In_opt_ HINSTANCE hPrevInstance, 
	_In_ LPWSTR lpCmdLine, 
	_In_ int nCmdShow)
{
	// Initialize and reset the time information for the application
	TimeData Time;
	ResetTimeInformation(&Time);

	// create the window and display it.
	RECT wRect { 0, 0, 1280, 720 };
	HWND wHandler = CreateAndSpawnWindow(L"CGraph Window", wRect, hInstance, nCmdShow);
	
	// Initialize DX11 and get all the information needed
	DX11Data dxData;
	if (!InitD3D11(wHandler, wRect, &dxData))
	{
		return -1;
	}

	//now init buffers and shaders
	BufferData vertexBuff;
	BufferData indexBuff;
	DX11VertexShaderData vsData;
	DX11PixelShaderData psData;

	
	if (!BuildTriangleShaders(*dxData.device, &vsData, &psData))
	{
		return -1;
	}

	if (!BuildTriangleGeometryBuffers(*dxData.device, &vertexBuff, &indexBuff))
	{
		return -1;
	}

	//this is for testing purposes;
	

	// Message loop
	MSG msg { 0 };
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

			DrawScene(dxData, vsData, psData, vertexBuff, indexBuff);
			
		}
	}

	return 0;
}