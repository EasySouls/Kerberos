#pragma once

namespace Kerberos
{
	enum class RendererAPI : uint8_t
	{
		None = 0, 
		OpenGL = 1,
		Vulkan = 2
	};

	class Renderer
	{
	public:
		inline static RendererAPI GetAPI() { return s_API; }

	private:
		static RendererAPI s_API;
	};
}