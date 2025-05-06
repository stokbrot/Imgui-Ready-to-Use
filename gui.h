#pragma comment(lib, "d3d9.lib")
#define _MBCS  // Enable Multi-Byte Character Set
#pragma once

#include <d3d9.h>



namespace gui
{

	constexpr int WIDTH = 500;
	constexpr int HEIGHT= 500;

	inline bool running = true;

	// winapi
	inline HWND window = nullptr;
	inline WNDCLASSEXW wc = {};

	// window movement
	inline POINTS position = {  };

	// directx 
	inline PDIRECT3D9 d3d = nullptr;
	inline LPDIRECT3DDEVICE9 device = nullptr;
	inline D3DPRESENT_PARAMETERS presentParameters = {};

	void Create_Window(
		const wchar_t* windowName,
		const wchar_t* className) noexcept;

	void Destroy_Window() noexcept;



	
	bool CreateDevice() noexcept;
	
	void ResetDevice() noexcept;
	void DestroyDevice() noexcept;


	void CreateImGui() noexcept;
	void DestroyImGui() noexcept;

	void BeginRender() noexcept;
	void EndRender() noexcept;
	void Render() noexcept;

	//Getter functions:
	bool GetCheckboxValue();
	int GetSliderValue();
	float GetFloatValue();
	const char* GetTextInput();
	int GetRadioSelection();
	const float* GetColorSelection();
};

