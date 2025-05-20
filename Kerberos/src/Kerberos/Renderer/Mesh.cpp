#include "kbrpch.h"
#include "Mesh.h"

#include <glm/ext/scalar_constants.hpp>

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

		Ref<Mesh> Mesh::CreateSphere(const float radius, const uint32_t sectorCount, const uint32_t stackCount)
		{
			std::vector<Vertex> vertices;
			std::vector<uint32_t> indices;

			const float lengthInv = 1.0f / radius;

			const float sectorStep = 2 * glm::pi<float>() / static_cast<float>(sectorCount);
			const float stackStep = glm::pi<float>() / static_cast<float>(stackCount);

			for (uint32_t i = 0; i <= stackCount; ++i)
			{
				const float stackAngle = glm::pi<float>() / 2 - i * stackStep; // starting from pi/2 to -pi/2
				const float xy = radius * cosf(stackAngle);             // r * cos(u)
				float z = radius * sinf(stackAngle);              // r * sin(u)

				// add (sectorCount+1) vertices per stack
				// the first and last vertices have same position and normal, but different tex coords
				for (uint32_t j = 0; j <= sectorCount; ++j)
				{
					const float sectorAngle = j * sectorStep;           // starting from 0 to 2pi

					// vertex position (x, y, z)
					float x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
					float y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)

					// normalized vertex normal (nx, ny, nz)
					float nx = x * lengthInv;
					float ny = y * lengthInv;
					float nz = z * lengthInv;

					// vertex tex coord (s, t) range between [0, 1]
					float s = static_cast<float>(j) / static_cast<float>(sectorCount);
					float t = static_cast<float>(i) / static_cast<float>(stackCount);

					vertices.push_back({.Position = {x, y, z}, .Normal = {nx, ny, nz}, .TexCoord = {s, t} });
				}
			}

			// Generate indices for quads (2 triangles per quad)
			// k1--k1+1
			// |  / |
			// | /  |
			// k2--k2+1
			for (uint32_t i = 0; i < stackCount; ++i)
			{
				uint32_t k1 = i * (sectorCount + 1);     // beginning of current stack
				uint32_t k2 = k1 + sectorCount + 1;      // beginning of next stack

				for (uint32_t j = 0; j < sectorCount; ++j, ++k1, ++k2)
				{
					// 2 triangles per sector excluding first and last stacks
					// k1 => current stack
					// k2 => next stack
					if (i != 0)
					{
						indices.push_back(k1);
						indices.push_back(k2);
						indices.push_back(k1 + 1);
					}

					if (i != (stackCount - 1))
					{
						indices.push_back(k1 + 1);
						indices.push_back(k2);
						indices.push_back(k2 + 1);
					}
				}
			}

			return CreateRef<Mesh>(vertices, indices);
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
