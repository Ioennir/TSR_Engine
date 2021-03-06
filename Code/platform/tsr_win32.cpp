
#if (defined(_WIN64) && _WIN64)

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0); break;
	case WM_SIZING:
		// prevent the white thing from happening. done
		break;
	case WM_SIZE:
		// Get window size
		// Rebuild framebuffer
		// commit framebuffer
		break;

	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void CreateAndSpawnWindow(LPCWSTR winName, WindowData & winData, HINSTANCE hInstance, int nCmdShow)
{
	RECT wRect{ 0, 0, 1280, 720 };
	// create and register the class to spawn the window
	LPCWSTR wcName = L"CGraphWindowClass";
	WNDCLASSEX wclass{ 0 };
	wclass.cbSize = sizeof(WNDCLASSEX);
	wclass.style = CS_HREDRAW | CS_VREDRAW;
	wclass.lpfnWndProc = WndProc;
	wclass.hInstance = hInstance;
	wclass.hCursor = LoadCursor(0, IDC_ARROW);
	wclass.lpszClassName = wcName;
	RegisterClassEx(&wclass);

	DWORD wStyle;
	wStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME;

	// this calculates the window rect given the client rect
	AdjustWindowRect(&wRect, wStyle, FALSE);

	HWND wHandler = CreateWindow(
		wcName,
		winName, // window name
		wStyle,
		100, 100,//CW_USEDEFAULT, CW_USEDEFAULT, // X AND Y start positions for the window
		wRect.right - wRect.left,
		wRect.bottom - wRect.top,
		0, 0, hInstance, 0);

	// Show the window
	ShowWindow(wHandler, nCmdShow);
	winData.handle = PTRCAST(void*, wHandler); // cast to void* to be platform agnostic
	//NOTE(Fran): check this out
	winData.width = 1280;//wRect.right - wRect.left;
	winData.height = 720;//wRect.bottom - wRect.top;
}

void FetchPerformanceCounter(long long* PerformanceCounter)
{
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(PerformanceCounter));
}

void FetchPerformanceFreq(long long* PerformanceFreq)
{
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(PerformanceFreq));
}

#endif