#include "kbrpch.h"
#include "WindowsInput.h"
#include "Kerberos/Application.h"

#include <GLFW/glfw3.h>

namespace Kerberos
{
	Input* Input::s_Instance = new WindowsInput();

	bool WindowsInput::IsKeyPressedImpl(const int keycode)
	{
		const auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		const auto state = glfwGetKey(window, keycode);

		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool WindowsInput::IsMouseButtonPressedImpl(const int button) 
	{
		const auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		const auto state = glfwGetMouseButton(window, button);

		return state == GLFW_PRESS;
	}

	std::pair<float, float> WindowsInput::GetMousePositionImpl() 
	{
		const auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);
		
		float xPosF = static_cast<float>(xPos);
		float yPosF = static_cast<float>(yPos);

		return { xPosF, yPosF };
	}

	float WindowsInput::GetMouseXImpl() 
	{
		auto [x, y] = GetMousePositionImpl();
		return x;
	}

	float WindowsInput::GetMouseYImpl() 
	{
		auto [x, y] = GetMousePositionImpl();
		return y;
	}
}
