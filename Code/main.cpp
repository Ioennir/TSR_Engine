#include "tsr_platform.cpp"
#include "tsr_gui.h"
#include "tsr_eastl.h"
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include "tsr_types.cpp"

//MACROS
#include "tsr_macros.h"

// DX11 layer
#include "tsr_dx11.cpp"

// profiling layer
#include "tsr_profiling.cpp"
#include "tsr_assets.cpp"
#include "tsr_rendering.cpp"

//TODO(Fran): Implement a naive input system, maybe winsdk has something

#if (defined(_WIN64) && _WIN64)
INT WINAPI wWinMain(
	_In_ HINSTANCE hInstance, 
	_In_opt_ HINSTANCE hPrevInstance, 
	_In_ LPWSTR lpCmdLine, 
	_In_ int nCmdShow)
{
#ifdef _DEBUG 
	printf("::\tTSR engine console log!\n");
	//check this works
	eastl::vector<float> v = {0.0f, 1.0f, 2.0f, 3.0f};
	eastl::string s = "EASTL GOING!\n";
	printf("%s", s.c_str());
	
#endif
	// Load vivi
	RenderData renderData;
	eastl::string path = "..\\..\\..\\MODELS\\vivi.obj";
	LoadSimpleMesh(path, &renderData);
	


	// Initialize and reset the time information for the application
	TimeData Time;
	FrameStats frameStats{ 0 };
	ResetTimeInformation(&Time);

	// create the window and display it.
	RECT wRect { 0, 0, 1280, 720 };


	HWND wHandler = CreateAndSpawnWindow(L"TSR Engine", wRect, hInstance, nCmdShow);


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

	BufferData primitiveVertexBuff;
	BufferData primitiveIndexBuff;

	
	if (!BuildTriangleShaders(*dxData.device, &vsData, &psData))
	{
		return -1;
	}

	if(!BuildGeometryBuffer(*dxData.device, renderData, &vertexBuff, &indexBuff))
	{
		return -1;
	}

	BuildPrimitiveBuffers(Primitive::Cube, *dxData.device, &primitiveVertexBuff, &primitiveIndexBuff);
	
	/*
	if (!BuildTriangleGeometryBuffers(*dxData.device, &vertexBuff, &indexBuff))
	{
		return -1;
	}
	*/

	float aspectRatio = dxData.windowViewport.Width / dxData.windowViewport.Height;
	CameraData camData{};
	InitializeCamera({ 0.0f, 0.0f, -5.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, 65.0f, aspectRatio, &camData);
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
			//SCENE UPDATE
			TSR_Update(dt);

			rotVelocity += imData.rotSpeed * dt;

			// SCENE RENDERING
			TSR_Draw(rotVelocity, &camData, &cbuffer, &imData, dxData, vsData, psData, vertexBuff, indexBuff, &renderData, primitiveVertexBuff, primitiveIndexBuff);
			// GUI RENDERING
			TSR_DrawGUI(dxData, &imData, frameStats);

			dxData.swapChain->Present(NO_VSYNC, 0);
		}
	}

	return 0;
}
#endif

// Note(Fran): Dummy main to attach to console when subsystem:console is specified.
int main()
{
#if (defined(_WIN64) && _WIN64)
	return wWinMain(GetModuleHandle(NULL), NULL, GetCommandLineW(), SW_SHOWNORMAL);
#endif
}