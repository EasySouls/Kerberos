#pragma once

#include "Vertex.h"
#include "VertexArray.h"
#include "Kerberos/Assets/Asset.h"

namespace Kerberos
{
	class Mesh : public Asset
	{
	public:
		Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		explicit Mesh(const std::string& filePath);
		~Mesh() override = default;

		static Ref<Mesh> CreateCube(float size);
		static Ref<Mesh> CreateSphere(float radius, uint32_t sectorCount, uint32_t stackCount);
		static Ref<Mesh> CreateTetrahedron(float size);
		static Ref<Mesh> CreatePlane(float size);
		static Ref<Mesh> CreatePlane(float width, float height);

		Ref<VertexArray> GetVertexArray() const { return m_VertexArray; }
		uint32_t GetIndexCount() const { return m_IndexCount; }
		uint32_t GetVertexCount() const 
		{ 
			if (m_VertexArray && !m_VertexArray->GetVertexBuffers().empty())
			{
				uint32_t count = 0;
				for (const auto& vertexBuffer : m_VertexArray->GetVertexBuffers())
				{
					count += vertexBuffer->GetCount();
				}
				return count;
			}
				
			return 0; 
		}

	private:
		void SetupMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

	private:
		Ref<VertexArray> m_VertexArray;
		uint32_t m_IndexCount = 0;
	};
}

