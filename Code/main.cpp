// TYPES & MACROS
#include "tsr_types.cpp"
#include "tsr_macros.h"

// EASTL
#include "tsr_eastl.h" 
// TODO(Fran): Maybe check in the future to fix these warnings myself and recompile eastl.
#pragma warning(push)
#pragma warning(disable:26495)
#include <EASTL/string.h>
#include <EASTL/vector.h>
#pragma warning(pop)

// PLATFORM SPECIFIC
#include "platform/tsr_platform.cpp"

// GUI INDEPENDENT CODE
#include "tsr_gui.h"

struct MaterialMapNames
{
	eastl::string diffuse;
	eastl::string metallic;
	eastl::string roughness;
	eastl::string normal;
	eastl::string emissive;
	eastl::string opacity;
};

// DX11 layer
#include "tsr_dx11.cpp"

// profiling layer
#include "tsr_profiling.cpp"

// assimp
// TODO(Fran): Maybe check in the future to fix these warnings myself and recompile assimp.
#pragma warning(push)
#pragma warning(disable:26451)
#pragma warning(disable:26812)
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma warning(pop)

#include "tsr_assets.cpp"

#include "tsr_rendering.cpp"
#include "tsr_primitives.cpp"


//TODO(Fran): Implement a naive input system, maybe winsdk has something or the Imgui input system itself.

#if (defined(_WIN64) && _WIN64)
INT WINAPI wWinMain(
	_In_ HINSTANCE hInstance, 
	_In_opt_ HINSTANCE hPrevInstance, 
	_In_ LPWSTR lpCmdLine, 
	_In_ int nCmdShow)
{
	// Initialize and reset the time information for the application
	TimeData Time;
	FrameStats frameStats{ 0 };
	ResetTimeInformation(&Time);

	// create the window and display it.
	WindowData winData{};
	CreateAndSpawnWindow(L"TSR Engine", winData, hInstance, nCmdShow);

	// Initialize DX11 and get all the information needed
	DX11Data dxData{};
	TSR_DX11_Init(winData, &dxData);

	// setup Imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& imIO = ImGui::GetIO();
	imIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
	ImGui_ImplWin32_Init(winData.handle);
	ImGui_ImplDX11_Init(dxData.device, dxData.context);
	ImGui::StyleColorsDark();
	
	// Load vivi
	eastl::string path = "..\\..\\..\\MODELS\\vivi\\vivi_modified.fbx";
	DrawComponent drawable{};
	eastl::vector<MaterialMapNames> mapNames;
	TSR_LoadMeshFromPath(&drawable.model, mapNames, path);
	TSR_FillComponentVertexInput(&drawable);
	TSR_DX11_ImportTextures(dxData.device, mapNames);

	ModelBuffers buffers{};
	DX11VertexShaderData vsData;
	DX11PixelShaderData psData;

	TSR_DX11_BuildShaders(dxData.device, &vsData, &psData);
	TSR_DX11_BuildGeometryBuffersFromComponent(dxData.device, &drawable, &buffers);
	
	//Primitives
	//NOTE(Fran): This is a test to check on generating primitive data from cpu computation to gpu rendering.
	ModelBuffers primitiveBuffers{};
	TSR_DX11_BuildPrimitiveBuffers(Primitive::Sphere, dxData.device, &primitiveBuffers);
	

	float aspectRatio = dxData.VP.Viewport.Width / dxData.VP.Viewport.Height;
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
			
			dt = TYPECAST(r32, Time.deltaTime);
			//SCENE UPDATE
			TSR_Update(dt);

			rotVelocity += imData.rotSpeed * dt;

			// SCENE RENDERING
			TSR_Draw(rotVelocity, &camData, &cbuffer, &imData, dxData, vsData, psData, &buffers, &primitiveBuffers, &drawable);
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