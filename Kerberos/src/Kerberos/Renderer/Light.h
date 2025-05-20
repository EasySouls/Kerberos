#pragma once

#include <glm/glm.hpp>

namespace Kerberos
{
    enum class LightType : uint8_t
    {
        Directional = 0,
        Point = 1,
        Spot = 2
    };

    struct Light
    {
        glm::vec3 Color = glm::vec3(1.0f);
        float Intensity = 1.0f;
        // bool CastShadows = false;

        LightType Type;

    protected:
        explicit Light(const LightType type) : Type(type) {}
    };

    struct DirectionalLight : public Light
    {
        glm::vec3 Direction = glm::vec3(0.0f, -1.0f, 0.0f);

        DirectionalLight() : Light(LightType::Directional) {}
    };

    struct PointLight : public Light
    {
        /// World-space position
        glm::vec3 Position = glm::vec3(0.0f); 

        /// Attenuation factors
        float Constant = 1.0f;
        float Linear = 0.09f;
        float Quadratic = 0.032f;

        PointLight() : Light(LightType::Point) {}
    };

    struct SpotLight : public PointLight 
    {
        glm::vec3 Direction = glm::vec3(0.0f, -1.0f, 0.0f);
        float CutOffAngleRadians = glm::radians(12.5f);
        float OuterCutOffAngleRadians = glm::radians(17.5f);

        SpotLight() : PointLight() { Type = LightType::Spot; }
    };
}