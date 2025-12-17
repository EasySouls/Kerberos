#pragma once

#include "Kerberos/Core.h"

namespace Kerberos
{
	enum class FramebufferTextureFormat : uint8_t
	{
		None = 0,

		/// Color
		RGBA8 = 1,

		RED_INTEGER = 2, 

		/// Depth and stencil
		DEPTH24STENCIL8 = 3,

		DEPTH24 = 4,

		DEPTH32 = 5,
		
		Depth = DEPTH24STENCIL8,
	};

	static constexpr bool IsDepthFormat(const FramebufferTextureFormat format)
	{
		return format == FramebufferTextureFormat::DEPTH24STENCIL8 
			|| format == FramebufferTextureFormat::DEPTH24
			|| format == FramebufferTextureFormat::DEPTH32;
	}

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
		glm::vec4 ClearColor = { 0.1f, 0.1f, 0.1f, 1.0f };
		glm::vec4 DepthClearValue = { 1.0f, 0.0f, 0.0f, 0.0f };
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

		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;
		virtual void ClearDepthAttachment(float value) const = 0;

		virtual void BindColorTexture(uint32_t slot, uint32_t index = 0) const = 0;
		virtual void BindDepthTexture(uint32_t slot) const = 0;

		virtual uint64_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;
		virtual uint64_t GetDepthAttachmentRendererID() const = 0;

		virtual FramebufferSpecification& GetSpecification() = 0;
		virtual const FramebufferSpecification& GetSpecification() const = 0;
		
		virtual void SetDebugName(const std::string& name) const = 0;

		template<typename T>
		T& As()
		{
			return *static_cast<T*>(this);
		}

		static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
	};
}

