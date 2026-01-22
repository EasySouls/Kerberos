#include "kbrpch.h"
#include "OpenGLCommandList.h"

#include "Kerberos/Renderer/GraphicsPipeline.h"

#include <glad/glad.h>

namespace Kerberos
{
	void OpenGLCommandList::Begin()
	{
		// OpenGL does not require explicit command buffer begin
	}
	void OpenGLCommandList::End()
	{
		// OpenGL does not require explicit command buffer end
	}
	void OpenGLCommandList::SetViewport(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height)
	{
		glViewport(static_cast<int>(x), static_cast<int>(y), static_cast<int>(width), static_cast<int>(height));
	}
	void OpenGLCommandList::SetScissorRect(const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height)
	{
		glScissor(static_cast<int>(x), static_cast<int>(y), static_cast<int>(width), static_cast<int>(height));
	}
	void OpenGLCommandList::BindPipeline(const Ref<GraphicsPipeline>& pipeline)
	{
		pipeline->Bind();
	}

	void OpenGLCommandList::BindVertexBuffer(const Ref<VertexBuffer>& vertexBuffer, uint32_t slot) 
	{
		vertexBuffer->Bind();
	}

	void OpenGLCommandList::BindIndexBuffer(const Ref<IndexBuffer>& indexBuffer) 
	{
		indexBuffer->Bind();
	}

	void OpenGLCommandList::DrawIndexed(const uint32_t indexCount, const uint32_t startIndexLocation, const int32_t baseVertexLocation) 
	{
		glDrawElementsBaseVertex(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, reinterpret_cast<void*>(startIndexLocation * sizeof(uint32_t)), baseVertexLocation);
	}

	void OpenGLCommandList::DrawVertexCount(const uint32_t vertexCount, const uint32_t startVertexLocation) 
	{
		glDrawArrays(GL_TRIANGLES, startVertexLocation, vertexCount);
	}
}
