#include "main.h"

//Drag all files into Project folder and then also into Sulution Explorer
// -> Right Click and Include into Project !on everything

// In Project settings Change:
// Linker->System->WindowsSubsystem
// Configuration Properties->General->C++ Language Standart 20

//Entry Point

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR	  lpCmdLine,
	int		  nCmdShow)
{
	gui::Create_Window(L"*Placeholder*", L"class1");
	
	

	while (gui::running) {
		gui::BeginRender();
		gui::Render();
		// add gui Elements in: gui::Render()
		gui::EndRender();	
		std::this_thread::sleep_for(std::chrono::microseconds(1));

		//main loop
		//OutputDebugStringA("Printing like this...\n");


	}



	gui::DestroyImGui();
	

	return EXIT_SUCCESS;
}