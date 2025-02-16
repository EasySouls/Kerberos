#include "kbrpch.h"
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Kerberos
{
	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI();
}