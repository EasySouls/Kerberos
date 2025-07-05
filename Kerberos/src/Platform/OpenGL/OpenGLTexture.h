#pragma once

#include "glad/glad.h"
#include "Kerberos/Renderer/Texture.h"

namespace Kerberos
{
	class OpenGLTexture2D : public Texture2D
	{
	public:
		explicit OpenGLTexture2D(const std::string& path);
		explicit OpenGLTexture2D(const TextureSpecification& spec, Buffer data);
		~OpenGLTexture2D() override;

		uint32_t GetWidth() const override { return m_Spec.Width; }
		uint32_t GetHeight() const override { return m_Spec.Height; }
		const TextureSpecification& GetSpecification() const override { return m_Spec; }

		uint64_t GetRendererID() const override { return m_RendererID; }
		
		void Bind(uint32_t slot = 0) const override;

		void SetData(void* data, uint32_t size) override;

		bool operator==(const Texture& other) const override
		{
			return m_RendererID == dynamic_cast<const OpenGLTexture2D&>(other).m_RendererID;
		}

		void SetDebugName(const std::string& name) override;

	private:
		std::string m_Path;
		TextureSpecification m_Spec;
		uint32_t m_RendererID;

		GLenum m_InternalFormat;
		GLenum m_DataFormat;
	};
}
