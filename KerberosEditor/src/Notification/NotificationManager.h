#pragma once

#include <vector>
#include <string>
#include "Notification.h"

namespace Kerberos
{
	class NotificationManager
	{
	public:
		NotificationManager() = default;

		void AddNotification(const std::string& message, Notification::Type type, float duration = 10.0f, const glm::vec4& bgColor = {0.0f, 0.0f, 0.0f, 1.0f}, const glm::vec4& textColor = {1.0f, 1.0f, 1.0f, 1.0f});
		void RenderNotifications();
	
	private:
		std::vector<Notification> m_Notifications;
		uint32_t m_NextId = 0;
	};
}

