#pragma once

#include "Kerberos/Core.h"

namespace Kerberos
{
	enum class FramebufferTextureFormat : uint8_t
	{
		None = 0,

		/// Color
		RGBA8 = 1,

		/// Depth and stencil
		DEPTH24STENCIL8 = 2,
		
		Depth = DEPTH24STENCIL8,
	};

	struct FramebufferTextureSpecification
	{
		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;

		FramebufferTextureSpecification() = default;

		FramebufferTextureSpecification(const FramebufferTextureFormat format)
			: TextureFormat(format)
		{}
	};

	struct FramebufferAttachmentSpecification
	{
		std::vector<FramebufferTextureSpecification> Attachments;

		FramebufferAttachmentSpecification() = default;

		FramebufferAttachmentSpecification(const std::initializer_list<FramebufferTextureSpecification>& attachments)
			: Attachments(attachments)
		{}
	};

	struct FramebufferSpecification
	{
		uint32_t Width;
		uint32_t Height;
		FramebufferAttachmentSpecification Attachments;
		uint32_t Samples = 1;
		bool SwapChainTarget = false;
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual uint64_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;

		virtual FramebufferSpecification& GetSpecification() = 0;
		virtual const FramebufferSpecification& GetSpecification() const = 0;

		template<typename T>
		T& As()
		{
			return *static_cast<T*>(this);
		}

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	};
}

