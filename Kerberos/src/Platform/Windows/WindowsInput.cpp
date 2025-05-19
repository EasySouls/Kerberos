#include "kbrpch.h"
#include "WindowsInput.h"
#include "Kerberos/Application.h"

#include <GLFW/glfw3.h>

namespace Kerberos
{
	Input* Input::s_Instance = new WindowsInput();

	bool WindowsInput::IsKeyPressedImpl(const KeyCode keycode)
	{
		const auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		const auto state = glfwGetKey(window, static_cast<int32_t>(keycode));

		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool WindowsInput::IsMouseButtonPressedImpl(const MouseButtonCode button) 
	{
		const auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		const auto state = glfwGetMouseButton(window, static_cast<int8_t>(button));

		return state == GLFW_PRESS;
	}

	glm::vec2 WindowsInput::GetMousePositionImpl() 
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
		const auto pos = GetMousePositionImpl();
		return pos.x;
	}

	float WindowsInput::GetMouseYImpl() 
	{
		const auto pos = GetMousePositionImpl();
		return pos.y;
	}
}
