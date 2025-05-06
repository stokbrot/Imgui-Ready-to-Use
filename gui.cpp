#include "gui.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND hWnd, 
	UINT msg, 
	WPARAM wParam, 
	LPARAM lParam
);

LRESULT WINAPI/*long __stdcall*/ WinProcess(
	HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam
)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
        case WM_SIZE:
            if (gui::device && wParam == SIZE_MINIMIZED)
                return 0;

            gui::presentParameters.BackBufferWidth = (UINT)LOWORD(lParam); // Queue resize
            gui::presentParameters.BackBufferHeight = (UINT)HIWORD(lParam);
            return 0;

        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
				break;
				
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

		case WM_CLOSE:
			gui::running = false;  // Set your running flag to false
			DestroyWindow(hWnd);   // Explicitly destroy the window
			return 0;

        case WM_LBUTTONDOWN:
            gui::position = MAKEPOINTS(lParam);
            return 0;

		case WM_MOUSEMOVE:
		{
			

			if (wParam & (MK_LBUTTON | MK_RBUTTON))
			{
				const POINTS pt = MAKEPOINTS(lParam);
				auto rect = ::RECT{};

				GetWindowRect(gui::window, &rect);

				rect.left += pt.x - gui::position.x;
				rect.top += pt.y - gui::position.y;


				if (gui::position.x >= 0 &&
					gui::position.y >= 0 &&
					gui::position.x <= gui::WIDTH &&
					gui::position.y <= 19 //Top bar
				) 
				{
					SetWindowPos(
						gui::window,
						HWND_TOPMOST,
						rect.left,
						rect.top,
						0, 0,
						SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER
					);			
				}
				
			}
			
			break;

		}

    }

	return DefWindowProcW(hWnd, msg, wParam, lParam);
}

void gui::Create_Window(const wchar_t* windowName, const wchar_t* className) noexcept
{
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = WinProcess;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandleW(0);
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = className;
	wc.hIconSm = nullptr;

	RegisterClassExW(&wc);

	window = CreateWindowW(
		className, windowName,
		WS_POPUP,
		100, 100, // spawn location
		WIDTH, HEIGHT,
		nullptr, nullptr, wc.hInstance, nullptr);

	if (!window) {
        MessageBoxA(0, "Window Creation Failed!", "Error", MB_ICONERROR);
        return;
    }

	CreateDevice();
	CreateImGui();
	
}

void gui::Destroy_Window() noexcept
{
	DestroyWindow(window);
	UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

bool gui::CreateDevice() noexcept
{
	if ((d3d = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
		return false;

	// Create the D3DDevice
	ZeroMemory(&presentParameters, sizeof(presentParameters));
	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	if (d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, window, D3DCREATE_HARDWARE_VERTEXPROCESSING, &presentParameters, &device) < 0)
		return false;

	return true;
}

void gui::ResetDevice() noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT hr = device->Reset(&presentParameters);
	if (hr == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}



void gui::DestroyDevice() noexcept
{
	if (device) { device->Release(); device = nullptr; }
	if (d3d) { d3d->Release(); d3d = nullptr; }
}

void gui::CreateImGui() noexcept
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); 
	io.IniFilename = NULL;

	      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
}

void gui::DestroyImGui() noexcept
{
	// Cleanup
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	DestroyDevice();
	Destroy_Window();
}

void gui::BeginRender() noexcept
{
	MSG msg;
	while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void gui::EndRender() noexcept
{
	// Rendering
	ImGui::EndFrame();
	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	
	device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

	if (device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}
	HRESULT result = device->Present(nullptr, nullptr, nullptr, nullptr);
	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();

	ShowWindow(gui::window, SW_SHOWDEFAULT);
	UpdateWindow(gui::window);
}

static bool checkbox_value = false;
static int slider_value = 50;
static float float_value = 0.5f;
static char text_input[128] = "Default Text";
static int radio_selection = 0;
static bool show_another_window = false;
static float color[3] = { 1.0f, 0.0f, 0.0f }; // Red as default color (RGB)


bool gui::GetCheckboxValue() { return checkbox_value; }
int gui::GetSliderValue() { return slider_value; }
float gui::GetFloatValue() { return float_value; }
const char* gui::GetTextInput() { return text_input; }
int gui::GetRadioSelection() { return radio_selection; }
const float* gui::GetColorSelection() { return color; }


void gui::Render() noexcept
{


	ImGui::SetNextWindowPos({ 0,0 });
	ImGui::SetNextWindowSize({ WIDTH,HEIGHT });
	ImGui::Begin(
		"ImGui Template",
		&running,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove);

	ImGui::Text("This is a basic ImGui template.");

	// Checkbox
	ImGui::Checkbox("Enable Feature", &checkbox_value);

	// Slider
	ImGui::SliderInt("Value", &slider_value, 0, 100);

	// Float Input
	ImGui::SliderFloat("Float Value", &float_value, 0.0f, 1.0f);

	// Text Input
	ImGui::InputText("Enter Text", text_input, IM_ARRAYSIZE(text_input));

	// Radio Buttons
	ImGui::Text("Choose an option:");
	ImGui::RadioButton("Option 1", &radio_selection, 0);
	ImGui::SameLine();
	ImGui::RadioButton("Option 2", &radio_selection, 1);
	ImGui::SameLine();
	ImGui::RadioButton("Option 3", &radio_selection, 2);

	// Button
	if (ImGui::Button("Open Window"))
		show_another_window = true;

	// Additional Window
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello from another window!");		
		ImGui::End();
	}

	//color picker
	ImGui::ColorEdit3("Pick Color", color);
	

	ImGui::End();
}
