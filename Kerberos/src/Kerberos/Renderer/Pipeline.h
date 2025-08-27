#pragma once

#include "Buffer.h"
#include "Framebuffer.h"
#include "Shader.h"

namespace Kerberos
{
	class Pipeline
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

			Topology PrimitiveTopology = Topology::Triangles;

			CullMode CullMode = CullMode::Back;

			WindingOrder FrontFace = WindingOrder::CounterClockwise;

			DepthTest DepthTest = DepthTest::None;
		};

		virtual ~Pipeline() = default;

		virtual const PipelineSpecification& GetSpecification() const = 0;

		static Ref<Pipeline> Create(const PipelineSpecification& spec);
	};
}
