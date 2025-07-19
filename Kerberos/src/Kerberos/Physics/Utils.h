#pragma once

#include "Kerberos/Renderer/Vertex.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/Array.h>

#include "Jolt/Core/Reference.h"
#include "Jolt/Geometry/IndexedTriangle.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"
#include "Jolt/Physics/Collision/Shape/Shape.h"


namespace Kerberos::Physics
{
	class Utils
	{
	public:
		static JPH::Array<JPH::Vec3> KBRVerticesToJoltVertices(const std::vector<Vertex>& vertices)
		{
			JPH::Array<JPH::Vec3> joltVertices;
			joltVertices.reserve(vertices.size());
			for (const auto& vertex : vertices)
			{
				joltVertices.emplace_back(vertex.Position.x, vertex.Position.y, vertex.Position.z);
			}
			return joltVertices;
		}

        static JPH::Ref<JPH::Shape> CreateJoltMeshShape(const Ref<Mesh>& mesh, const std::string& debugName)
        {
            const std::vector<Vertex>& kerberosVertices = mesh->GetVertices();
            const std::vector<uint32_t>& kerberosIndices = mesh->GetIndices();

            JPH::VertexList jphVertices;
            jphVertices.reserve(kerberosVertices.size());
            for (const auto& vertex : kerberosVertices)
            {
                jphVertices.push_back(
                    JPH::Float3(vertex.Position.x, vertex.Position.y, vertex.Position.z)
                );
            }

            JPH::IndexedTriangleList jphTriangles;
            KBR_CORE_ASSERT(kerberosIndices.size() % 3 == 0, "Mesh indices must form triangles!");

            jphTriangles.reserve(kerberosIndices.size() / 3);
            for (size_t i = 0; i < kerberosIndices.size(); i += 3)
            {
                jphTriangles.push_back(
                    JPH::IndexedTriangle(
                        kerberosIndices[i],
                        kerberosIndices[i + 1],
                        kerberosIndices[i + 2]
                    )
                );
            }

            const JPH::MeshShapeSettings meshShapeSettings(jphVertices, jphTriangles);

            // meshShapeSettings.mBuildFaceQueryTree = true; // Often true for static meshes
            // meshShapeSettings.mMaxTrianglesPerLeaf = 8;   // Adjust as needed

            const JPH::ShapeSettings::ShapeResult shapeResult = meshShapeSettings.Create();
            if (shapeResult.HasError())
            {
                KBR_CORE_ERROR("Jolt: shape error on entity {}: {}", debugName, shapeResult.GetError().c_str());
                return nullptr;
            }
           
            return shapeResult.Get();
        }
	};
}
