#include "kbrpch.h"
#include "Mesh.h"

namespace Kerberos
{
	Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) 
	{
		SetupMesh(vertices, indices);
	}

	Mesh::Mesh(const std::string& filePath) 
	{
		/// Load mesh from file
		/// call SetupMesh() with loaded vertices and indices
		
		throw std::runtime_error("Not implemented yet!");
	}

	Ref<Mesh> Mesh::CreateCube(const float size) 
	{
		std::vector<Vertex> vertices;

		constexpr glm::vec3 positions[] = {
			{ -1.0f, -1.0f, -1.0f}, {1.0f, -1.0f, -1.0f}, {1.0f,  1.0f, -1.0f}, {-1.0f,  1.0f, -1.0f},
		{ -1.0f, -1.0f,  1.0f }, { 1.0f, -1.0f,  1.0f }, { 1.0f,  1.0f,  1.0f }, { -1.0f,  1.0f,  1.0f}
		};

		constexpr glm::vec3 normals[] = {
			{0,0,1}, {0,0,-1}, {1,0,0}, {-1,0,0},
			{0,1,0}, {0,-1,0}
		};

		constexpr glm::vec2 texCoords[] = {
			{0,0}, {1,0}, {1,1}, {0,1},
		};

		const float halfSize = size * 0.5f;

		// Front face
		vertices.push_back({ .Position = positions[0] * halfSize, .Normal = normals[0], .TexCoord = texCoords[0] });
		vertices.push_back({ .Position = positions[1] * halfSize, .Normal = normals[0], .TexCoord = texCoords[1] });
		vertices.push_back({ .Position = positions[2] * halfSize, .Normal = normals[0], .TexCoord = texCoords[2] });
		vertices.push_back({ .Position = positions[3] * halfSize, .Normal = normals[0], .TexCoord = texCoords[3] });

		// Back face
		vertices.push_back({ .Position = positions[4] * halfSize, .Normal = normals[1], .TexCoord = texCoords[0] });
		vertices.push_back({ .Position = positions[5] * halfSize, .Normal = normals[1], .TexCoord = texCoords[1] });
		vertices.push_back({ .Position = positions[6] * halfSize, .Normal = normals[1], .TexCoord = texCoords[2] });
		vertices.push_back({ .Position = positions[7] * halfSize, .Normal = normals[1], .TexCoord = texCoords[3] });

		// Top face
		vertices.push_back({ .Position = positions[3] * halfSize, .Normal = normals[4], .TexCoord = texCoords[0] });
		vertices.push_back({ .Position = positions[2] * halfSize, .Normal = normals[4], .TexCoord = texCoords[1] });
		vertices.push_back({ .Position = positions[6] * halfSize, .Normal = normals[4], .TexCoord = texCoords[2] });
		vertices.push_back({ .Position = positions[7] * halfSize, .Normal = normals[4], .TexCoord = texCoords[3] });


		// Bottom face
		vertices.push_back({ .Position = positions[0] * halfSize, .Normal = normals[5], .TexCoord = texCoords[0] });
		vertices.push_back({ .Position = positions[1] * halfSize, .Normal = normals[5], .TexCoord = texCoords[1] });
		vertices.push_back({ .Position = positions[5] * halfSize, .Normal = normals[5], .TexCoord = texCoords[2] });
		vertices.push_back({ .Position = positions[4] * halfSize, .Normal = normals[5], .TexCoord = texCoords[3] });

		// Right face
		vertices.push_back({ .Position = positions[1] * halfSize, .Normal = normals[2], .TexCoord = texCoords[0] });
		vertices.push_back({ .Position = positions[5] * halfSize, .Normal = normals[2], .TexCoord = texCoords[1] });
		vertices.push_back({ .Position = positions[6] * halfSize, .Normal = normals[2], .TexCoord = texCoords[2] });
		vertices.push_back({ .Position = positions[2] * halfSize, .Normal = normals[2], .TexCoord = texCoords[3] });

		// Left face
		vertices.push_back({ .Position = positions[0] * halfSize, .Normal = normals[3], .TexCoord = texCoords[0] });
		vertices.push_back({ .Position = positions[3] * halfSize, .Normal = normals[3], .TexCoord = texCoords[1] });
		vertices.push_back({ .Position = positions[7] * halfSize, .Normal = normals[3], .TexCoord = texCoords[2] });
		vertices.push_back({ .Position = positions[4] * halfSize, .Normal = normals[3], .TexCoord = texCoords[3] });

		std::vector<uint32_t> indices;
		indices.reserve(6 * 2 * 3); /// 6 faces, 2 triangles per face, 3 indices per triangle
		for (uint32_t i = 0; i < 6; i++)
		{
			const uint32_t offset = i * 4;
			indices.push_back(offset + 0);
			indices.push_back(offset + 1);
			indices.push_back(offset + 2);
			indices.push_back(offset + 2);
			indices.push_back(offset + 3);
			indices.push_back(offset + 0);
		}

		return CreateRef<Mesh>(vertices, indices);
	}

	Ref<Mesh> Mesh::CreateSphere(float radius, uint32_t rings, uint32_t sectors) 
	{
		throw std::runtime_error("Not implemented yet!");
	}

	void Mesh::SetupMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) 
	{
		m_VertexArray = VertexArray::Create();

		const auto vertexBuffer = VertexBuffer::Create(vertices.size() * sizeof(Vertex));
		vertexBuffer->SetLayout(Vertex::GetLayout());
		vertexBuffer->SetData(vertices.data(), vertices.size() * sizeof(Vertex));
		m_VertexArray->AddVertexBuffer(vertexBuffer);

		const auto indexBuffer = IndexBuffer::Create(indices.data(), static_cast<uint32_t>(indices.size()));
		m_VertexArray->SetIndexBuffer(indexBuffer);

		m_IndexCount = static_cast<uint32_t>(indices.size());
	}
}
