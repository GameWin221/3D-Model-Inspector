#include "Application.h"

int main()
{
	Application::Init();

	while (Application::isRunning())
	{
		Application::Update();
		Application::Render();
	}

	Application::Exit();
}