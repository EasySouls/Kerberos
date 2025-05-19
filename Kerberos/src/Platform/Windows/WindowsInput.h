#pragma once

#include "Kerberos/Core/Input.h"

namespace Kerberos
{
	class WindowsInput final : public Input
	{
	protected:
		bool IsKeyPressedImpl(KeyCode keycode) override;
		bool IsMouseButtonPressedImpl(MouseButtonCode button) override;
		glm::vec2 GetMousePositionImpl() override;
		float GetMouseXImpl() override;
		float GetMouseYImpl() override;
	};

}
