
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

HWND CreateAndSpawnWindow(LPCWSTR winName, RECT wRect, HINSTANCE hInstance, int nCmdShow)
{
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

	return wHandler;
}

void FetchPerformanceCounter(long long* performanceCounter)
{
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(performanceCounter));
}

void FetchPerformanceFreq(long long* performanceFreq)
{
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(performanceFreq));
}

#endif