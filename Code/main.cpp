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
#include "tsr_types.h"

// DX11 layer
#include "tsr_dx11.cpp"
#include "tsr_time.cpp"

//TODO(Fran): start sort of a profiling layer and add the time thing into that.
//TODO(Fran): https://stackoverflow.com/questions/431470/window-border-width-and-height-in-win32-how-do-i-get-it
// check the window vs windowclient size thingy
//TODO(Fran): check the mem leak from the buffer construction.

struct TimeData
{
	__int64 baseTime{ 0 };
	__int64 currTime{ 0 };
	__int64 prevTime{ 0 };
	__int64 countsPerSec{ 0 };
	r64 secondsPerCount{ 0.0 };
	r64 deltaTime{ 0.0 };
	r64 totalTime{ 0.0 };
};

struct FrameStats
{
	r32 fps;
	r32 ms_per_frame;
	
	r32 avgfps;
	r32 avgmspf;
};

void CalculateFrameStats(TimeData & tData, FrameStats * fStats)
{
	// avg frames per second
	// avg time in ms per frame
	// update Frame stats struct

	static ui64 frameCount = 0;
	static r64 elapsedTime = 0.0;
	++frameCount;
	
	static ui64 frameCountAvg = 0;
	static r64 elapsedTimeAvg = 0.0;
	++frameCountAvg;

	//fetches every 1/30 of a second
	r64 currTimeInS = tData.currTime * tData.secondsPerCount;
	r64 timeDiff = currTimeInS - elapsedTime;
	if (timeDiff >= 1.0/30.0)
	{
		fStats->fps = static_cast<r64>((1.0 / timeDiff) * frameCount);
		fStats->ms_per_frame = (timeDiff / frameCount) * 1000.0;
	
		frameCount = 0;
		elapsedTime = currTimeInS;
	}

	//fetches every second
	double timeDiffAvg = tData.totalTime - elapsedTimeAvg;
	if (timeDiffAvg >= 1.0) {
		fStats->avgfps = static_cast<r64>(frameCountAvg);
		fStats->avgmspf = 1000.0f / fStats->avgfps;

		frameCountAvg = 0;
		elapsedTimeAvg += 1.0;
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

void DrawGUI(DX11Data & dxData, IMData * imData, FrameStats & fStats)
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//ImGui::DockSpaceOverViewport();

	ImGui::Begin("Cube data");
		ImGui::SliderFloat3("Rotation axis", imData->rot, -1.0f, 1.0f);
		ImGui::DragFloat("Rotation speed", &imData->rotSpeed, 60.0f, -1000.0f, 1000.0f);
	ImGui::End();

	ImGui::Begin("Frame statistics");
		ImGui::Text("fps: %f", fStats.fps);
		ImGui::Text("ms per frame: %f", fStats.ms_per_frame);
		ImGui::Separator();
		ImGui::Text("avg fps: %f", fStats.avgfps);
		ImGui::Text("avg ms per frame: %f", fStats.avgmspf);
	ImGui::End();

	ImGuiWindowFlags rtWindowFlags = 0;// = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar;
	ImVec2 rtSize{ 640.0f, 360.0f };
	//ImGui::SetNextWindowSize(rtSize);
	ImGui::SetNextWindowBgAlpha(1.0f);
	ImGui::Begin("Viewport", 0, rtWindowFlags);
	{
		//https://github.com/ocornut/imgui/issues/2987
		ImGui::Image(reinterpret_cast<void*>(dxData.scnData.shaderResourceView), ImVec2{ 640.0f, 360.0f }, ImVec2{ 0,0 }, ImVec2{ 1,1 });
	}
	ImGui::End();
	ImGui::Render();	
	
	ID3D11RenderTargetView * views[1];
	views[0] = dxData.renderTargetView;
	//views[1] = dxData.textureRTView;
	dxData.imDeviceContext->OMSetRenderTargets(1, views, dxData.depthStencilView);
	//dxData.imDeviceContext->OMSetRenderTargets(1, &dxData.textureRTView, dxData.depthStencilView);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	
}

void InitializeCamera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 target, DirectX::XMFLOAT3 up, float fov, float aspectRatio, CameraData* camData)
{
	// camera position, target and up vector in 3D world
	DirectX::XMVECTOR cEye = DirectX::XMVectorSet(position.x, position.y, position.z, 0.0f);
	DirectX::XMVECTOR cFocus = DirectX::XMVectorSet(target.x, target.y, target.z, 0.0f);
	DirectX::XMVECTOR cUp = DirectX::XMVectorSet(up.x, up.y, up.z, 0.0f);
	
	// build view matrix
	DirectX::XMMATRIX mView = DirectX::XMMatrixLookAtLH(cEye, cFocus, cUp);

	// frustum data
	float yFov = DirectX::XMConvertToRadians(fov);
	float nearZ = 0.1f;
	float farZ = 1000.0f;
	DirectX::XMMATRIX mProj = DirectX::XMMatrixPerspectiveFovLH(yFov, aspectRatio, nearZ, farZ);

	// world mat
	DirectX::XMMATRIX mWorld = DirectX::XMMatrixIdentity();

	camData->mWorld = mWorld;
	camData->mView = mView;
	camData->mProj = mProj;
}

void InitializeCBuffer(CameraData & camData, DX11Data * dxData, ConstantBuffer * cbuffer) {
	//Simple translation followed by the camera view and projection
	//Note(Fran): Camera world now is identity but maybe we should multiply it aswell for the future
	//transpose?
	DirectX::XMMATRIX mWorld = DirectX::XMMatrixTranslation(0.0f, 0.0f, 2.0f);
	DirectX::XMMATRIX mWVP = DirectX::XMMatrixTranspose(mWorld * camData.mWorld * camData.mView * camData.mProj);
	*cbuffer = { 
		mWorld,
		mWVP
	};

	D3D11_BUFFER_DESC cbdesc{ 0 };
	cbdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbdesc.Usage = D3D11_USAGE_DEFAULT;// D3D11_USAGE_DYNAMIC;
	cbdesc.CPUAccessFlags = 0;//D3D11_CPU_ACCESS_WRITE;
	cbdesc.ByteWidth = sizeof(*cbuffer);
	D3D11_SUBRESOURCE_DATA csd{};
	csd.pSysMem = cbuffer;
	dxData->device->CreateBuffer(&cbdesc, &csd, &dxData->dx11_cbuffer);
	
}

// TODO(Fran): Check why this is not updating the constant buffer
void UpdateCBuffer(const CameraData & camData,float deltarot, float rotaxis[3], ConstantBuffer * cbuffer) {
	
	float anim = DirectX::XMConvertToRadians(deltarot);
	
	// triangle transformations/ world matrix basically rotate around arbitrary axis with arbitrary speed
	//DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0.0f, 0.0f, 2.0f);
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationAxis({ rotaxis[0], rotaxis[1], rotaxis[2], 0.0f }, anim);

	DirectX::XMMATRIX currentWorld = rotationMatrix;// *translation;

	DirectX::XMMATRIX mWVP = DirectX::XMMatrixTranspose(currentWorld * camData.mView * camData.mProj);

	*cbuffer = {
		currentWorld,
		mWVP
	};
}

void DrawScene(float rotVelocity,CameraData * camData, ConstantBuffer * cbuffer, IMData * imData, DX11Data & dxData, DX11VertexShaderData & vsData, DX11PixelShaderData & psData, BufferData & vb, BufferData & ib, FrameStats & fStats)
{
	//clear backbuffer
	DirectX::XMVECTORF32 clearColor_mw{ 1.0f, 0.5f, 0.0f, 1.0f };
	DirectX::XMVECTORF32 clearColor_sw{ 0.0f, 0.5f, 0.5f, 1.0f };

	// VIEWPORT RENDERING
	dxData.imDeviceContext->OMSetRenderTargets(1, &dxData.scnData.renderTargetView, dxData.scnData.depthStencilView);
	dxData.imDeviceContext->RSSetViewports(1, &dxData.scnData.viewport);

	dxData.imDeviceContext->ClearRenderTargetView(dxData.scnData.renderTargetView, reinterpret_cast<const float*>(&clearColor_sw));
	dxData.imDeviceContext->ClearDepthStencilView(dxData.scnData.depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// TRIANGLE RENDERING
	// Bind shaders and buffers
	dxData.imDeviceContext->IASetInputLayout(vsData.inputLayout);
	dxData.imDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxData.imDeviceContext->VSSetShader(vsData.shader, 0, 0);
	dxData.imDeviceContext->PSSetShader(psData.shader, 0, 0);
	dxData.imDeviceContext->IASetVertexBuffers(0, 1, &vb.buffer, &vb.stride, &vb.offset);
	dxData.imDeviceContext->IASetIndexBuffer(ib.buffer, DXGI_FORMAT_R32_UINT, ib.offset);
	

	//CBUFFER
	// Update constant buffer
	UpdateCBuffer(*camData, rotVelocity, imData->rot, cbuffer);
	dxData.imDeviceContext->UpdateSubresource(dxData.dx11_cbuffer, 0, 0, cbuffer, 0, 0);
	dxData.imDeviceContext->VSSetConstantBuffers(0, 1, &dxData.dx11_cbuffer);

	dxData.imDeviceContext->DrawIndexed(36, 0, 0);


	// MAIN WINDOW RENDERING
	dxData.imDeviceContext->OMSetRenderTargets(1, &dxData.renderTargetView, dxData.depthStencilView);
	dxData.imDeviceContext->RSSetViewports(1, &dxData.windowViewport);

	dxData.imDeviceContext->ClearRenderTargetView(dxData.renderTargetView, reinterpret_cast<const float*>(&clearColor_mw));
	dxData.imDeviceContext->ClearDepthStencilView(dxData.depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//// TRIANGLE RENDERING

	dxData.imDeviceContext->DrawIndexed(36, 0, 0);
	
	// GUI RENDERING
	DrawGUI(dxData, imData, fStats);

	
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
#ifdef _DEBUG 
	printf("::\tCGraph console log!\n");
#endif
	// Initialize and reset the time information for the application
	TimeData Time;
	FrameStats frameStats{ 0 };
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
	imIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
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

	float aspectRatio = dxData.windowViewport.Width / dxData.windowViewport.Height;
	CameraData camData{};
	InitializeCamera({ 0.0f, 0.0f, -2.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, 65.0f, aspectRatio, &camData);
	ConstantBuffer cbuffer{};
	InitializeCBuffer(camData, &dxData, &cbuffer);

	IMData imData{};
	//this is for testing purposes;
	
	float rotVelocity = 0.0f;
	float dt = 0.0f;
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
			CalculateFrameStats(Time, &frameStats);
			
			dt = (float)Time.deltaTime;
			//do the update and render
			UpdateScene(dt);

			rotVelocity += imData.rotSpeed * dt;

			DrawScene(rotVelocity, &camData, &cbuffer, &imData, dxData, vsData, psData, vertexBuff, indexBuff, frameStats);
			
		}
	}

	return 0;
}

// Note(Fran): Dummy main to attach to console when subsystem:console is specified.
int main()
{
	return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLineW(), SW_SHOWNORMAL);
}