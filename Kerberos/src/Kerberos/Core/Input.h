#pragma once

#include "Kerberos/Core/KeyCodes.h"
#include "Kerberos/Core/MouseButtonCodes.h"
#include "glm/glm.hpp"

namespace Kerberos
{
	class Input
	{
	public:
		static bool IsKeyPressed(const KeyCode keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
		static bool IsMouseButtonPressed(const MouseButtonCode button) { return s_Instance->IsMouseButtonPressedImpl(button); }
		static glm::vec2 GetMousePosition() { return s_Instance->GetMousePositionImpl(); }
		static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
		static float GetMouseY() { return s_Instance->GetMouseYImpl(); }

		Input() = default;
		virtual ~Input() = default;
		
		Input(const Input& other) = delete;
		Input(Input&& other) noexcept = delete;

		Input& operator=(const Input& other) = delete;

		Input& operator=(Input&& other) noexcept = delete;

	protected:
		virtual bool IsKeyPressedImpl(KeyCode keycode) = 0;
		virtual bool IsMouseButtonPressedImpl(MouseButtonCode button) = 0;
		virtual glm::vec2 GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;

	private:
		static Input* s_Instance;
	};
}