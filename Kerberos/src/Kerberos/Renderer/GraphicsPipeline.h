#pragma once

#include "Buffer.h"
#include "Framebuffer.h"
#include "Shader.h"

namespace Kerberos
{
	class GraphicsPipeline
	{
	public:
		enum class Topology : uint8_t
		{
			Triangles = 0, Lines, LineStrip
		};

		enum class CullMode : uint8_t
		{
			None = 0, Front, Back, FrontAndBack
		};

		enum class WindingOrder : uint8_t
		{
			Clockwise = 0, CounterClockwise
		};

		enum class DepthTest : uint8_t
		{
			None = 0, Less, LessEqual, Equal, Greater, GreaterEqual, NotEqual, Always, Never
		};

		struct PipelineSpecification
		{
			std::string Name;

			Ref<Shader> Shader;

			Ref<Framebuffer> TargetFramebuffer;

			BufferLayout Layout;

			bool Wireframe = false;

			Topology PrimitiveTopology = Topology::Triangles;

			CullMode CullMode = CullMode::Back;

			WindingOrder FrontFace = WindingOrder::CounterClockwise;

			DepthTest DepthTest = DepthTest::None;
		};

		virtual ~GraphicsPipeline() = default;

		virtual void Bind() const = 0;

		virtual const PipelineSpecification& GetSpecification() const = 0;

		virtual Ref<Shader> GetShader() const { return GetSpecification().Shader; }
		virtual Ref<Framebuffer> GetTargetFramebuffer() const { return GetSpecification().TargetFramebuffer; }

		template<typename T>
		T& As()
		{
			return *static_cast<T*>(this);
		}

		static Ref<GraphicsPipeline> Create(const PipelineSpecification& spec);
	};
}
