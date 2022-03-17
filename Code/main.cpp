// windows includes
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
//#define DEBUG
//#define BORDERLESS

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

// std includes
#include <iostream>
#include <sstream>

// DX11 layer
#include "cgraph_dx11.cpp"

//TODO(Fran): start sort of a profiling layer and add the time thing into that.
//TODO(Fran): https://stackoverflow.com/questions/431470/window-border-width-and-height-in-win32-how-do-i-get-it
// check the window vs windowclient size thingy
//TODO(Fran): It might be a good idea to draw the editor with imgui first and 
// then setup the render target within the editor to render the scene, it might be cool to be able to swap between wireframe
// and full rendered or smth like that it would allow in the future to have lit, unlit etc.

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

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(
	HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam)
{
	
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

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

	DWORD wStyle;

#ifdef BORDERLESS
	wStyle = WS_POPUPWINDOW; //borderless window
#else
	wStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX; // style for the window
#endif 


	AdjustWindowRect(&wRect, WS_OVERLAPPEDWINDOW, FALSE);
	HWND wHandler = CreateWindow(
		wcName,
		winName, // window name
		wStyle,
		100,100,//CW_USEDEFAULT, CW_USEDEFAULT, // X AND Y start positions for the window
		wRect.right - wRect.left,
		wRect.bottom - wRect.top,
		0, 0, hInstance, 0);

	// Show the window
	ShowWindow(wHandler, nCmdShow);

	return wHandler;
}

void UpdateScene(float dt)
{

}

void DrawGUI(DX11Data & dxData)
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport();

	ImGui::Begin("Test Window");
		ImGui::Text("This is example text.");
	ImGui::End();
	ImGuiWindowFlags rtWindowFlags = 0;// = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
	ImVec2 rtSize{ 640.0f, 360.0f };
	//ImGui::SetNextWindowSize(rtSize);
	ImGui::SetNextWindowBgAlpha(1.0f);
	ImGui::Begin("Viewport", 0, rtWindowFlags);
	{
		//https://github.com/ocornut/imgui/issues/2987
		ImGui::Image(reinterpret_cast<void*>(dxData.shaderResView), ImVec2{ 640.0f, 360.0f }, ImVec2{ 1,0 }, ImVec2{ 0,1 });
	}
	ImGui::End();
	ImGui::Render();	
	
	ID3D11RenderTargetView * views[2];
	views[0] = dxData.renderTargetView;
	views[1] = dxData.textureRTView;
	dxData.imDeviceContext->OMSetRenderTargets(2, views, dxData.depthStencilView);
	//dxData.imDeviceContext->OMSetRenderTargets(1, &dxData.textureRTView, dxData.depthStencilView);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	
}

void DrawScene(DX11Data & dxData, DX11VertexShaderData & vsData, DX11PixelShaderData & psData, BufferData & vb, BufferData & ib)
{
	//clear backbuffer
	DirectX::XMVECTORF32 red { 1.0f, 0.0f, 0.0f, 1.0f };

	//text
	dxData.imDeviceContext->ClearRenderTargetView(dxData.textureRTView, reinterpret_cast<const float*>(&red));
	dxData.imDeviceContext->ClearDepthStencilView(dxData.depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	//dxData.imDeviceContext->ClearRenderTargetView(dxData.renderTargetView, reinterpret_cast<const float*>(&red));
	//dxData.imDeviceContext->ClearDepthStencilView(dxData.depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	dxData.imDeviceContext->IASetInputLayout(vsData.inputLayout);
	dxData.imDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	dxData.imDeviceContext->VSSetShader(vsData.shader, 0, 0);
	dxData.imDeviceContext->PSSetShader(psData.shader, 0, 0);

	dxData.imDeviceContext->IASetVertexBuffers(0, 1, &vb.buffer, &vb.stride, &vb.offset);
	dxData.imDeviceContext->IASetIndexBuffer(ib.buffer, DXGI_FORMAT_R32_UINT, ib.offset);
	
	dxData.imDeviceContext->DrawIndexed(3, 0, 0);

	DrawGUI(dxData);

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

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

	// setup Imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& imIO = ImGui::GetIO();
	imIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;// | ImGuiConfigFlags_ViewportsEnable;
	ImGui_ImplWin32_Init(wHandler);
	ImGui_ImplDX11_Init(dxData.device, dxData.imDeviceContext);
	ImGui::StyleColorsDark();

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