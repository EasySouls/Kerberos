#pragma once
#include <Kerberos.h>


class Sandbox2D : public Kerberos::Layer
{
public:
	Sandbox2D();
	~Sandbox2D() override = default;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(Kerberos::Timestep deltaTime) override;
	void OnImGuiRender() override;
	void OnEvent(Kerberos::Event& event) override;

private:
	Kerberos::OrthographicCameraController m_CameraController;

	glm::vec4 m_SquareColor = { 0.8f, 0.3f, 0.2f, 1.0f };
	Kerberos::Ref<Kerberos::Texture2D> m_Texture;

	struct ProfileResult
	{
		const char* Name;
		float Time;
	};

	float m_Fps = 0;

	std::vector<ProfileResult> m_ProfileResults;
};

