#pragma once

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>

namespace Kerberos
{
    struct DirectionalLight
    {
        bool IsEnabled = true;
        glm::vec3 Direction = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec3 Color = glm::vec3(1.0f);
        float Intensity = 1.0f;
    };

    struct PointLight
    {
        /// World-space position
        glm::vec3 Position = glm::vec3(0.0f); 

        glm::vec3 Color = glm::vec3(1.0f);
        float Intensity = 1.0f;

        /// Attenuation factors
        float Constant = 1.0f;
        float Linear = 0.09f;
        float Quadratic = 0.032f;
    };

    struct SpotLight : PointLight 
    {
        glm::vec3 Direction = glm::vec3(0.0f, -1.0f, 0.0f);
        float CutOffAngleRadians = glm::radians(12.5f);
        float OuterCutOffAngleRadians = glm::radians(17.5f);
    };
}