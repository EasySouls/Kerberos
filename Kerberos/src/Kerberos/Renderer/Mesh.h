#pragma once

#include "Vertex.h"
#include "VertexArray.h"

namespace Kerberos
{
	class Mesh
	{
	public:
		Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		explicit Mesh(const std::string& filePath);
		~Mesh() = default;

		static Ref<Mesh> CreateCube(float size);
		Ref<Mesh> CreateSphere(float radius, uint32_t rings, uint32_t sectors);
	private:
		Ref<VertexArray> m_VertexArray;
	};
}

